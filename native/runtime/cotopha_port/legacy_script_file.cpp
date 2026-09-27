
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
    Copyright (c) 2003-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include "runtime/cotopha_port/legacy_script_file.h"
#include "runtime/cotopha_port/legacy_file.h"
#include "runtime/cotopha_port/legacy_atomic_file.h"
#include "runtime/cotopha_port/legacy_image_export.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include <sakuragl/sgl_erisa_lib.h>
#include "platform/log.h"

// Adapts a borrowed legacy file to the current engine's actual NOA/ERISAN APIs.
class LegacyScriptStream final : public SSystem::SFileInterface {
public:
    explicit LegacyScriptStream(ESLFileObject* file = nullptr, bool own = false)
        : file(file), owner(own) {}
    ~LegacyScriptStream() { if (owner) delete file; }
    SSystem::SFileInterface* Duplicate() const override {
        auto* copy = file ? file->Duplicate() : nullptr;
        return copy ? new LegacyScriptStream(copy, true) : nullptr;
    }
    size_t Read(void* p, size_t n) override { return file ? file->Read(p, n) : 0; }
    size_t Write(const void* p, size_t n) override { return file ? file->Write(p, n) : 0; }
    bool IsSeekable() const override { return true; }
    int64_t GetLength() const override { return file ? file->GetLargeLength() : 0; }
    int64_t GetPosition() const override { return file ? file->GetLargePosition() : 0; }
    int64_t Seek(int64_t n, SeekOrigin from) override {
        return file ? file->SeekLarge(n, static_cast<ESLFileObject::SeekOrigin>(from)) : 0;
    }
    SSystem::SError SetEndOfFile() override {
        return file ? static_cast<SSystem::SError>(file->SetEndOfFile()) : SSystem::errFailed;
    }
    ESLFileObject* file;
private:
    bool owner;
};

class LegacyScriptArchive final : public ESLFileObject {
public:
    DECLARE_CLASS_INFO(LegacyScriptArchive, ESLFileObject)
    explicit LegacyScriptArchive(ESLFileObject* source) : stream(source) { SetAttribute(source->GetAttribute()); }
    ESLError Open() { return static_cast<ESLError>(archive.OpenArchive(&stream)); }
    ESLError Descend(const wchar_t* path, const wchar_t* password, bool streaming) {
        return static_cast<ESLError>(archive.DescendFile(path, password,
            streaming ? ERISA::SGLArchiveFile::openAsStream : ERISA::SGLArchiveFile::openAsNormal));
    }
    ESLError Ascend() { return static_cast<ESLError>(archive.AscendFile()); }
    ESLFileObject* Duplicate() const override {
        auto* duplicate = archive.Duplicate();
        return duplicate ? new LegacyFileAdapter(duplicate, GetAttribute()) : nullptr;
    }
    unsigned long Read(void* p, unsigned long n) override { return archive.Read(p,n); }
    unsigned long Write(const void* p, unsigned long n) override { return archive.Write(p,n); }
    unsigned long GetLength() const override { return archive.GetLength(); }
    unsigned long GetPosition() const override { return archive.GetPosition(); }
    unsigned long Seek(long n, SeekOrigin origin) override { return SeekLarge(n,origin); }
    UINT64 GetLargeLength() const override { return archive.GetLength(); }
    UINT64 GetLargePosition() const override { return archive.GetPosition(); }
    UINT64 SeekLarge(INT64 n, SeekOrigin origin) override {
        return archive.Seek(n, static_cast<SSystem::SFileInterface::SeekOrigin>(origin));
    }
    ESLError SetEndOfFile() override { return static_cast<ESLError>(archive.SetEndOfFile()); }
private:
    LegacyScriptStream stream;
    ERISA::SGLArchiveFile archive;
};
IMPLEMENT_CLASS_INFO(LegacyScriptArchive, ESLFileObject)

class ERISADecodeContext {
public:
    explicit ERISADecodeContext(size_t bytes) : bits(bytes), decoder(&bits) {}
    void AttachInputFile(ESLFileObject* file) { stream.file=file; bits.AttachInputStream(&stream); }
    void PrepareToDecodeERISANCode() { decoder.PrepareToDecodeERISANCode(); }
    size_t DecodeERISANCodeBytes(SBYTE* out, size_t bytes) { return decoder.Read(out, bytes); }
private:
    LegacyScriptStream stream;
    ERISA::SGLDecodeBitStream bits;
    ERISA::SGLERISANDecodeContext decoder;
};
class ERISAEncodeContext {
public:
    explicit ERISAEncodeContext(size_t bytes) : bits(bytes), encoder(&bits) {}
    void AttachOutputFile(ESLFileObject* file) { stream.file=file; bits.AttachOutputStream(&stream); }
    void PrepareToEncodeERISANCode() { encoder.PrepareToEncodeERISANCode(); }
    size_t EncodeERISANCodeBytes(const SBYTE* in, size_t bytes) { return encoder.EncodeERISANCodeBytes(in, bytes); }
    ESLError FinishERISACode() {
        // The old API finalized and flushed in one call. In the current SDK
        // FinishERISACode only emits arithmetic-code bits; FinishEncoding also
        // invokes SGLEncodeBitStream::Flushout to commit them to the file.
        return static_cast<ESLError>(encoder.FinishEncoding());
    }
private:
    LegacyScriptStream stream;
    ERISA::SGLEncodeBitStream bits;
    ERISA::SGLERISANEncodeContext encoder;
};

static ESLError DecodeSavedRecord(EMCFile& encoded, EMemoryFile& decoded, const char* kind) {
    DWORD bytes = 0;
    if (encoded.Read(&bytes, sizeof(bytes)) != sizeof(bytes))
        return ESLErrorMsg("Compressed save record has no decoded-size field");
    if (bytes && encoded.GetLargePosition() >= encoded.GetLargeLength())
        return ESLErrorMsg("Compressed save record has no encoded payload");
    ESLError error = decoded.Create(bytes);
    if (error) return error;
    ERISADecodeContext decoder(0x10000);
    decoder.AttachInputFile(&encoded);
    decoder.PrepareToDecodeERISANCode();
    std::vector<SBYTE> block(std::min<size_t>(bytes, 0x10000));
    size_t total = 0;
    while (total < bytes) {
        const size_t requested = std::min<size_t>(block.size(), bytes - total);
        const size_t got = decoder.DecodeERISANCodeBytes(block.data(), requested);
        if (got != requested || decoded.Write(block.data(), got) != got) {
            study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
                "Legacy File %s ERISAN failed: expected=%u total=%zu requested=%zu got=%zu",
                kind, bytes, total, requested, got);
            return ESLErrorMsg("ERISAN saved-object decoded-size mismatch");
        }
        total += got;
    }
    const BYTE* head = static_cast<const BYTE*>(decoded.GetBuffer());
    study::platform::LogPrint(study::platform::LogPriority::Debug, "StudySteady",
        "Legacy File %s decoded: bytes=%u head=%02x %02x %02x %02x %02x",
        kind, bytes, bytes>0?head[0]:0, bytes>1?head[1]:0,
        bytes>2?head[2]:0, bytes>3?head[3]:0, bytes>4?head[4]:0);
    decoded.Seek(0, ESLFileObject::FromBegin);
    return eslErrSuccess;
}


