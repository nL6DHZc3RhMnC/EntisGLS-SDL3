#include "legacy_file.h"
#include <algorithm>
#include <cstring>
#include <limits>

// ESLFileObject/EMemoryFile preserve the SDK interfaces and ownership rules.
// Platform file I/O is supplied by LegacyFileAdapter below, including 64-bit NOA
// offsets (the game's psb.noa is larger than 4 GiB).
IMPLEMENT_CLASS_INFO(ESLFileObject, ESLObject)
IMPLEMENT_CLASS_INFO(EMemoryFile, ESLFileObject)

namespace {
class FileLock {
public:
    explicit FileLock(const ESLFileObject &file) : file_(file) { file_.Lock(); }
    ~FileLock() { file_.Unlock(); }
private:
    const ESLFileObject &file_;
};
}

ESLFileObject::ESLFileObject() : m_nAttribute(0), m_pOpener(nullptr) {
    InitializeCriticalSection(&m_cs);
}
ESLFileObject::~ESLFileObject() { DeleteCriticalSection(&m_cs); }
void ESLFileObject::Lock() const { EnterCriticalSection(const_cast<CRITICAL_SECTION *>(&m_cs)); }
void ESLFileObject::Unlock() const { LeaveCriticalSection(const_cast<CRITICAL_SECTION *>(&m_cs)); }
ESLError ESLFileObject::SetEndOfFile() { return eslErrGeneral; }
UINT64 ESLFileObject::GetLargeLength() const { return GetLength(); }
UINT64 ESLFileObject::SeekLarge(INT64 offset, SeekOrigin origin) { return Seek(offset, origin); }
UINT64 ESLFileObject::GetLargePosition() const { return GetPosition(); }
ESLFileObject *ESLFileObject::OpenFileObject(const wchar_t *path, int flags) {
    return m_pOpener ? m_pOpener->OpenFileObject(path, flags) : nullptr;
}

EMemoryFile::EMemoryFile() : m_ptrMemory(nullptr), m_nLength(0), m_nPosition(0), m_nBufferSize(0) {}
EMemoryFile::~EMemoryFile() { Delete(); }
ESLError EMemoryFile::Create(unsigned long length) {
    FileLock lock(*this);
    Delete();
    // The original ESL heap accepts a DWORD byte count even on 64-bit hosts.
    if (length > UINT32_MAX) return eslErrInvalidParam;
    m_ptrMemory = eslHeapAllocate(nullptr, std::max<unsigned long>(length, 1), 0);
    if (!m_ptrMemory) return eslErrGeneral;
    m_nBufferSize = std::max<unsigned long>(length, 1);
    m_nAttribute = modeRead | modeCreate;
    return eslErrSuccess;
}
ESLError EMemoryFile::Open(const void *memory, unsigned long length) {
    if (!memory && length) return eslErrInvalidParam;
    FileLock lock(*this);
    Delete();
    m_ptrMemory = const_cast<void *>(memory);
    m_nBufferSize = m_nLength = length;
    m_nAttribute = modeRead;
    return eslErrSuccess;
}
void EMemoryFile::Delete() {
    FileLock lock(*this);
    if (m_ptrMemory && (m_nAttribute & modeWrite)) eslHeapFree(nullptr, m_ptrMemory);
    m_ptrMemory = nullptr;
    m_nLength = m_nPosition = m_nBufferSize = 0;
    m_nAttribute = 0;
}
ESLFileObject *EMemoryFile::Duplicate() const {
    FileLock lock(*this);
    EMemoryFile *copy = new EMemoryFile;
    if (copy->Create(m_nLength) || copy->Write(m_ptrMemory, m_nLength) != m_nLength) {
        delete copy;
        return nullptr;
    }
    copy->Seek(0, FromBegin);
    return copy;
}
unsigned long EMemoryFile::Read(void *buffer, unsigned long bytes) {
    FileLock lock(*this);
    if (!(m_nAttribute & modeRead) || m_nPosition >= m_nLength || !buffer) return 0;
    const unsigned long count = std::min(bytes, m_nLength - m_nPosition);
    if (count) std::memcpy(buffer, static_cast<const BYTE *>(m_ptrMemory) + m_nPosition, count);
    m_nPosition += count;
    return count;
}
unsigned long EMemoryFile::Write(const void *buffer, unsigned long bytes) {
    FileLock lock(*this);
    if (!(m_nAttribute & modeWrite) || !bytes || !buffer ||
        m_nPosition > UINT32_MAX || bytes > UINT32_MAX - m_nPosition) return 0;
    const unsigned long end = m_nPosition + bytes;
    if (end > m_nBufferSize) {
        const unsigned long capacity = std::min<unsigned long>(UINT32_MAX,
            std::max(end, m_nBufferSize + m_nBufferSize / 2 + 4096));
        void *memory = eslHeapReallocate(nullptr, m_ptrMemory, capacity, ESL_HEAP_ZERO_INIT);
        if (!memory) return 0;
        m_ptrMemory = memory;
        m_nBufferSize = capacity;
    }
    if (m_nPosition > m_nLength)
        std::memset(static_cast<BYTE *>(m_ptrMemory) + m_nLength, 0, m_nPosition - m_nLength);
    std::memcpy(static_cast<BYTE *>(m_ptrMemory) + m_nPosition, buffer, bytes);
    m_nLength = std::max(m_nLength, end);
    m_nPosition = end;
    return bytes;
}
unsigned long EMemoryFile::GetLength() const { FileLock lock(*this); return m_nLength; }
unsigned long EMemoryFile::GetPosition() const { FileLock lock(*this); return m_nPosition; }
unsigned long EMemoryFile::Seek(long offset, SeekOrigin origin) {
    FileLock lock(*this);
    unsigned long base;
    switch (origin) {
        case FromBegin: base = 0; break;
        case FromCurrent: base = m_nPosition; break;
        case FromEnd: base = m_nLength; break;
        default: return m_nPosition;
    }
    if (offset < 0) {
        const unsigned long magnitude = static_cast<unsigned long>(-(offset + 1)) + 1;
        m_nPosition = magnitude > base ? 0 : base - magnitude;
    } else {
        if (static_cast<unsigned long>(offset) > ULONG_MAX - base) return m_nPosition;
        m_nPosition = base + offset;
    }
    return m_nPosition;
}
ESLError EMemoryFile::SetEndOfFile() {
    FileLock lock(*this);
    if (!(m_nAttribute & modeWrite)) return eslErrGeneral;
    if (m_nPosition > m_nLength) {
        const auto position = m_nPosition;
        if (position > UINT32_MAX) return eslErrInvalidParam;
        m_nPosition = position - 1;
        const BYTE zero = 0;
        if (Write(&zero, 1) != 1) { m_nPosition = position; return eslErrGeneral; }
    }
    m_nLength = m_nPosition;
    return eslErrSuccess;
}

