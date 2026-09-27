#ifndef STUDYSTEADY_CHECKED_STATE_FILE_H
#define STUDYSTEADY_CHECKED_STATE_FILE_H
#include <cstdint>
#include <cstring>
namespace StudySteadyLegacy {
class CheckedStateFile final:public ESLFileObject {
    ESLFileObject& file_;
public:
    bool failed=false;
    explicit CheckedStateFile(ESLFileObject& file):file_(file){SetAttribute(file.GetAttribute());}
    ESLFileObject* Duplicate() const override{return file_.Duplicate();}
    unsigned long Read(void* out,unsigned long bytes) override {
        const auto read=failed?0:file_.Read(out,bytes);
        if(read!=bytes){failed=true;if(bytes>read)std::memset(static_cast<uint8_t*>(out)+read,0,bytes-read);}
        return read;
    }
    unsigned long Write(const void* in,unsigned long bytes) override {
        const auto written=failed?0:file_.Write(in,bytes);failed|=written!=bytes;return written;
    }
    unsigned long GetLength() const override{return file_.GetLength();}
    unsigned long GetPosition() const override{return file_.GetPosition();}
    unsigned long Seek(long offset,SeekOrigin origin) override{return file_.Seek(offset,origin);}
    UINT64 GetLargeLength() const override{return file_.GetLargeLength();}
    UINT64 GetLargePosition() const override{return file_.GetLargePosition();}
    UINT64 SeekLarge(INT64 offset,SeekOrigin origin) override{return file_.SeekLarge(offset,origin);}
    ESLError SetEndOfFile() override {auto error=file_.SetEndOfFile();failed|=error!=0;return error;}
    ESLError Result(ESLError error=eslErrSuccess) const{return error?error:failed?eslErrGeneral:eslErrSuccess;}
};

}
#endif
