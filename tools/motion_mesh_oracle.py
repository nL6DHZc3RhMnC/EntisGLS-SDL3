#!/usr/bin/env python3
"""Compare portable mesh kernels against StudySteady DLL machine code in Unicorn.

Only the three bounded, allocation-free arithmetic functions are executed. No
Windows host entry point, imports, file access, or network code is executed.
Install optional oracle dependency into build/motion-oracle-venv: unicorn==2.1.4.
Normal native/Android builds do not depend on Unicorn or the Windows DLL.
"""
import argparse
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]
DLL_SHA='40d2e510106fc8a79e0fd07e5c0d4839d75503089d6fc9de0f0904f0ab73bd4d'
FUNCTIONS=[0x10059270,0x10059480,0x100595C0]

def f32(x): return struct.unpack('<f',struct.pack('<f',x))[0]

def make_emulator(dll):
    from unicorn import Uc,UC_ARCH_X86,UC_MODE_32
    from unicorn.x86_const import UC_X86_REG_CR4
    pe=struct.unpack_from('<I',dll,0x3c)[0]
    sections=struct.unpack_from('<H',dll,pe+6)[0]
    optional_size=struct.unpack_from('<H',dll,pe+20)[0]
    optional=pe+24
    if struct.unpack_from('<H',dll,optional)[0]!=0x10b: raise ValueError('Expected PE32')
    base=struct.unpack_from('<I',dll,optional+28)[0]
    size,headers=struct.unpack_from('<II',dll,optional+56)
    uc=Uc(UC_ARCH_X86,UC_MODE_32)
    uc.mem_map(base,(size+4095)&~4095)
    uc.mem_write(base,dll[:headers])
    for i in range(sections):
        off=optional+optional_size+40*i
        _,rva,raw_size,raw=struct.unpack_from('<IIII',dll,off+8)
        if raw_size:uc.mem_write(base+rva,dll[raw:raw+raw_size])
    for addr in [0x02000000,0x03000000,0x04000000]:uc.mem_map(addr,0x10000)
    uc.reg_write(UC_X86_REG_CR4,uc.reg_read(UC_X86_REG_CR4)|0x200)
    return uc

def native(uc,op,ratio,a,b,c,simd,alias=False):
    from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_XMM3
    src=0x03000000; dst=src if alias else src+0x300
    uc.mem_write(src,struct.pack('<96f',*(a+b+c)))
    uc.mem_write(0x10107430,bytes([simd]))
    stack=0x0200f000
    args=[0x04000000,src+128]
    if op==2:args.append(src+256)
    uc.mem_write(stack,struct.pack('<'+'I'*len(args),*args))
    uc.reg_write(UC_X86_REG_ESP,stack)
    uc.reg_write(UC_X86_REG_ECX,dst)
    uc.reg_write(UC_X86_REG_EDX,src)
    uc.reg_write(UC_X86_REG_XMM3,int.from_bytes(struct.pack('<f',ratio),'little'))
    uc.emu_start(FUNCTIONS[op],0x04000000,count=10000)
    return bytes(uc.mem_read(dst,128))

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--dll',type=Path,default=ROOT/'build/analysis/emotedriver.dll')
    p.add_argument('--psb',type=Path,default=ROOT/'build/game/haz_a.psb')
    p.add_argument('--json',type=Path,default=ROOT/'build/motion-bridge-host/haz_a.json')
    p.add_argument('--probe',type=Path,default=ROOT/'build/motion-tjs-host/motion_mesh_probe')
    p.add_argument('--report',type=Path,default=ROOT/'artifacts/motion-mesh-oracle.json')
    args=p.parse_args();dll=args.dll.read_bytes()
    if hashlib.sha256(dll).hexdigest()!=DLL_SHA:raise ValueError('DLL hash mismatch; refuse wrong reference')
    uc=make_emulator(dll);rng=random.Random(230923);cases=[]
    ratios=[0.0,-0.0,2**-24,2**-23,.125,.5,.999,1.0]
    for i in range(64):
        a,b,c=[[f32(rng.uniform(-1000,1000)) for _ in range(32)] for _ in range(3)]
        for ratio in ratios:cases.append((0,ratio,a,b,c))
        cases.append((1,0,a,b,c));cases.append((2,0,a,b,c))
    # Real v4 binary resources; the JSON contains offsets, not copied meshes.
    data=args.psb.read_bytes();root=json.loads(args.json.read_text());resources=[]
    def walk(v):
        if isinstance(v,dict):
            if 'rawMeshList' in v:
                resource=v['rawMeshList'];n=v['variable']['meshCount']
                if resource['$resourceBytes']!=n*128:raise ValueError('Unexpected resource shape')
                frames=[list(struct.unpack_from('<32f',data,resource['$resourceOffset']+i*128)) for i in range(n)]
                resources.append(frames)
                for i in range(n-1):
                    for ratio in [0.0,.25,.5,.75,1.0]:cases.append((0,ratio,frames[i],frames[i+1],[0.0]*32))
            for x in v.values():walk(x)
        elif isinstance(v,list):
            for x in v:walk(x)
    walk(root)
    build=ROOT/'build/motion-mesh-oracle';build.mkdir(exist_ok=True)
    input_path=build/'cases.bin';output_path=build/'portable.bin';expected_path=build/'windows.bin'
    input_path.write_bytes(b''.join(struct.pack('<If96f',op,ratio,*(a+b+c)) for op,ratio,a,b,c in cases))
    expected=[]
    for i,(op,ratio,a,b,c) in enumerate(cases):
        outputs=[native(uc,op,ratio,a,b,c,simd,alias) for simd in [0,1] for alias in [False,True]]
        if len(set(outputs))!=1:raise ValueError(f'Windows SSE/scalar/alias disagreement at {i}')
        expected.append(outputs[0])
    expected=b''.join(expected);expected_path.write_bytes(expected)
    subprocess.run([str(args.probe),'--kernels',str(input_path),str(output_path)],check=True)
    actual=output_path.read_bytes()
    result={'status':'pass' if actual==expected else 'fail','dll_sha256':DLL_SHA,
            'kernels':{hex(a):name for a,name in zip(FUNCTIONS,['interpolate','add','add_subtract'])},
            'cases':len(cases),'native_kernel_executions':len(cases)*4,'real_psb_resources':len(resources),
            'comparison':'bitwise binary32; both DLL SIMD paths, separate and in-place output',
            'portable_sha256':hashlib.sha256(actual).hexdigest(),'windows_sha256':hashlib.sha256(expected).hexdigest(),
            'full_mesh_controller_machine_code_tested':False,'rendering_tested':False}
    args.report.parent.mkdir(exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
    if actual!=expected:raise SystemExit('Portable kernel output differs from Windows reference')

if __name__=='__main__':main()