//////////////////////////////////////////////////////////////////////////////
// ファイル入出力用オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSFile, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSFile::ECSFile( void )
{
	m_pFile = NULL ;
	m_dwOpenFlags = 0 ;
	m_nCharaEncoding = EDescription::ceShiftJIS ;
	m_ppif = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSFile::~ECSFile( void )
{
	Close( ) ;
	//
	if ( m_ppif != NULL )
	{
		::eslHeapFree( NULL, m_ppif, 0 ) ;
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Open
	( const wchar_t * pwszFileName,
		DWORD dwOpenFlags, ECSContext * pContext )
{
    Close();
    m_downloadFailed = false;
    if (pwszFileName == nullptr || pwszFileName[0] == 0)
        return ESLErrorMsg("Android File.Open requires a file path");
    bool atomicCandidate = false;
    if (pContext != nullptr)
        m_pFile = LegacyAtomicSaveFile::TryOpen(pContext->GetEnvironment(),pwszFileName,dwOpenFlags,atomicCandidate);
    if (atomicCandidate && !m_pFile) return ESLErrorMsg("Cannot stage writable savedata file without modifying the existing file");
    if (pContext != nullptr && !atomicCandidate)
        m_pFile = pContext->OpenFileOnScript(pwszFileName, dwOpenFlags);
    else if (pContext == nullptr) {
        auto* file = SSystem::SFileOpener::DefaultNewOpenFile(pwszFileName, dwOpenFlags);
        if (file) m_pFile = new LegacyFileAdapter(file, dwOpenFlags);
    }
    if (m_pFile == nullptr) return eslErrGeneral;
    m_strFileName.m_varStr = pwszFileName;
    m_stackFile.Push(m_pFile);
    m_dwOpenFlags = dwOpenFlags;
    return eslErrSuccess;
}

// インターネット上のファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenURL
	( const wchar_t * pwszURL,
		const wchar_t * pwszDownloadFile,
		unsigned int nFlags, ECSEnvironment * pEnv )
{
    Close();
    m_downloadFailed = true;
    return ESLErrorMsg("File.OpenURL is not connected to an Android network transport");
}

// メモリファイルを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CreateMemoryFile( DWORD dwInitBufSize )
{
	Close( ) ;
	m_downloadFailed = false;
	//
	EMemoryFile *	pmemfile = new EMemoryFile ;
	ESLError err = pmemfile->Create(dwInitBufSize);
	if (err) { delete pmemfile; return err; }
	m_stackFile.Push( pmemfile ) ;
	m_pFile = pmemfile ;
	m_dwOpenFlags = m_pFile->GetAttribute( ) ;
	//
	return	eslErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Close( void )
{
	ESLError closeError = eslErrSuccess;
	while ( m_stackFile.GetSize() > 0 )
	{
		ESLFileObject *	pfile = m_stackFile.Pop( ) ;
		if (auto *atomic = dynamic_cast<LegacyAtomicSaveFile *>(pfile)) {
			if (const auto error = atomic->FinishClose()) closeError = error;
		}
		delete	pfile ;
	}
	m_pFile = NULL ;
	m_strFileName.m_varStr.FreeString( ) ;
	m_dwOpenFlags = 0 ;
	return	closeError ;
}

// アーカイブを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenArchive( void )
{
    if (!m_pFile) return eslErrGeneral;
    auto* archive = new LegacyScriptArchive(m_pFile);
    ESLError err = archive->Open();
    if (err) { delete archive; return err; }
    m_stackFile.Push(archive); m_pFile = archive;
    return eslErrSuccess;
}

// アーカイブを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CloseArchive( void )
{
    if (!ESLTypeCast<LegacyScriptArchive>(m_pFile)) return eslErrGeneral;
    delete m_stackFile.Pop(); m_pFile = m_stackFile.GetLastAt();
    return eslErrSuccess;
}

// アーカイブファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenArchiveFile
	( const char * pszFilePath, const char * pszPassword, bool fStream )
{
    auto* archive = ESLTypeCast<LegacyScriptArchive>(m_pFile);
    if (!archive) return eslErrGeneral;
    return archive->Descend(EWideString(pszFilePath), EWideString(pszPassword), fStream);
}

// アーカイブファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CloseArchiveFile( void )
{
    auto* archive = ESLTypeCast<LegacyScriptArchive>(m_pFile);
    return archive ? archive->Ascend() : eslErrGeneral;
}

// 文字エンコーディングを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SetCharacterEncoding( const char * pszType )
{
	m_nCharaEncoding = EDescription::GetCharaEncodingType( pszType ) ;
	return	eslErrSuccess ;
}

// 文字エンコーディングを取得する
//////////////////////////////////////////////////////////////////////////////
const char * ECSFile::GetCharacterEncoding( void ) const
{
	return	EDescription::GetCharaEncodingName
				( (EDescription::CharacterEncoding) m_nCharaEncoding ) ;
}

// ファイルのダウンロード状況を取得
//////////////////////////////////////////////////////////////////////////////
INT64 ECSFile::GetCurrentDownloaded( void ) const
{
    return -1; // No pending asynchronous download exists in this implementation.
}

// ダウンロードファイルのファイル長を取得
//////////////////////////////////////////////////////////////////////////////
INT64 ECSFile::GetDownloadingFileLength( void ) const
{
    return -1;
}

// ファイルのダウンロード完了を取得
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsFileDownloaded( void ) const
{
    return m_pFile != nullptr && !m_downloadFailed;
}

// ファイルのダウンロードが失敗しているか？
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsFileDownloadFailed( void ) const
{
    return m_downloadFailed;
}

// ファイルのダウンロードを中断
//////////////////////////////////////////////////////////////////////////////
void ECSFile::CancelFileDownloading( void ) const
{
    // OpenURL returns an explicit error before starting any transfer.
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->Read( ptrBuffer, nBytes ) ;
	}
	return	0 ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->Write( ptrBuffer, nBytes ) ;
	}
	return	0 ;
}

// ファイル長取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::GetFileLength( void ) const
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->GetLargeLength( ) ;
	}
	return	0 ;
}

// ファイルポインタ取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::GetFilePosition( void ) const
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->GetLargePosition( ) ;
	}
	return	0 ;
}

// ファイルポインタ移動
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::Seek( INT64 nPos, int nSeekType )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->SeekLarge( nPos, (ESLFileObject::SeekOrigin) nSeekType ) ;
	}
	return	0 ;
}

// EOF 判定
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsEndOfFile( void ) const
{
	if ( m_pFile != NULL )
	{
		return	(m_pFile->GetLargePosition() >= m_pFile->GetLargeLength()) ;
	}
	return	true ;
}