LegacyFileAdapter::LegacyFileAdapter(SSystem::SFileInterface *file, int flags,
                                     SSystem::SFileOpener *opener)
    : file_(file), opener_(opener) { SetAttribute(flags); }
ESLFileObject *LegacyFileAdapter::Duplicate() const {
    FileLock lock(*this);
    auto *copy = file_->Duplicate();
    return copy ? new LegacyFileAdapter(copy, m_nAttribute, opener_) : nullptr;
}
unsigned long LegacyFileAdapter::Read(void *buffer, unsigned long bytes) {
    FileLock lock(*this); return file_->Read(buffer, bytes);
}
unsigned long LegacyFileAdapter::Write(const void *buffer, unsigned long bytes) {
    FileLock lock(*this); return file_->Write(buffer, bytes);
}
unsigned long LegacyFileAdapter::GetLength() const { return GetLargeLength(); }
unsigned long LegacyFileAdapter::Seek(long offset, SeekOrigin origin) { return SeekLarge(offset, origin); }
unsigned long LegacyFileAdapter::GetPosition() const { return GetLargePosition(); }
UINT64 LegacyFileAdapter::GetLargeLength() const { FileLock lock(*this); return file_->GetLength(); }
UINT64 LegacyFileAdapter::SeekLarge(INT64 offset, SeekOrigin origin) {
    FileLock lock(*this);
    return file_->Seek(offset, static_cast<SSystem::SFileInterface::SeekOrigin>(origin));
}
UINT64 LegacyFileAdapter::GetLargePosition() const { FileLock lock(*this); return file_->GetPosition(); }
ESLError LegacyFileAdapter::SetEndOfFile() { FileLock lock(*this); return static_cast<ESLError>(file_->SetEndOfFile()); }
ESLFileObject *LegacyFileAdapter::OpenFileObject(const wchar_t *path, int flags) {
    FileLock lock(*this);
    auto *opened = opener_ ? opener_->NewOpenFile(path, flags) : file_->NewOpenFile(path, flags);
    return opened ? new LegacyFileAdapter(opened, flags, opener_) : nullptr;
}
