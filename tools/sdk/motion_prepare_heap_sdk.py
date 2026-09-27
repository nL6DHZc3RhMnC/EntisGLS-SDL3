#!/usr/bin/env python3
"""Generate a checked copy of the SDK ObjectHeap translation unit; SDK is read-only."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
import re
from pathlib import Path
from entis_sdk import COTOPHA

SOURCE=COTOPHA/'Source/common/glscs/glscs_sakura2_obj_heap.cpp'


def function(text,name,body):
    text,count=re.subn(r'SError ObjectHeap::'+name+r'\s*\([^)]*\)\s*\{.*?\n\}',lambda _:body,text,flags=re.S)
    if count!=1:raise ValueError(f'{name}: expected 1, got {count}')
    return text


def generate():
    text=SOURCE.read_text(encoding='utf-8-sig').replace('\r\n','\n')
    for kind in ('Static','Dynamic'):
        text=function(text,'LoadHeap'+kind,'''SError ObjectHeap::LoadHeapKIND(SFileInterface* file, VirtualMachine* vm, Context* context)
{
    if(LoadHeapHeader(file,vm,context)!=errSuccess)return errFailed;
    unsigned int valid=0;
    for(size_t i=0;i<m_nLength;++i) {
        int clsid=VirtualMachine::clsidInvalid;
        if(file->Read(&clsid,sizeof(clsid))!=sizeof(clsid))return errFailed;
        if(clsid==VirtualMachine::clsidInvalid)continue;
        if(!vm)return errFailed;
        Object* object=vm->NewObjectByIdentity(context,clsid);
        if(!object)return errFailed;
        m_ptrArray[i]=object;
        object->m_dwHighAddr=(m_nSelector<<24)|(i&0x00ffffff);
        ++valid;
        if(object->LoadKIND(file,vm,context)!=errSuccess)return errFailed;
    }
    const bool matches=valid==m_nHeapCount;
    m_nHeapCount=valid;
    return matches?errSuccess:errFailed;
}'''.replace('KIND',kind))
        text=function(text,'SaveHeap'+kind,'''SError ObjectHeap::SaveHeapKIND(SFileInterface* file, VirtualMachine* vm, Context* context)
{
    if(SaveHeapHeader(file,vm,context)!=errSuccess)return errFailed;
    for(size_t i=0;i<m_nLength;++i) {
        Object* object=m_ptrArray[i];
        const int clsid=object&&vm?vm->GetClassIdentity(object->GetTypeName()):VirtualMachine::clsidInvalid;
        if(object&&clsid==VirtualMachine::clsidInvalid)return errFailed;
        if(file->Write(&clsid,sizeof(clsid))!=sizeof(clsid))return errFailed;
        if(object&&object->SaveKIND(file,vm,context)!=errSuccess)return errFailed;
    }
    return errSuccess;
}'''.replace('KIND',kind))
    text=function(text,'LoadHeapHeader','''SError ObjectHeap::LoadHeapHeader(SFileInterface* file, VirtualMachine* vm, Context* context)
{
    if(!file)return errFailed;
    uint32_t headerSize=0;HEAP_HEADER header{};
    if(file->Read(&headerSize,4)!=4 || headerSize<sizeof(header) || headerSize>4096 ||
       file->Read(&header,sizeof(header))!=sizeof(header))return errFailed;
    for(uint32_t remain=headerSize-sizeof(header);remain;) {
        uint8_t discard[256];const uint32_t count=remain>sizeof(discard)?sizeof(discard):remain;
        if(file->Read(discard,count)!=count)return errFailed;remain-=count;
    }
    const auto position=file->GetPosition(),length=file->GetLength();
    if(header.nHeapLimit>1048576 || header.nHeapCount>header.nHeapLimit ||
       header.iHeapNext>header.nHeapLimit || position>length ||
       uint64_t(header.nHeapLimit)*4>length-position)return errFailed;
    RemoveAll(vm,context);
    SetLength(header.nHeapLimit);
    m_nHeapCount=header.nHeapCount;m_iHeapNext=header.iHeapNext;
    return errSuccess;
}''')
    text=function(text,'SaveHeapHeader','''SError ObjectHeap::SaveHeapHeader(SFileInterface* file, VirtualMachine*, Context*)
{
    if(!file || m_nLength>1048576 || m_nHeapCount>m_nLength || m_iHeapNext>m_nLength)return errFailed;
    HEAP_HEADER header{m_nHeapCount,m_iHeapNext,uint32_t(m_nLength)};
    const uint32_t size=sizeof(header);
    return file->Write(&size,4)==4 && file->Write(&header,size)==size?errSuccess:errFailed;
}''')
    return '// Generated from original SDK SHA-256 '+hashlib.sha256(SOURCE.read_bytes()).hexdigest()+'\n'+text


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    text=generate();args.output.parent.mkdir(parents=True,exist_ok=True)
    if not args.output.exists() or args.output.read_text()!=text:args.output.write_text(text)
    print(args.output)

if __name__=='__main__':main()