// EOF 設定
//////////////////////////////////////////////////////////////////////////////
void ECSFile::SetEndOfFile( void )
{
	if ( (m_pFile != NULL) && (m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		m_pFile->SetEndOfFile( ) ;
	}
}

// 文字列の読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::ReadText( ECSObject & obj )
{
    if (!m_pFile || !(m_dwOpenFlags & ESLFileObject::modeRead)) return eslErrGeneral;
    const bool utf16 = m_nCharaEncoding == EDescription::ceUnknown ||
                       m_nCharaEncoding == EDescription::ceUTF16;
    std::vector<uint8_t> bytes;
    uint8_t unit[2];
    const size_t width = utf16 ? 2 : 1;
    while (true) {
        size_t got = m_pFile->Read(unit,width);
        if (!got) break;
        if (got != width) return ESLErrorMsg("Truncated UTF-16 text input");
        if (unit[0] == '\n' && (!utf16 || unit[1] == 0)) break;
        bytes.insert(bytes.end(),unit,unit+width);
    }
    if (bytes.size() >= width && bytes[bytes.size()-width] == '\r' &&
        (!utf16 || bytes.back() == 0)) bytes.resize(bytes.size()-width);
    EWideString text;
    if (utf16) {
        if (!StudySteadyLegacyWire::DecodeUtf16(bytes.data(),bytes.size()/2,text)) return eslErrGeneral;
    } else {
        EString narrow(reinterpret_cast<const char*>(bytes.data()),bytes.size());
        EDescription::DecodeText(text,narrow,static_cast<EDescription::CharacterEncoding>(m_nCharaEncoding));
    }
    if (obj.m_vtType == csvtString) ((ECSString&)obj).m_varStr=text;
    else if (obj.m_vtType == csvtInteger) {
        EStreamWideString source=text; ((ECSInteger&)obj).SetValue(source.GetInteger());
    } else if (obj.m_vtType == csvtReal) {
        EStreamWideString source=text; ((ECSReal&)obj).m_varReal=source.GetRealNumber();
    } else return ESLErrorMsg("Invalid File.ReadText target type");
    return eslErrSuccess;
}

// 文字列の書き出し
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::WriteText( ECSObject & obj )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	0 ;
	}
	//
	// オブジェクト型の変換
	//
	EWideString	wstrText ;
	if ( obj.m_vtType == csvtString )
	{
		wstrText = ((ECSString&)obj).m_varStr ;
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		wstrText.FromInteger( ((ECSInteger&)obj).GetValue() ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		wstrText.FromReal( ((ECSReal&)obj).m_varReal ) ;
	}
	else if ( obj.m_vtType == csvtPointerReference )
	{
		CSVariableType	csvtRefType =
			((ECSPointerReference*)&obj)->GetMemoryObjectType() ;
		ESLError	err ;
		if ( csvtRefType == csvtReal )
		{
			REAL64		rValue ;
			err = obj.OperateReal( rValue ) ;
			if ( err )
			{
				return	0 ;
			}
			wstrText.FromReal( rValue ) ;
		}
		else
		{
			INT64		nValue ;
			err = obj.OperateInteger( nValue ) ;
			if ( err )
			{
				return	0 ;
			}
			wstrText.FromInteger( nValue ) ;
		}
	}
	else
	{
		return	0 ;
	}
	wstrText += L"\r\n" ;
	//
	// 文字コードを変換して書き出す
	//
	if (m_nCharaEncoding == EDescription::ceUnknown || m_nCharaEncoding == EDescription::ceUTF16)
    {
        std::vector<uint8_t> encoded;
        if (!StudySteadyLegacyWire::EncodeUtf16(wstrText, encoded)) return 0;
        return encoded.empty() ? 0 : m_pFile->Write(encoded.data(), encoded.size());
	}
	EString	strText ;
	EDescription::EncodeText
		( strText, wstrText,
			(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
	return	m_pFile->Write( strText.CharPtr(), strText.GetLength() ) ;
}

// バイナリの読み込み
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::ReadBinary( ECSObject & obj, long int nBytes )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	0 ;
	}
	DWORD	dwReadBytes = 0 ;
	if ( obj.m_vtType == csvtPointer )
	{
		//
		// ポインタ
		//
		void *	ptrBuf = obj.GetBuffer( 0, nBytes, true ) ;
		if ( ptrBuf != NULL )
		{
			dwReadBytes = m_pFile->Read( ptrBuf, nBytes ) ;
			obj.FlushBuffer( 0, nBytes, ptrBuf, true ) ;
		}
	}
	else if ( obj.m_vtType == csvtString )
	{
		//
		// 文字列
		//
		ECSString &	strObj = ((ECSString&)obj) ;
		if ( (m_nCharaEncoding == EDescription::ceUnknown)
			|| (m_nCharaEncoding == EDescription::ceUTF16) )
		{
			if (nBytes <= 0) return 0;
            std::vector<uint8_t> encoded((size_t(nBytes) + 1) & ~size_t(1));
            dwReadBytes = m_pFile->Read(encoded.data(), encoded.size());
            if ((dwReadBytes & 1) || !StudySteadyLegacyWire::DecodeUtf16(encoded.data(), dwReadBytes / 2, strObj.m_varStr)) return 0;
		}
		else
		{
			EString		strText ;
			EWideString	wstrText ;
			dwReadBytes = m_pFile->Read( strText.GetBuffer( nBytes ), nBytes ) ;
			strText.ReleaseBuffer( dwReadBytes ) ;
			EDescription::DecodeText
				( wstrText, strText,
					(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
			strObj.m_varStr = wstrText ;
		}
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		//
		// 整数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(SDWORD) ;
		}
		else if ( nBytes > sizeof(INT64) )
		{
			nBytes = sizeof(INT64) ;
		}
		INT64	nVal = 0 ;
		dwReadBytes = m_pFile->Read( &nVal, nBytes ) ;
		((ECSInteger&)obj).SetValue( nVal ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		//
		// 実数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(REAL64) ;
		}
		else if ( nBytes < sizeof(REAL32) )
		{
			nBytes = 0 ;
		}
		else if ( nBytes < sizeof(REAL64) )
		{
			nBytes = sizeof(REAL32) ;
		}
		else
		{
			nBytes = sizeof(REAL64) ;
		}
		if ( nBytes == sizeof(REAL32) )
		{
			REAL32	rVal = 0 ;
			dwReadBytes = m_pFile->Read( &rVal, nBytes ) ;
			((ECSReal&)obj).m_varReal = rVal ;
		}
		else if ( nBytes == sizeof(REAL64) )
		{
			dwReadBytes =
				m_pFile->Read( &(((ECSReal&)obj).m_varReal), nBytes ) ;
		}
	}
	else
	{
		ECSFile *	pfile = ESLTypeCast<ECSFile>( &obj ) ;
		if ( (pfile != NULL)
			&& (pfile->m_pFile != NULL)
			&& (pfile->m_dwOpenFlags & ESLFileObject::modeWrite) )
		{
			const DWORD		dwBufSize = 0x10000 ;
			EStreamBuffer	buf ;
			void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
			while ( dwReadBytes < (DWORD) nBytes )
			{
				DWORD	dwCurBytes = nBytes - dwReadBytes ;
				if ( dwCurBytes > dwBufSize )
				{
					dwCurBytes = dwBufSize ;
				}
				DWORD	dwCurRead = m_pFile->Read( ptrBuf, dwCurBytes ) ;
				DWORD	dwCurWritten =
					pfile->m_pFile->Write( ptrBuf, dwCurRead ) ;
				dwReadBytes += dwCurWritten ;
				if ( (dwCurRead < dwCurBytes) || (dwCurWritten < dwCurBytes) )
				{
					break ;
				}
			}
		}
	}
	return	dwReadBytes ;
}

// バイナリの書き出し
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::WriteBinary( ECSObject & obj, long int nBytes )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	0 ;
	}
	if ( obj.m_vtType == csvtPointer )
	{
		//
		// ポインタ
		//
		DWORD	dwWrittenBytes = 0 ;
		void *	ptrBuf = obj.GetBuffer( 0, nBytes, false ) ;
		if ( ptrBuf != NULL )
		{
			dwWrittenBytes = m_pFile->Write( ptrBuf, nBytes ) ;
			obj.FlushBuffer( 0, nBytes, ptrBuf, false ) ;
		}
		return	dwWrittenBytes ;
	}
	else if ( obj.m_vtType == csvtString )
	{
		//
		// 文字列
		//
		ECSString &	strObj = ((ECSString&)obj) ;
		if ( (m_nCharaEncoding == EDescription::ceUnknown)
			|| (m_nCharaEncoding == EDescription::ceUTF16) )
		{
			std::vector<uint8_t> encoded;
            if (!StudySteadyLegacyWire::EncodeUtf16(strObj.m_varStr, encoded)) return 0;
            size_t bytes = nBytes <= 0 ? encoded.size() : std::min<size_t>(nBytes, encoded.size());
            return bytes ? m_pFile->Write(encoded.data(), bytes) : 0;
		}
		EString		strText ;
		EWideString	wstrText = strObj.m_varStr ;
		EDescription::EncodeText
			( strText, wstrText,
				(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
		if ( nBytes <= 0 )
		{
			nBytes = strText.GetLength( ) ;
		}
		return	m_pFile->Write( strText.CharPtr(), nBytes ) ;
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		//
		// 整数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(SDWORD) ;
		}
		else if ( nBytes > sizeof(INT64) )
		{
			nBytes = sizeof(INT64) ;
		}
		INT64	nVal = ((ECSInteger&)obj).GetValue( ) ;
		return	m_pFile->Write( &nVal, nBytes ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(REAL64) ;
		}
		else if ( nBytes < sizeof(REAL32) )
		{
			nBytes = 0 ;
		}
		else if ( nBytes < sizeof(REAL64) )
		{
			nBytes = sizeof(REAL32) ;
		}
		else
		{
			nBytes = sizeof(REAL64) ;
		}
		if ( nBytes == sizeof(REAL32) )
		{
			REAL32	rVal = (REAL32) ((ECSReal&)obj).m_varReal ;
			return	m_pFile->Write( &rVal, nBytes ) ;
		}
		else if ( nBytes == sizeof(REAL64) )
		{
			return	m_pFile->Write( &(((ECSReal&)obj).m_varReal), nBytes ) ;
		}
	}
	else
	{
		ECSFile *	pfile = ESLTypeCast<ECSFile>( &obj ) ;
		if ( (pfile != NULL)
			&& (pfile->m_pFile != NULL)
			&& (pfile->m_dwOpenFlags & ESLFileObject::modeRead) )
		{
			const DWORD		dwBufSize = 0x10000 ;
			EStreamBuffer	buf ;
			void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
			DWORD	dwTotalWritten = 0 ;
			while ( dwTotalWritten < (DWORD) nBytes )
			{
				DWORD	dwCurBytes = nBytes - dwTotalWritten ;
				if ( dwCurBytes > dwBufSize )
				{
					dwCurBytes = dwBufSize ;
				}
				DWORD	dwCurRead = pfile->m_pFile->Read( ptrBuf, dwCurBytes ) ;
				DWORD	dwCurWritten = m_pFile->Write( ptrBuf, dwCurRead ) ;
				dwTotalWritten += dwCurWritten ;
				if ( (dwCurRead < dwCurBytes) || (dwCurWritten < dwCurBytes) )
				{
					break ;
				}
			}
			return	dwTotalWritten ;
		}
	}
	return	0 ;
}

