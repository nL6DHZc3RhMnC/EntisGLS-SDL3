#!/usr/bin/env python3
"""Audit actual scenario SuperSprite effects; do not count alpha-mask IDs as effects."""
import argparse,collections,hashlib,json,re,struct,subprocess,tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from noa import entries
from csx_inspect import CSX
ROOT=Path(__file__).resolve().parents[1]
TYPES=['Nothing','TileImage','FilterWhite','FilterLight','FilterBlack','FilterDark','RasterScroll','WaveCircle','Shimmer','ShadingOff','ShadingLight','SmashParticle2D','SmashParticle3D']
TAGS=re.compile(r'<([A-Za-z_][\w-]*)(\s[^<>]*?)?/?>')
ATTRS=re.compile(r'([\w@:-]+)="([^"]*)"')
def natural(text):return [int(x)if x.isdigit()else x for x in re.split(r'(\d+)',text)]
def scan(item):
    archive,e=item
    with archive.open('rb')as f:
        f.seek(64+e.offset);tag,length=struct.unpack('<8sQ',f.read(16));data=f.read(length)
    if e.encoding==0x80000010:
        with tempfile.TemporaryDirectory(prefix='study-effect-audit-')as tmp:
            src=Path(tmp)/'input';dst=Path(tmp)/'output';src.write_bytes(data)
            subprocess.run([str(ROOT/'build/host/erisan_decode'),str(src),str(dst),str(e.size)],check=True,stdout=subprocess.DEVNULL)
            data=dst.read_bytes()
    elif e.encoding!=0:raise ValueError(f'{archive.name}/{e.name}: unsupported encoding')
    if len(data)!=e.size:raise ValueError('decoded size mismatch')
    text=data.decode('utf-16')if data[:2]in[b'\xff\xfe',b'\xfe\xff']else data.decode('utf-8-sig')
    found=[];other=[];tag_counts=collections.Counter();mesh_mentions=[]
    for match in TAGS.finditer(text):
        command=match[1];a=dict(ATTRS.findall(match[2]or''));tag_counts[command]+=1
        if command=='dynbs':mesh_mentions.append({'command':command,'attributes':a})
        if command not in ['effect','chgbg','chgscrn']:continue
        if 'type'not in a:continue
        try:kind=int(a['type'],0);flags=int(a.get('flags','0'),0)
        except ValueError:other.append({'command':command,'attributes':a});continue
        # ScreenData::SetEffectParam gives bit 0x100 alpha-mask selection priority
        # over 0x200 SuperSprite effects. ChangeScreen's added 0x2000 is unrelated.
        route='alpha-mask'if flags&0x100 else 'super'if flags&0x200 else 'nothing'
        mapped=TYPES[kind]if 0<=kind<len(TYPES)else'Nothing' # original bounds check
        found.append({'archive':archive.name,'file':e.name,'command':command,'line':a.get('@l'),
            'route':route,'type':kind,'effect':mapped if route=='super'else'Nothing','flags':flags,'attributes':a})
    return {'archive':archive.name,'file':e.name,'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data),'commands':found,'dynamic':other,'tags':dict(tag_counts),'dynbs':mesh_mentions,'mesh_tokens':{token:text.count(token)for token in ['dynbs','SetMeshWarpEffect','LoadLayerSet','PhysicalSpringMesh','<layer_set']}}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,default=ROOT/'StudySteadyR18');p.add_argument('--csx',type=Path,default=ROOT/'build/game/script.csx');p.add_argument('--output',type=Path,default=ROOT/'native/motion_bridge/evidence/super_effect_usage.json');a=p.parse_args()
    items=[(archive,e)for archive in sorted(a.game.glob('*.noa'))for e in entries(archive)if e.name.lower().endswith('.srcxml')and not e.attributes&0x10]
    with ThreadPoolExecutor(max_workers=4)as pool:scans=list(pool.map(scan,items))
    grouped=collections.defaultdict(list)
    for source in scans:
        for row in source['commands']:
            if row['route']=='super':grouped[row['effect']].append(row)
    effects=[]
    for effect,rows in grouped.items():
        rows.sort(key=lambda r:(not r['file'].startswith('common'),natural(r['file']),int(r['line']or-1),r['archive']))
        effects.append({'effect':effect,'script_type':TYPES.index(effect),'native_type':TYPES.index(effect)+(1 if TYPES.index(effect)>=9 else 0),'supported':effect in TYPES[:7]+TYPES[9:11],
            'count':len(rows),'by_archive':dict(collections.Counter(r['archive']for r in rows)),
            'first':rows[:8],'first_by_archive':{archive:next(r for r in rows if r['archive']==archive)for archive in sorted({r['archive']for r in rows})},'files':len({(r['archive'],r['file'])for r in rows})})
    effects.sort(key=lambda r:r['script_type'])
    csx=CSX(a.csx);decoded={};unknown=[];calls=[]
    for f in csx.functions:
        pos=f['address'];end=pos+f.get('size',0)
        while pos<end:
            try:ins=csx.instruction(pos)
            except (ValueError,IndexError,KeyError)as error:unknown.append({'function':f['name'],'address':hex(pos),'error':str(error)});break
            decoded[pos]=ins
            if 'call'in ins['mnemonic']and any(k in str(ins['args'])for k in ['SetEffectParameter','SetMeshWarpEffect']):calls.append({'function':f['name'],**ins})
            pos+=ins['size']
    slots=[]
    for ci,cls in enumerate(csx.classes):
        for mi,method in enumerate(cls['methods']):
            if method['name']!='SetMeshWarpEffect':continue
            pair=struct.pack('<II',ci,mi);pos=0;raw=[]
            while True:
                pos=csx.image.find(pair,pos)
                if pos<0:break
                if pos>=5 and csx.image[pos-5]in[19,20]:
                    address=pos-5;raw.append(hex(address))
                    owner=next((f for f in csx.functions if f['address']<=address<f['address']+f.get('size',0)),None)
                    # These original helpers have a naked arithmetic body then
                    # return to object bytecode for their final native call.
                    if owner and address+16==owner['address']+owner['size'] and csx.image[address+13:address+16]==b'\x01\x12\x00':
                        ins=csx.instruction(address)
                        if not any(c['address']==address for c in calls):calls.append({'function':owner['name'],'validated_after_naked_region':True,**ins})
                pos+=1
            slots.append({'class':cls['name'],'class_index':ci,'method_index':mi,'typed_calls':raw})
    report={'archives':dict(collections.Counter(s['archive']for s in scans)),'script_count':len(scans),
        'decoded_xml_bytes':sum(s['bytes']for s in scans),'scope':'All .srcxml entries, including original/patch and alternate route variants; counts are static occurrences, not execution counts.',
        'routing_evidence':{'function':'ScreenData::SetEffectParam','alpha_priority':'0x21822..0x2190a: flags & 0x100 => Nothing plus alphaNNN.eri','super_branch':'0x2190f..0x219cb: otherwise flags & 0x200 => GetEffectParam type table','type_table':'ScreenData::GetEffectParam 0x20e05..0x20eac; invalid indices become Nothing'},
        'effects':effects,'first_scene':[r for s in scans if s['file']in['common1_1.srcxml','common1_1_R.srcxml']for r in s['commands']if r['route']=='super'],
        'dynbs_commands':[{'archive':v['archive'],'file':v['file'],'commands':v['dynbs']}for v in scans if v['dynbs']],
        'mesh_xml_tokens':{token:sum(v['mesh_tokens'][token]for v in scans)for token in ['dynbs','SetMeshWarpEffect','LoadLayerSet','PhysicalSpringMesh','<layer_set']},
        'mesh_calls':[c for c in calls if'SetMeshWarpEffect'in str(c['args'])],'mesh_typed_slots':slots,
        'mesh_constant_strings':[{'text':s,'references':csx.string_references[i]}for i,s in enumerate(csx.strings)if'SetMeshWarpEffect'in s],
        'native_effect_calls':calls,'decoded_instructions':len(decoded),'undecoded_regions':unknown,
        'xml_tags':dict(sum((collections.Counter(v['tags'])for v in scans),collections.Counter())),
        'dynamic_xml_types':[{'archive':s['archive'],'file':s['file'],'entries':s['dynamic']}for s in scans if s['dynamic']],
        'sources':[{'archive':s['archive'],'file':s['file'],'sha256':s['sha256'],'bytes':s['bytes']}for s in scans]}
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print('scripts',report['archives'],'total',report['script_count'],'bytes',report['decoded_xml_bytes'])
    for e in effects:print(e['script_type'],e['effect'],'supported'if e['supported']else'UNSUPPORTED',e['by_archive'],'first',[(x['file'],x['line'],x['command'])for x in e['first'][:3]])
    print('SetMeshWarpEffect decoded',len(report['mesh_calls']),'raw',sum(len(x['typed_calls'])for x in slots),'dynamic strings',len(report['mesh_constant_strings']))
    print('evidence',a.output)
if __name__=='__main__':main()
