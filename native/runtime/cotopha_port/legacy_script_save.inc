namespace {
class CheckedSaveMemory final : public EMemoryFile {
public:
    unsigned long Write(const void *data,unsigned long count) override {
        const auto written=EMemoryFile::Write(data,count);
        if(written!=count)failed=true;
        return written;
    }
    ESLError SetEndOfFile() override {
        const auto error=EMemoryFile::SetEndOfFile();if(error)failed=true;return error;
    }
    bool failed=false;
};
UINT64 SaveRecordID(const char *name){UINT64 id;std::memcpy(&id,name,8);return id;}
bool SameSavedBytes(ESLFileObject &file,const EMemoryFile &expected) {
    if(file.GetLargeLength()!=expected.GetLargeLength())return false;
    file.SeekLarge(0,ESLFileObject::FromBegin);
    const auto *bytes=static_cast<const uint8_t *>(expected.GetBuffer());
    uint8_t block[0x10000];
    for(UINT64 at=0;at<expected.GetLargeLength();) {
        const size_t count=std::min<UINT64>(sizeof(block),expected.GetLargeLength()-at);
        if(file.Read(block,count)!=count||std::memcmp(block,bytes+at,count))return false;
        at+=count;
    }
    return true;
}
ESLError BuildSavedContainer(CheckedSaveMemory &output,CheckedSaveMemory &title,
    CheckedSaveMemory &payload,bool context,bool noCompress) {
    if(output.Create(0x10000))return eslErrGeneral;
    const bool raw=context&&(noCompress||payload.GetLength()>0x40000);
    const UINT64 titleID=SaveRecordID("title   ");
    const UINT64 payloadID=SaveRecordID(context?(raw?"ccontext":"context "):"object  ");
    EMCFile emc;EMCFile::FILE_HEADER header;
    EMCFile::SetFileHeader(header,EMCFile::fidUndefinedEMC,"cotomi context file");
    if(emc.Open(&output,&header)||emc.DescendRecord(&titleID)||
        emc.Write(title.GetBuffer(),title.GetLength())!=title.GetLength()||emc.AscendRecord()||
        emc.DescendRecord(&payloadID))return eslErrGeneral;
    if(raw) {
        if(emc.Write(payload.GetBuffer(),payload.GetLength())!=payload.GetLength())return eslErrGeneral;
    } else {
        uint8_t length[4];StudySteadyLegacyWire::Write32(length,payload.GetLength());
        if(emc.Write(length,sizeof(length))!=sizeof(length))return eslErrGeneral;
        ERISAEncodeContext encoder(0x10000);encoder.AttachOutputFile(&emc);encoder.PrepareToEncodeERISANCode();
        if(encoder.EncodeERISANCodeBytes(static_cast<const SBYTE *>(payload.GetBuffer()),payload.GetLength())!=payload.GetLength()||
            encoder.FinishERISACode())return ESLErrorMsg("Saved payload encoding failed before commit");
    }
    if(emc.AscendRecord())return eslErrGeneral;
    emc.Close();
    if(output.failed)return ESLErrorMsg("Saved container write failed before commit");
    // Read/decode bytes, not ECS objects: validation must not run constructors
    // or replace the current script context as a side effect of saving it.
    output.SeekLarge(0,ESLFileObject::FromBegin);
    EMCFile validation;
    if(validation.Open(&output)||validation.DescendRecord(&titleID)||!SameSavedBytes(validation,title)||
        validation.AscendRecord()||validation.DescendRecord(&payloadID))return eslErrGeneral;
    if(raw) {
        if(!SameSavedBytes(validation,payload))return eslErrGeneral;
    } else {
        EMemoryFile decoded;
        if(DecodeSavedRecord(validation,decoded,"atomic verification")||!SameSavedBytes(decoded,payload))
            return ESLErrorMsg("Saved payload verification failed before commit");
    }
    if(validation.AscendRecord())return eslErrGeneral;
    validation.Close();return eslErrSuccess;
}
}