// セーブファイルの中のチャンクを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenSaveFile( EMCFile & emcfile, ESLFileObject & file )
{
    // BMP's serialized FILEHEADER is exactly 14 bytes on every target ABI.
    const UINT64 original = file.GetLargePosition();
    if (original > file.GetLargeLength()) return eslErrGeneral;
    uint8_t header[14];
    UINT64 position = original;
    if (file.Read(header, sizeof(header)) == sizeof(header) && header[0]=='B' && header[1]=='M') {
        const uint32_t bytes = StudySteadyLegacyWire::Read32(header + 2);
        if (bytes < sizeof(header) || bytes > file.GetLargeLength() - original)
            return eslErrGeneral;
        position += bytes;
    }
    file.SeekLarge(position, ESLFileObject::FromBegin);
    if (emcfile.Open(&file)) {
        file.SeekLarge(original, ESLFileObject::FromBegin);
        return eslErrGeneral;
    }
    return eslErrSuccess;
}

// セーブファイル見出しの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadContextTitle( ECSObject *& pObj, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = m_pFile->GetPosition( ) ;
	if ( OpenSaveFile( emcfile, *m_pFile ) )
	{
		return	eslErrGeneral ;
	}
	if ( emcfile.DescendRecord( (UINT64*) "title   " ) )
	{
		return	ESLErrorMsg
			( "セーブファイルに見出し"
				"レコードが見つかりませんでした。" ) ;
	}
	if ( context.LoadObject( emcfile, pObj ) )
	{
		return	ESLErrorMsg( "見出しの読み込みに失敗しました。" ) ;
	}
	if ( pObj != NULL )
	{
		pObj->CommitAllReference( context ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	m_pFile->Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// オブジェクトの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadObject( ECSObject *& pObj, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = m_pFile->GetPosition( ) ;
	if ( OpenSaveFile( emcfile, *m_pFile ) )
	{
		return	eslErrGeneral ;
	}
	if ( emcfile.DescendRecord( (UINT64*) "object  " ) )
	{
		return	ESLErrorMsg
			( "セーブファイルにデータ"
				"レコードが見つかりませんでした。" ) ;
	}
	try
	{
		EMemoryFile memfile;
        ESLError decodeError = DecodeSavedRecord(emcfile, memfile, "object");
        if (decodeError) return decodeError;
		//
		ESLError objectError = context.LoadObject(memfile, pObj);
        if (objectError) {
            study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Legacy File LoadObject failed after ERISAN: %s", GetESLErrorMsg(objectError));
            return objectError;
        }
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "オブジェクトの読み込みに失敗しました。" ) ;
	}
	if ( pObj != NULL )
	{
		pObj->CommitAllReference( context ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	m_pFile->Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// コンテキストの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadContext( ECSContext & context, bool fNoCompressed )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	ESLFileObject *	pfile = m_pFile->Duplicate( ) ;
	Close( ) ;
	ESLError		err = LoadContext( *pfile, fNoCompressed, context ) ;
	delete	pfile ;
	return	err ;
}

ESLError ECSFile::LoadContext
	( ESLFileObject & file, bool fNoCompressed, ECSContext & context )
{
	if ( ECotophaScript::GetCurrentThread() != NULL )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = file.GetPosition( ) ;
	if ( OpenSaveFile( emcfile, file ) )
	{
		return	eslErrGeneral ;
	}
	if ( !fNoCompressed )
	{
		for ( ; ; )
		{
			if ( emcfile.DescendRecord() )
			{
				return	ESLErrorMsg
					( "セーブファイルにコンテキスト"
						"レコードが見つかりませんでした。" ) ;
			}
			UINT64	idRect = emcfile.GetRecordID() ;
			if ( idRect == *((UINT64*) "ccontext") )
			{
				fNoCompressed = true ;
				break ;
			}
			if ( idRect == *((UINT64*) "context ") )
			{
				fNoCompressed = false ;
				break ;
			}
			emcfile.AscendRecord() ;
		}
	}
	else
	{
		if ( emcfile.DescendRecord( (UINT64*) "ccontext" ) )
		{
			return	ESLErrorMsg
				( "セーブファイルにコンテキスト"
					"レコードが見つかりませんでした。" ) ;
		}
	}
	if ( fNoCompressed )
	{
		try
		{
			if ( const auto loadError = context.Load( emcfile ) )
			{
				study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy File restore failed: %s",GetESLErrorMsg(loadError));
				return loadError;
			}
		}
		catch( ... )
		{
			return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
		}
		emcfile.AscendRecord( ) ;
		emcfile.Close( ) ;
		return	eslErrSuccess ;
	}
	try
	{
		EMemoryFile memfile;
        ESLError decodeError = DecodeSavedRecord(emcfile, memfile, "context");
        if (decodeError) return decodeError;
		//
		if ( const auto loadError = context.Load( memfile ) )
		{
			study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy File restore failed: %s",GetESLErrorMsg(loadError));
			return loadError;
		}
	}
	catch( ... )
	{
		return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	file.Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// セーブファイルサムネイル画像の書き出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SaveThumbnailImage
	( ECSSprite * pPreview, int nWidth, int nHeight )
{
    auto *atomic=dynamic_cast<LegacyAtomicSaveFile *>(m_pFile);
    BeginSaveAttempt();
    if(!pPreview||!m_pFile||!(m_dwOpenFlags&ESLFileObject::modeWrite))return eslErrInvalidParam;
    auto *source=pPreview->GetImage();if(!source)return eslErrGeneral;
    if(!atomic&&!ESLTypeCast<EMemoryFile>(m_pFile))return eslErrNotSupported;
    if(m_stackFile.GetSize()!=1)return eslErrNotSupported;
    SakuraGL::SGLImage thumbnail;
    if(const auto error=LegacyCreateThumbnail(*source,thumbnail,nWidth,nHeight))return error;
    EMemoryFile encoded;
    if(const auto error=LegacyEncodeImage(thumbnail,encoded,L"image/bmp",-1))return error;
    if(atomic)return atomic->StagePrefix(encoded.GetBuffer(),encoded.GetLength());
    return CommitStaged(encoded,m_pFile->GetLargePosition());
}

// オブジェクトの書き出し
//////////////////////////////////////////////////////////////////////////////
#include "runtime/cotopha_port/legacy_script_save.inc"
ESLError ECSFile::SaveObject(ECSObject &object,ECSObject *title,ECSContext &context) {
    return SaveStaged(&object,title,false,false,context);
}
ESLError ECSFile::SaveContext(ECSObject *title,bool noCompress,ECSContext &context) {
    const auto error=SaveStaged(nullptr,title,true,noCompress,context);
    if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy File SaveContext failed: %s",GetESLErrorMsg(error));
    return error;
}

//////////////////////////////////////////////////////////////////////////////
// オブジェクトをダンプする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpObject( ECSObject & obj, ECSContext & context )
{
	if ( (m_pFile == NULL) || (context.m_pcsxi == NULL)
			|| !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	context.SuspendAllThread( ) ;
	obj.IndexAllMember( ) ;
	//
	EStreamBuffer	bufDump ;
	obj.DumpObject( bufDump, 0, context ) ;
	//
	EPtrBuffer	ptrbuf = bufDump.GetBuffer( ) ;
	m_pFile->Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	context.ResumeAllThread( ) ;
	//
	return	eslErrSuccess ;
}

// コンテキストをダンプする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpContext( ECSContext & context )
{
	if ( (m_pFile == NULL) || (context.m_pcsxi == NULL)
			|| !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	context.SuspendAllThread( ) ;
	//
	EStreamBuffer	bufDump ;
	context.DumpContext( bufDump ) ;
	//
	EPtrBuffer	ptrbuf = bufDump.GetBuffer( ) ;
	m_pFile->Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	context.ResumeAllThread( ) ;
	//
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSFile::GetTypeName( void ) const
{
	return	L"File" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSFile::Duplicate( void )
{
	ECSFile *	pFile = new ECSFile ;
	if ( m_pFile != NULL )
	{
		pFile->m_pFile = m_pFile->Duplicate() ;
		if (pFile->m_pFile != NULL) pFile->m_stackFile.Push(pFile->m_pFile);
		pFile->m_strFileName.m_varStr = m_strFileName.m_varStr ;
		pFile->m_dwOpenFlags = m_dwOpenFlags ;
		pFile->m_nCharaEncoding = m_nCharaEncoding ;
	}
	return	pFile ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元のオブジェクトが存在しません。" ) ;
	}
	INT64		nValue ;
	ESLError	err = pEntity->OperateInteger( nValue ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_pFile != NULL )
	{
		Seek( nValue, ESLFileObject::FromBegin ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	if ( csuopType == csuotLogicalNot )
	{
		m_pResult = new ECSInteger( - (long int) IsEndOfFile() ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "File 型の定義されていない単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "File 型への演算オブジェクトが存在しません。" ) ;
	}
	if ( csopType != csotAdd )
	{
		return	ESLErrorMsg( "File 型の定義されていない演算子です。" ) ;
	}
	WriteText( *pEntity ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	if ( obj.m_vtType != csvtInteger )
	{
		return	ESLErrorMsg( "File 型の定義されていない比較です。" ) ;
	}
	ECSInteger	intPos ;
	intPos.SetValue( GetFilePosition( ) ) ;
	return	intPos.Compare( context, nResult, cscpType, obj ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg( "定義されていない File メンバ関数です。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg
			( "定義されていない File メンバ関数の呼び出しです。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateBoolean( int & nBoolean )
{
	nBoolean = - (int) IsEndOfFile() ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateSizeOf( INT64 & nSize )
{
	nSize = GetFileLength() ;
	return	eslErrSuccess ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateInteger( INT64 & nValue )
{
	nValue = GetFilePosition() ;
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateString( EWideString & wstrValue )
{
	ECSString	strText ;
	ReadText( strText ) ;
	wstrValue = strText.m_varStr ;
	return	eslErrSuccess ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSFile::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	EMemoryFile *	pmemfile = ESLTypeCast<EMemoryFile>( m_pFile ) ;
	if ( pmemfile != NULL )
	{
		if ( (unsigned int) (iOffset + nSize) <= pmemfile->GetLength() )
		{
			return	((BYTE*)pmemfile->GetBuffer()) + iOffset ;
		}
	}
	return	NULL ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = m_strFileName.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	UINT64	nFilePos = GetFilePosition( ) ;
	file.Write( &m_dwOpenFlags, sizeof(m_dwOpenFlags) ) ;
	file.Write( &m_nCharaEncoding, sizeof(m_nCharaEncoding) ) ;
	file.Write( &nFilePos, sizeof(nFilePos) ) ;
	//
	DWORD			dwFlags = 0 ;
	EMemoryFile *	pmemfile = ESLTypeCast<EMemoryFile>( m_pFile ) ;
	if ( pmemfile != NULL )
	{
		dwFlags |= 0x01 ;
	}
	file.Write( &dwFlags, sizeof(DWORD) ) ;
	//
	if ( pmemfile != NULL )
	{
		DWORD	dwLength = pmemfile->GetLength() ;
		file.Write( &dwLength, sizeof(DWORD) ) ;
		file.Write( pmemfile->GetBuffer(), dwLength ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Load( ESLFileObject & file, ECSContext & context )
{
	Close( ) ;
	//
	ESLError	err = m_strFileName.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	UINT64		nFilePos ;
	DWORD		dwOpenFlags ;
	int			nCharaEncoding ;
	file.Read( &dwOpenFlags, sizeof(dwOpenFlags) ) ;
	file.Read( &nCharaEncoding, sizeof(nCharaEncoding) ) ;
	file.Read( &nFilePos, sizeof(nFilePos) ) ;
	//
	if ( !m_strFileName.m_varStr.IsEmpty() )
	{
		err = Open
			( m_strFileName.m_varStr,
				(dwOpenFlags & ~ESLFileObject::modeCreateFlag), &context ) ;
		if ( !err )
		{
			m_nCharaEncoding = nCharaEncoding ;
			Seek( nFilePos, ESLFileObject::FromBegin ) ;
		}
	}
	DWORD	dwFlags = 0 ;
	file.Read( &dwFlags, sizeof(DWORD) ) ;
	if ( dwFlags & 0x01 )
	{
		DWORD	dwLength = 0 ;
		file.Read( &dwLength, sizeof(DWORD) ) ;
		CreateMemoryFile( dwLength ) ;
		//
		EStreamBuffer	buf ;
		void *	ptrBuf = buf.PutBuffer( dwLength ) ;
		file.Read( ptrBuf, dwLength ) ;
		//
		Seek( 0, ESLFileObject::FromBegin ) ;
		Write( ptrBuf, dwLength ) ;
		//
		m_nCharaEncoding = nCharaEncoding ;
		Seek( nFilePos, ESLFileObject::FromBegin ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	if ( !m_strFileName.m_varStr.IsEmpty() )
	{
		strDump = "filename = " + EString(m_strFileName.m_varStr) ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// メンバ関数
//////////////////////////////////////////////////////////////////////////////

ECSStrTagArray *	ECSFile::m_staFuncName = NULL ;
const wchar_t *	ECSFile::m_pwszFuncName[40] =
{
	L"Open", L"OpenURL", L"CreateMemoryFile", L"Close",
	L"OpenArchive", L"CloseArchive",
	L"OpenArchiveFile", L"CloseArchiveFile",
	L"GetCharacterEncoding", L"SetCharacterEncoding",
	L"IsEndOfFile", L"SetEndOfFile",
	L"GetCurrentDownloaded", L"GetDownloadingFileLength",
	L"IsFileDownloaded", L"IsFileDownloadFailed", L"CancelFileDownloading",
	L"GetLength", L"GetPosition", L"Seek",
	L"ReadText", L"WriteText", L"Read", L"Write", L"GetFileTime",
	L"LoadContextTitle", L"LoadObject", L"LoadContext",
	L"SaveThumbnailImage", L"SaveObject", L"SaveContext",
	L"DumpObject", L"DumpContext",
	L"IsExisting", L"Rename",
	L"FindFile", L"FindDirectory", L"FilterFilePath",
	NULL
} ;
const ECSFile::PFUNC_CALL	ECSFile::m_pfnCallFunc[39] =
{
	&ECSFile::Call_Open,
	&ECSFile::Call_OpenURL,
	&ECSFile::Call_CreateMemoryFile,
	&ECSFile::Call_Close,
	&ECSFile::Call_OpenArchive,	&ECSFile::Call_CloseArchive,
	&ECSFile::Call_OpenArchiveFile,	&ECSFile::Call_CloseArchiveFile,
	&ECSFile::Call_GetCharacterEncoding,
	&ECSFile::Call_SetCharacterEncoding,
	&ECSFile::Call_IsEndOfFile,	&ECSFile::Call_SetEndOfFile,
	&ECSFile::Call_GetCurrentDownloaded,
	&ECSFile::Call_GetDownloadingFileLength,
	&ECSFile::Call_IsFileDownloaded,
	&ECSFile::Call_IsFileDownloadFailed,
	&ECSFile::Call_CancelFileDownloading,
	&ECSFile::Call_GetLength, &ECSFile::Call_GetPosition, &ECSFile::Call_Seek,
	&ECSFile::Call_ReadText, &ECSFile::Call_WriteText,
	&ECSFile::Call_Read, &ECSFile::Call_Write, &ECSFile::Call_GetFileTime,
	&ECSFile::Call_LoadContextTitle, &ECSFile::Call_LoadObject,
	&ECSFile::Call_LoadContext, &ECSFile::Call_SaveThumbnailImage,
	&ECSFile::Call_SaveObject, &ECSFile::Call_SaveContext,
	&ECSFile::Call_DumpObject, &ECSFile::Call_DumpContext,
	&ECSFile::Call_IsExisting, &ECSFile::Call_Rename,
	&ECSFile::Call_FindFile, &ECSFile::Call_FindDirectory,
	&ECSFile::Call_FilterFilePath,
} ;

// メンバ関数 : Integer Open( String sFileName, [Integer nFlags] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Open
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	int				nOpenFlags ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
		( nOpenFlags, lstArg, 2,
			(ESLFileObject::modeRead | ESLFileObject::shareRead) ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( Open( wstrFileName, nOpenFlags, &context ) ) ) ;
}

// メンバ関数 : Integer OpenURL( String strURL, String strDownloadFile := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenURL
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	ECSWideString	wstrDownloadFile ;
	int				nFlags ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrDownloadFile, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( OpenURL
			( wstrFileName, wstrDownloadFile,
				(unsigned int) nFlags, context.GetEnvironment() ) ) ) ;
}

// メンバ関数 : Integer CreateMemoryFile( [Integer nInitBufSize] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CreateMemoryFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nInitBufSize ;
	err = context.GetArgumentAsInt( nInitBufSize, lstArg, 1, 0x4000 ) ;
	if ( err )
		return	err ;
	//
	err = CreateMemoryFile( nInitBufSize ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Close()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Close
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Close( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer OpenArchive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenArchive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = OpenArchive( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseArchive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CloseArchive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = CloseArchive( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer OpenArchiveFile
//		( String sFilepath, String sPassword, Integer fStream )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenArchiveFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	EWideString	wstrFilePath, wstrPassword ;
	int			fStream ;
	err = context.GetArgumentAsStr( wstrFilePath, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrPassword, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fStream, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	err =
		OpenArchiveFile
			( EString( wstrFilePath ),
				EString( wstrPassword ), fStream != 0 ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseArchiveFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CloseArchiveFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = CloseArchiveFile( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : String GetCharacterEncoding()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetCharacterEncoding
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pstrEncoding = new ECSString ;
	pstrEncoding->m_varStr = GetCharacterEncoding( ) ;
	//
	return	context.PushObject( pstrEncoding ) ;
}

// メンバ関数 : SetCharacterEncoding( String sType )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SetCharacterEncoding
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrType ;
	err = context.GetArgumentAsStr( wstrType, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	SetCharacterEncoding( EString( wstrType ) ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsEndOfFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsEndOfFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( - (int) IsEndOfFile() ) ) ;
}

// メンバ関数 : Integer GetCurrentDownloaded()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetCurrentDownloaded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetCurrentDownloaded() ) ) ;
}

// メンバ関数 : Integer GetDownloadingFileLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetDownloadingFileLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetDownloadingFileLength() ) ) ;
}

// メンバ関数 : Boolean IsFileDownloaded()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsFileDownloaded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (int) IsFileDownloaded() ) ) ;
}

// メンバ関数 : Boolean IsFileDownloadFailed()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsFileDownloadFailed
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (int) IsFileDownloadFailed() ) ) ;
}

// メンバ関数 : CancelFileDownloading()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CancelFileDownloading
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	CancelFileDownloading( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetEndOfFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SetEndOfFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	SetEndOfFile( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetFileLength() ) ) ;
}

// メンバ関数 : Integer GetPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetFilePosition() ) ) ;
}

// メンバ関数 : Integer Seek( Integer nPos, [Integer nOrigin] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Seek
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	INT64	nPos ;
	int		nSeekOrigin ;
	err = context.GetArgumentAsInt64( nPos, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
		( nSeekOrigin, lstArg, 2, ESLFileObject::FromBegin ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( Seek( nPos, nSeekOrigin ) ) ) ;
}

// メンバ関数 : Integer ReadText( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_ReadText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "読込先オブジェクトが指定されていません。" ) ;
	}
	return	context.PushObject( new ECSInteger( ReadText( *pObj ) ) ) ;
}

// メンバ関数 : Integer WriteText( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_WriteText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "書き出しオブジェクトが指定されていません。" ) ;
	}
	return	context.PushObject( new ECSInteger( WriteText( *pObj ) ) ) ;
}

// メンバ関数 : Integer Read( object, Integer nBytes := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Read
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "読込先オブジェクトが指定されていません。" ) ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( ReadBinary( *pObj, nBytes ) ) ) ;
}

// メンバ関数 : Integer Write( object, Integer nBytes := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Write
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "書き出しオブジェクトが指定されていません。" ) ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( WriteBinary( *pObj, nBytes ) ) ) ;
}

// メンバ関数 : Integer GetFileTime
//		( Time & ftCreate, Time & ftLastAccess, Time & ftLastWrite )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetFileTime
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
    ESLError err = context.VerifyArgumentCount(lstArg, 1, 4);
    if (err) return err;
    auto* env = context.GetEnvironment();
    SSystem::SFileOpener::State state{};
    auto result = SSystem::errFailed;
    if (env && !m_strFileName.m_varStr.IsEmpty()) {
        auto* writable = env->GetWritableFileOpener();
        if (writable) result = writable->QueryState(m_strFileName.m_varStr, state);
        if (result) result = env->SSystem::SEnvironment::QueryFileState(m_strFileName.m_varStr, state);
    }
    if (result) return context.PushObject(context.new_CSInteger(eslErrGeneral));
    const SSystem::DATE_TIME* times[] = { &state.dtCreated, &state.dtAccessed, &state.dtModified };
    const uint32_t fields[] = { SSystem::SFileOpener::fieldCreatedTime,
        SSystem::SFileOpener::fieldAccessedTime, SSystem::SFileOpener::fieldModifiedTime };
    for (int i=0; i<3; ++i) {
        auto* value = ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(lstArg,i+1,L"Time"));
        if (!value) continue;
        if (!(state.bitFields & fields[i])) return context.PushObject(context.new_CSInteger(eslErrGeneral));
        const auto& t = *times[i];
        value->SetMemberAsInt(L"nYear", t.nYear); value->SetMemberAsInt(L"nMonth", t.nMonth);
        value->SetMemberAsInt(L"nDay", t.nDay); value->SetMemberAsInt(L"nWeek", t.nWeek);
        value->SetMemberAsInt(L"nHour", t.nHour); value->SetMemberAsInt(L"nMinute", t.nMinute);
        value->SetMemberAsInt(L"nSecond", t.nSecond);
    }
    return context.PushObject(context.new_CSInteger(eslErrSuccess));
}

// メンバ関数 : Integer LoadContextTitle( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadContextTitle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = NULL ;
	err = LoadContextTitle( pObj, context ) ;
	if ( !err && pObj )
	{
		ECSObject *	pDst = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
		if ( pDst != NULL )
		{
			err = pDst->Move( context, pObj ) ;
			if ( err )
			{
				delete	pObj ;
			}
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadObject( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = NULL ;
	err = LoadObject( pObj, context ) ;
	if ( !err && pObj )
	{
		ECSObject *	pDst = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
		if ( pDst != NULL )
		{
			err = pDst->Move( context, pObj ) ;
			if ( err )
			{
				delete	pObj ;
			}
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadContext()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fNoCompressed ;
	err = context.GetArgumentAsInt( fNoCompressed, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	const auto previousStatus = context.GetStatus();
    err = LoadContext( context, (fNoCompressed != 0) ) ;
    // A failed destructive restore has halted the context. Returning an
    // Integer here would continue at the saved IP with an uncommitted stack.
    // Failures before restoration begins still return the original script
    // error value and leave the caller's executing context intact.
    if (err && previousStatus == ECSContext::xsExecution &&
        context.GetStatus() == ECSContext::xsHalt) return err;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SaveThumbnailImage
//		( Reference sprThumbnail, [Integer nWidth, Integer nHeight] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveThumbnailImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
    BeginSaveAttempt();
    if(const auto error=context.VerifyArgumentCount(lstArg,2,4))return error;
    auto *sprite=ESLTypeCast<ECSSprite>(context.GetArgumentObjectAs(lstArg,1,L"Sprite"));
    if(!sprite)return ESLErrorMsg("SaveThumbnailImage requires a Sprite");
    int width=0,height=0;
    if(const auto error=context.GetArgumentAsInt(width,lstArg,2,0))return error;
    if(const auto error=context.GetArgumentAsInt(height,lstArg,3,0))return error;
    return context.PushObject(context.new_CSInteger(SaveThumbnailImage(sprite,width,height)));
}

// メンバ関数 : Integer SaveObject( object, [title] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
    BeginSaveAttempt();
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "引数にオブジェクトが指定されていません。" ) ;
	}
	err = SaveObject( *pObj, ECSObject::GetEntity( lstArg.GetAt(2) ), context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SaveContext( [title] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
    BeginSaveAttempt();
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int	fNoCompress ;
	err = context.GetArgumentAsInt( fNoCompress, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	err = SaveContext
		( ECSObject::GetEntity( lstArg.GetAt(1) ), (fNoCompress != 0), context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer DumpObject( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_DumpObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "引数にオブジェクトが指定されていません。" ) ;
	}
	err = DumpObject( *pObj, context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer DumpContext()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_DumpContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = DumpContext( context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsExisting( String sFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsExisting
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ESLFileObject *	pfile = context.OpenFileOnScript( wstrFileName, 0 ) ;
	ECSInteger *	pResult = new ECSInteger( pfile ? -1 : 0 ) ;
	delete	pfile ;
	return	context.PushObject( pResult ) ;
}

// メンバ関数 : Integer Rename( String sOldFile, String sNewFile )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Rename
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
    ESLError err = context.VerifyArgumentCount(lstArg,2,3);
    if (err) return err;
    ECSWideString oldPath, newPath;
    if ((err=context.GetArgumentAsStr(oldPath,lstArg,1,NULL))) return err;
    if ((err=context.GetArgumentAsStr(newPath,lstArg,2,NULL))) return err;
    auto* env=context.GetEnvironment();
    auto* opener=env ? env->GetWritableFileOpener() : nullptr;
    if (!opener) return context.PushObject(context.new_CSInteger(eslErrGeneral));
    // The writable opener owns the Android save-directory boundary.
    auto result = newPath.IsEmpty() ? opener->RemoveSubFile(oldPath)
                                   : opener->RenameSubFile(oldPath,newPath);
    return context.PushObject(context.new_CSInteger(static_cast<INT64>(result)));
}

// メンバ関数 : Error FindFile( String[]& aFiles, String sFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FindFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	Call_FindFileDirectory( context, lstArg, false ) ;
}

// メンバ関数 : Error FindDirectory( String[]& aDirs, sDirName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FindDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	Call_FindFileDirectory( context, lstArg, true ) ;
}

ESLError ECSFile::Call_FindFileDirectory
	( ECSContext & context,
		ECSObjArray<ECSObject> & lstArg, bool fDirectory )
{
    ESLError err=context.VerifyArgumentCount(lstArg,2,3);
    if (err) return err;
    auto* files=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(lstArg,1,L"Array"));
    if (!files) return ESLErrorMsg("File.FindFile requires an Array");
    ECSWideString pattern;
    if ((err=context.GetArgumentAsStr(pattern,lstArg,2,NULL))) return err;
    auto* env=context.GetEnvironment();
    auto* opener=env ? env->GetWritableFileOpener() : nullptr;
    if (!opener) return context.PushObject(context.new_CSInteger(eslErrGeneral));
    EWideString directory=pattern.GetFileDirectoryPart();
    EWideString wildcard=pattern.GetFileNamePart();
    if (wildcard.IsEmpty()) wildcard=L"*";
    SSystem::SObjectArray<SSystem::SString> names;
    if (fDirectory) opener->ListSubDirectories(names,directory);
    else opener->ListSubFiles(names,directory);
    for (size_t i=0;i<names.GetLength();++i) {
        const auto* name=names.GetAt(i);
        if (name && SSystem::SFileOpener::IsMatchWildCardTo(wildcard,*name))
            files->m_varArray.Add(new ECSString(ECSWideString(*name)));
    }
    return context.PushObject(context.new_CSInteger(eslErrSuccess));
}

// メンバ関数 : String FilterFilePath( String sFilePath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FilterFilePath
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFilePath ;
	err = context.GetArgumentAsStr( wstrFilePath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	//
	ECSString *	pStrResult = context.new_CSString() ;
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		pStrResult->m_varStr = pEnv->FilterFilePath( wstrFilePath ) ;
	}
	else
	{
		pStrResult->m_varStr = wstrFilePath ;
	}
	return	context.PushObject( pStrResult ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSFile::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_FILE_INTERFACE" ) )
	{
		if ( m_ppif == NULL )
		{
			m_ppif = (PLUGIN_FILE*)
				::eslHeapAllocate( NULL, sizeof(PLUGIN_FILE), 0 ) ;
			m_ppif->pBackLink = this ;
			m_ppif->pfnOpen = PIC_Open ;
			m_ppif->pfnClose = PIC_Close ;
			m_ppif->pfnSetCharacterEncoding = PIC_SetCharacterEncoding ;
			m_ppif->pfnGetCharacterEncoding = PIC_GetCharacterEncoding ;
			m_ppif->pfnGetFileLength = PIC_GetFileLength ;
			m_ppif->pfnGetFilePosition = PIC_GetFilePosition ;
			m_ppif->pfnSeek = PIC_Seek ;
			m_ppif->pfnIsEndOfFile = PIC_IsEndOfFile ;
			m_ppif->pfnSetEndOfFile = PIC_SetEndOfFile ;
			m_ppif->pfnReadText = PIC_ReadText ;
			m_ppif->pfnWriteText = PIC_WriteText ;
			m_ppif->pfnReadBinary = PIC_ReadBinary ;
			m_ppif->pfnWriteBinary = PIC_WriteBinary ;
			m_ppif->pfnLoadContextTitle = PIC_LoadContextTitle ;
			m_ppif->pfnLoadObject = PIC_LoadObject ;
			m_ppif->pfnLoadContext = PIC_LoadContext ;
			m_ppif->pfnSaveThumbnailImage = PIC_SaveThumbnailImage ;
			m_ppif->pfnSaveObject = PIC_SaveObject ;
			m_ppif->pfnSaveContext = PIC_SaveContext ;
			m_ppif->pfnDumpObject = PIC_DumpObject ;
			m_ppif->pfnDumpContext = PIC_DumpContext ;
			m_ppif->pfnGetFile = PIC_GetFile ;
		}
		return	(ECS_FILE_INTERFACE*) m_ppif ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

ESLError __stdcall ECSFile::PIC_Open
	( ECS_FILE_INTERFACE * instance,
		const wchar_t * pwszFileName,
		DWORD dwOpenFlags, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	FileFromPlugin(instance)->
				Open( pwszFileName, dwOpenFlags, context ) ;
}

ESLError __stdcall ECSFile::PIC_Close( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->Close( ) ;
}

ESLError __stdcall ECSFile::PIC_SetCharacterEncoding
	( ECS_FILE_INTERFACE * instance, const char * pszType )
{
	return	FileFromPlugin(instance)->SetCharacterEncoding( pszType ) ;
}

const char * __stdcall ECSFile::PIC_GetCharacterEncoding
	( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->GetCharacterEncoding( ) ;
}

unsigned long int __stdcall ECSFile::PIC_GetFileLength
	( ECS_FILE_INTERFACE * instance )
{
	return	(unsigned long int) FileFromPlugin(instance)->GetFileLength( ) ;
}

unsigned long int __stdcall ECSFile::PIC_GetFilePosition
	( ECS_FILE_INTERFACE * instance )
{
	return	(unsigned long int) FileFromPlugin(instance)->GetFilePosition( ) ;
}

unsigned long int __stdcall ECSFile::PIC_Seek
	( ECS_FILE_INTERFACE * instance, long int nPos, int nSeekType )
{
	return	(unsigned long int) FileFromPlugin(instance)->Seek( nPos, nSeekType ) ;
}

int __stdcall ECSFile::PIC_IsEndOfFile( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->IsEndOfFile( ) ;
}

void __stdcall ECSFile::PIC_SetEndOfFile( ECS_FILE_INTERFACE * instance )
{
	FileFromPlugin(instance)->SetEndOfFile( ) ;
}

ESLError __stdcall ECSFile::PIC_ReadText
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->ReadText( *obj ) ;
}

unsigned long int __stdcall ECSFile::PIC_WriteText
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->WriteText( *obj ) ;
}

unsigned long int __stdcall ECSFile::PIC_ReadBinary
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->ReadBinary( *obj, nBytes ) ;
}

unsigned long int __stdcall ECSFile::PIC_WriteBinary
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->WriteBinary( *obj, nBytes ) ;
}

ESLError __stdcall ECSFile::PIC_LoadContextTitle
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT **pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj ;
	ESLError		err =
		FileFromPlugin(instance)->LoadContextTitle( obj, *context ) ;
	if ( !err )
	{
		*pObj = obj->CreateInterface( ) ;
	}
	return	err ;
}

ESLError __stdcall ECSFile::PIC_LoadObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT **pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj ;
	ESLError		err =
		FileFromPlugin(instance)->LoadObject( obj, *context ) ;
	if ( !err )
	{
		*pObj = obj->CreateInterface( ) ;
	}
	return	err ;
}

ESLError __stdcall ECSFile::PIC_LoadContext
	( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	return	FileFromPlugin(instance)->LoadContext( *context, true ) ;
}

ESLError __stdcall ECSFile::PIC_SaveThumbnailImage
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pPreview, int nWidth, int nHeight )
{
    auto *sprite=pPreview?ESLTypeCast<ECSSprite>(ECSObject::GetEntity(ObjectFromPlugin(pPreview))):nullptr;
    return FileFromPlugin(instance)->SaveThumbnailImage(sprite,nWidth,nHeight);
}

ESLError __stdcall ECSFile::PIC_SaveObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pObj, ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj = ObjectFromPlugin( pObj ) ;
	ECSObject *		pTitleObj = NULL ;
	if ( pTitle != NULL )
	{
		pTitleObj = ObjectFromPlugin( pTitle ) ;
	}
	return	FileFromPlugin(instance)->
				SaveObject( *obj, pTitleObj, *context ) ;
}

ESLError __stdcall ECSFile::PIC_SaveContext
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		pTitleObj = NULL ;
	if ( pTitle != NULL )
	{
		pTitleObj = ObjectFromPlugin( pTitle ) ;
	}
	return	FileFromPlugin(instance)->SaveContext( pTitleObj, true, *context ) ;
}

ESLError __stdcall ECSFile::PIC_DumpObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->DumpObject( *obj, *context ) ;
}

ESLError __stdcall ECSFile::PIC_DumpContext
	( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	return	FileFromPlugin(instance)->DumpContext( *context ) ;
}

ECS_FILE * __stdcall ECSFile::PIC_GetFile( ECS_FILE_INTERFACE * instance )
{
	ECSFile *	pcsf = FileFromPlugin(instance) ;
	if ( pcsf->m_pFile == NULL )
	{
		return	NULL ;
	}
	return	new ECSFilePIInterface( pcsf->m_pFile ) ;
}
