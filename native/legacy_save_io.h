#pragma once
#include "../tools/legacy_serialization.h"
#include <cmath>
#include <cstring>
#include <vector>

// Explicit Windows x86 scalar layout. Reader errors are sticky and every
// collection has a count bound; callers never serialize native C++ structs.
namespace LegacySave {
struct Writer {
    ESLFileObject& file;
    ESLError error=eslErrSuccess;
    void Bytes(const void* data,size_t length) {if(!error&&file.Write(data,length)!=length)error=eslErrGeneral;}
    void U32(uint32_t value) {uint8_t bytes[4];StudySteadyLegacyWire::Write32(bytes,value);Bytes(bytes,4);}
    void I32(int32_t value) {U32(uint32_t(value));}
    void F32(double value) {float x=float(value);uint32_t bits;std::memcpy(&bits,&x,4);U32(bits);}
    void F64(double value) {uint64_t bits;std::memcpy(&bits,&value,8);uint8_t bytes[8];StudySteadyLegacyWire::Write64(bytes,bits);Bytes(bytes,8);}
    void String(const wchar_t* value) {EWideString string=value?value:L"";if(!error&&!StudySteadyLegacyWire::WriteWideString(file,string))error=eslErrGeneral;}
    void Reference(ECSReference& value,ECSContext& context) {if(!error)error=value.Save(file,context);}
    void Object(ECSObject& value,ECSContext& context) {if(!error)error=value.Save(file,context);}
    void Rect(const SakuraGL::SGLRect& value) {I32(value.left);I32(value.top);I32(value.right);I32(value.bottom);}
};
struct Reader {
    ESLFileObject& file;
    ESLError error=eslErrSuccess;
    void Bytes(void* data,size_t length) {if(error){std::memset(data,0,length);return;}if(file.Read(data,length)!=length){std::memset(data,0,length);error=eslErrGeneral;}}
    uint32_t U32() {uint8_t bytes[4];Bytes(bytes,4);return StudySteadyLegacyWire::Read32(bytes);}
    int32_t I32() {return int32_t(U32());}
    float F32() {uint32_t bits=U32();float value;std::memcpy(&value,&bits,4);if(!std::isfinite(value))error=eslErrInvalidParam;return value;}
    double F64() {uint8_t bytes[8];Bytes(bytes,8);uint64_t bits=StudySteadyLegacyWire::Read64(bytes);double value;std::memcpy(&value,&bits,8);if(!std::isfinite(value))error=eslErrInvalidParam;return value;}
    uint32_t Count(uint32_t maximum=65536) {const auto count=U32();if(count>maximum){error=eslErrInvalidParam;return 0;}return count;}
    EWideString String() {
        EWideString result;const auto count=Count(8*1024*1024);
        const auto position=file.GetPosition(),length=file.GetLength();
        if(position>length||count>(length-position)/2)error=eslErrGeneral;
        if(error)return result;
        std::vector<uint8_t> bytes(size_t(count)*2);Bytes(bytes.data(),bytes.size());
        if(!error&&!StudySteadyLegacyWire::DecodeUtf16(bytes.data(),count,result))error=eslErrGeneral;
        return result;
    }
    void Reference(ECSReference& value,ECSContext& context) {if(!error)error=value.Load(file,context);}
    void Object(ECSObject& value,ECSContext& context) {if(!error)error=value.Load(file,context);}
    SakuraGL::SGLRect Rect() {const auto left=I32(),top=I32(),right=I32(),bottom=I32();return SakuraGL::SGLRect(left,top,right,bottom);}
    void Reserved() {if(U32()!=0)error=eslErrInvalidParam;}
};
}