void ECSFile::BeginSaveAttempt() {
    for(unsigned i=0;i<m_stackFile.GetSize();++i)
        if(auto *atomic=dynamic_cast<LegacyAtomicSaveFile *>(m_stackFile.GetAt(i)))atomic->BeginSave();
}
ESLError ECSFile::SaveStaged(ECSObject *object,ECSObject *title,bool saveContext,
    bool noCompress,ECSContext &context) {
    auto *atomic=dynamic_cast<LegacyAtomicSaveFile *>(m_pFile);
    // Mark the delayed Open first: even argument/serializer failures must not
    // make a later Close publish its empty create/truncate staging file.
    BeginSaveAttempt();
    if(!m_pFile||!(m_dwOpenFlags&ESLFileObject::modeWrite))return eslErrGeneral;
    if(!atomic&&!ESLTypeCast<EMemoryFile>(m_pFile))
        return ESLErrorMsg("Atomic save requires a savedata file or owned memory file");
    if(m_stackFile.GetSize()!=1)return ESLErrorMsg("Saving into archive/nested streams is not supported");
    if(saveContext&&ECotophaScript::GetCurrentThread())return eslErrGeneral;
    const UINT64 prefix=m_pFile->GetLargePosition();
    // The original SaveContext hides its output File name while serializing
    // that File inside the context. Keep this behavior but restore the live
    // name on success/failure so a retry still targets the same transaction.
    const EWideString filename=m_strFileName.m_varStr;
    struct RestoreName {
        ECSString &value;const EWideString &name;
        ~RestoreName(){value.m_varStr=name;}
    } restoreName{m_strFileName,filename};
    if(saveContext)m_strFileName.m_varStr=L"";
    try {
        CheckedSaveMemory savedTitle,payload,complete;
        if(savedTitle.Create(0x1000)||payload.Create(0x10000))return eslErrGeneral;
        if(title)title->IndexAllMember();
        if(const auto error=context.SaveObject(savedTitle,title))return error;
        if(savedTitle.failed)return ESLErrorMsg("Saved title write failed before commit");
        const auto error=saveContext?context.Save(payload):context.SaveObject(payload,object);
        if(error) {
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy File save serialization failed: %s",GetESLErrorMsg(error));
            return error;
        }
        if(payload.failed)return ESLErrorMsg("Saved payload write failed before commit");
        if(const auto error=BuildSavedContainer(complete,savedTitle,payload,saveContext,noCompress))return error;
        return CommitStaged(complete,prefix);
    } catch(...) {return ESLErrorMsg("Save serialization failed; existing destination was preserved");}
}

ESLError ECSFile::CommitStaged(EMemoryFile &complete,UINT64 prefix) {
    if(auto *atomic=dynamic_cast<LegacyAtomicSaveFile *>(m_pFile))
        return atomic->Replace(complete.GetBuffer(),complete.GetLength(),prefix);
    auto *original=ESLTypeCast<EMemoryFile>(m_pFile);
    if(!original||m_stackFile.GetSize()!=1||prefix>original->GetLargeLength()||
        prefix>UINT32_MAX-complete.GetLargeLength())return eslErrInvalidParam;
    std::unique_ptr<EMemoryFile> replacement(new EMemoryFile);
    if(replacement->Create(prefix+complete.GetLargeLength())||
        (prefix&&replacement->Write(original->GetBuffer(),prefix)!=prefix)||
        replacement->Write(complete.GetBuffer(),complete.GetLength())!=complete.GetLength())return eslErrGeneral;
    // Allocations and writes have all succeeded. Swap the whole owned memory
    // stream only now, so failed serializers leave its bytes and cursor intact.
    auto *old=m_stackFile.Pop();
    m_pFile=replacement.release();m_stackFile.Push(m_pFile);delete old;
    return eslErrSuccess;
}
