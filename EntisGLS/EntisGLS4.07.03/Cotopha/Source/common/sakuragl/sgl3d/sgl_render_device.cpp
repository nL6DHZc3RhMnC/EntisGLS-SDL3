
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// シェーダー・シリアライザ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DShaderBinary, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinary::S3DShaderBinary( void )
{
	m_flagAttrModified = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinary::~S3DShaderBinary( void )
{
}

// プログラム識別子取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DShaderBinary::GetIdentity( void ) const
{
	return	m_strID ;
}

// プログラム識別子設定
//////////////////////////////////////////////////////////////////////////////
void S3DShaderBinary::SetIdentity( const wchar_t * pwszID )
{
	m_strID = pwszID ;
}

// バイナリ設定
//////////////////////////////////////////////////////////////////////////////
void S3DShaderBinary::SetBinary
	( const S3DShaderBinary::Format * pFormat, size_t nFormatBytes,
		const void * pProgram, size_t nProgramBytes )
{
	m_bufFormat.RemoveAll() ;
	m_bufBinary.RemoveAll() ;
	//
	eslCopyMemory
		( m_bufFormat.GetArray( nFormatBytes ), pFormat, nFormatBytes ) ;
	eslCopyMemory
		( m_bufBinary.GetArray( nProgramBytes ), pProgram, nProgramBytes ) ;
	//
	m_bufFormat.FinishArray() ;
	m_bufBinary.FinishArray() ;
}

// フォーマット取得
//////////////////////////////////////////////////////////////////////////////
const S3DShaderBinary::Format *
	S3DShaderBinary::GetFormat( size_t& nBytes ) const
{
	nBytes = m_bufFormat.GetLength() ;
	return	(const Format*) m_bufFormat.GetConstArray() ;
}

// バイナリ取得
//////////////////////////////////////////////////////////////////////////////
const void * S3DShaderBinary::GetBinrary( size_t& nBytes ) const
{
	nBytes = m_bufBinary.GetLength() ;
	return	m_bufBinary.GetConstArray() ;
}

// 属性値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DShaderBinary::GetAttrStringAs
	( const wchar_t * pwszName, const wchar_t * pwszDefValue ) const
{
	SString *	pstrValue = m_ssoaAttr.GetAs( pwszName ) ;
	if ( pstrValue != NULL )
	{
		return	*pstrValue ;
	}
	return	pwszDefValue ;
}

int64_t S3DShaderBinary::GetAttrIntegerAs
	( const wchar_t * pwszName, int64_t nDefValue ) const
{
	SString *	pstrValue = m_ssoaAttr.GetAs( pwszName ) ;
	if ( pstrValue != NULL )
	{
		return	pstrValue->AsInteger() ;
	}
	return	nDefValue ;
}

// 属性値設定
//////////////////////////////////////////////////////////////////////////////
void S3DShaderBinary::SetAttrStringAs
	( const wchar_t * pwszName, const wchar_t * pwszValue )
{
	m_flagAttrModified = true ;
	//
	SString *	pstrValue = m_ssoaAttr.GetAs( pwszName ) ;
	if ( pstrValue != NULL )
	{
		*pstrValue = pwszValue ;
	}
	else
	{
		pstrValue = new SString( pwszValue ) ;
		m_ssoaAttr.SetAs( pwszName, pstrValue ) ;
	}
}

void S3DShaderBinary::SetAttrIntegerAs
	( const wchar_t * pwszName, int64_t nValue )
{
	m_flagAttrModified = true ;
	//
	SString *	pstrValue = m_ssoaAttr.GetAs( pwszName ) ;
	if ( pstrValue == NULL )
	{
		pstrValue = new SString ;
		m_ssoaAttr.SetAs( pwszName, pstrValue ) ;
	}
	pstrValue->FromInteger( nValue ) ;
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinary::Load( SSystem::SFileInterface& file )
{
	Header	hdr ;
	if ( file.Read( &hdr, sizeof(Header) ) < sizeof(Header) )
	{
		return	sglErrFailed ;
	}
	if ( (hdr.nIDLength == 0)
		|| (hdr.nFormatBytes == 0)
		|| (hdr.nIDLength & 0xFF000000)
		|| (hdr.nFormatBytes & 0xFF000000)
		|| (hdr.nBinaryBytes & 0xFF000000)
		|| (hdr.nAttrLength & 0xFF000000))
	{
		return	sglErrFailed ;
	}
	size_t	nReadBytes ;
	nReadBytes = file.Read
		( m_strID.LockBuffer
			( (size_t) hdr.nIDLength ),
				(size_t) hdr.nIDLength * sizeof(uint16_t) ) ;
	m_strID.UnlockBuffer( (ssize_t) hdr.nIDLength ) ;
	if ( nReadBytes != (size_t) hdr.nIDLength * sizeof(uint16_t) )
	{
		return	sglErrFailed ;
	}
	//
	nReadBytes = file.Read
		( m_bufFormat.GetArray
			( (size_t) hdr.nFormatBytes ), (size_t) hdr.nFormatBytes ) ;
	m_bufFormat.FinishArray() ;
	if ( nReadBytes != (size_t) hdr.nFormatBytes )
	{
		return	sglErrFailed ;
	}
	//
	nReadBytes = file.Read
		( m_bufBinary.GetArray
			( (size_t) hdr.nBinaryBytes ), (size_t) hdr.nBinaryBytes ) ;
	m_bufBinary.FinishArray() ;
	if ( nReadBytes != (size_t) hdr.nBinaryBytes )
	{
		return	sglErrFailed ;
	}
	//
	nReadBytes = file.Read
		( m_bufAttr.GetArray
			( (size_t) hdr.nAttrLength + 2 ),
				(size_t) hdr.nAttrLength * sizeof(uint16_t) ) ;
	m_bufAttr.FinishArray() ;
	if ( nReadBytes != (size_t) hdr.nAttrLength * sizeof(uint16_t) )
	{
		return	sglErrFailed ;
	}
	//
	m_flagAttrModified = false ;
	m_ssoaAttr.RemoveAll() ;
	//
	const uint16_t *	pwAttr = m_bufAttr.GetConstArray() ;
	size_t				nAttrLen = m_bufAttr.GetLength() ;
	for ( size_t i = 0; i < nAttrLen; i ++ )
	{
		SString	strName = pwAttr + i ;
		i += strName.GetLength() + 1 ;
		//
		SString *	pstrValue = new SString( pwAttr + i ) ;
		i += pstrValue->GetLength() + 1 ;
		//
		m_ssoaAttr.SetAs( strName, pstrValue ) ;
	}
	return	sglErrSuccess ;
}

// 書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinary::Save( SSystem::SFileInterface& file )
{
	if ( m_flagAttrModified )
	{
		m_bufAttr.RemoveAll() ;
		//
		for ( size_t i = 0; i < m_ssoaAttr.GetLength(); i ++ )
		{
			const SString *	pstrTag = m_ssoaAttr.GetTagAt( i ) ;
			SString *		pstrValue = m_ssoaAttr.GetAt( i ) ;
			if ( pstrTag && pstrValue )
			{
				m_bufAttr.AddArray
					( pstrTag->GetConstArray(), pstrTag->GetLength() ) ;
				m_bufAttr.Add( 0 ) ;
				m_bufAttr.AddArray
					( pstrValue->GetConstArray(), pstrValue->GetLength() ) ;
				m_bufAttr.Add( 0 ) ;
			}
		}
		//
		m_flagAttrModified = false ;
	}
	//
	Header	hdr ;
	hdr.nIDLength = (uint32_t) m_strID.GetLength() ;
	hdr.nFormatBytes = (uint32_t) m_bufFormat.GetLength() ;
	hdr.nBinaryBytes = (uint32_t) m_bufBinary.GetLength() ;
	hdr.nAttrLength = (uint32_t) m_bufAttr.GetLength() ;
	//
	if ( file.Write( &hdr, sizeof(Header) ) < sizeof(Header) )
	{
		return	sglErrFailed ;
	}
	//
	file.Write
		( m_strID.GetConstArray(), m_strID.GetLength() * sizeof(uint16_t) ) ;
	file.Write
		( m_bufFormat.GetConstArray(), m_bufFormat.GetLength() ) ;
	file.Write
		( m_bufBinary.GetConstArray(), m_bufBinary.GetLength() ) ;
	file.Write
		( m_bufAttr.GetConstArray(), m_bufAttr.GetLength() * sizeof(uint16_t) ) ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// GLSL バイナリマネージャ
//////////////////////////////////////////////////////////////////////////////

S3DShaderBinaryLibrary *	S3DShaderBinaryLibrary::m_pInstance = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DShaderBinaryLibrary, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinaryLibrary::S3DShaderBinaryLibrary( void )
{
	m_flagModified = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinaryLibrary::~S3DShaderBinaryLibrary( void )
{
}

// デバイス情報取得
//////////////////////////////////////////////////////////////////////////////
const S3DShaderBinaryLibrary::DeviceInfo *
	S3DShaderBinaryLibrary::GetDeviceInfo
		( S3DShaderBinary::DeivceType typeDev ) const
{
	SSmartLock<SCriticalSection>	lock( (SCriticalSection*) &m_csSync ) ;
	for ( size_t i = 0; i < m_arrDevice.GetLength(); i ++ )
	{
		DeviceInfo *	pdi = m_arrDevice.GetAt( i ) ;
		if ( pdi && (pdi->m_typeDev == typeDev) )
		{
			return	pdi ;
		}
	}
	return	NULL ;
}

// デバイス情報設定
//////////////////////////////////////////////////////////////////////////////
void S3DShaderBinaryLibrary::SetDeviceInfo
	( S3DShaderBinary::DeivceType typeDev,
		const wchar_t * pwszVender,
		const wchar_t * pwszRenderer,
		const wchar_t * pwszVersion )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	DeviceInfo *	pdi = (DeviceInfo*) GetDeviceInfo( typeDev ) ;
	if ( pdi == NULL )
	{
		pdi = new DeviceInfo ;
		pdi->m_typeDev = typeDev ;
		m_arrDevice.Add( pdi ) ;
	}
	pdi->m_strVender = pwszVender ;
	pdi->m_strRenderer = pwszRenderer ;
	pdi->m_strVersion = pwszVersion ;
}

// バイナリ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DShaderBinaryLibrary::FindBinary
	( const wchar_t * pwszID, S3DShaderBinary::DeivceType typeDev ) const
{
	SSmartLock<SCriticalSection>	lock( (SCriticalSection*) &m_csSync ) ;
	for ( size_t i = 0; i < m_arrBinary.GetLength(); i ++ )
	{
		S3DShaderBinary *	psb = m_arrBinary.GetAt( i ) ;
		if ( psb && (psb->GetIdentity() == pwszID) )
		{
			size_t	nFormatBytes ;
			const S3DShaderBinary::Format *
					pFormat = psb->GetFormat( nFormatBytes ) ;
			if ( pFormat && (pFormat->typeDev == typeDev) )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}

// バイナリ取得
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinary *
	S3DShaderBinaryLibrary::GetBinary
		( const wchar_t * pwszID, S3DShaderBinary::DeivceType typeDev ) const
{
	SSmartLock<SCriticalSection>	lock( (SCriticalSection*) &m_csSync ) ;
	return	m_arrBinary.GetAt( (size_t) FindBinary( pwszID, typeDev ) ) ;
}

// バイナリ追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinaryLibrary::AddBinary( S3DShaderBinary * pBinary )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ESLAssert( pBinary != NULL ) ;
	if ( pBinary == NULL )
	{
		return	sglErrFailed ;
	}
	size_t	nFormatBytes ;
	const S3DShaderBinary::Format *
			pFormat = pBinary->GetFormat( nFormatBytes ) ;
	if ( pFormat == NULL )
	{
		return	sglErrFailed ;
	}
	ssize_t	i =
		FindBinary
			( pBinary->GetIdentity(),
				(S3DShaderBinary::DeivceType) pFormat->typeDev ) ;
	if ( i >= 0 )
	{
		m_arrBinary.RemoveAt( (size_t) i ) ;
	}
	m_arrBinary.Add( pBinary ) ;
	m_flagModified = true ;
	return	sglErrSuccess ;
}

// バイナリ削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinaryLibrary::RemoveDeviceBinary
		( S3DShaderBinary::DeivceType typeDev )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	bool	fRemoved = false ;
	for ( size_t i = 0; i < m_arrBinary.GetLength(); i ++ )
	{
		S3DShaderBinary *	psb = m_arrBinary.GetAt( i ) ;
		if ( psb != NULL )
		{
			size_t	nFormatBytes ;
			const S3DShaderBinary::Format *
					pFormat = psb->GetFormat( nFormatBytes ) ;
			if ( pFormat && (pFormat->typeDev == typeDev) )
			{
				m_arrBinary.SetAt( i, NULL ) ;
				fRemoved = true ;
			}
		}
	}
	if ( fRemoved )
	{
		m_arrBinary.TrimEmpty() ;
	}
	return	sglErrSuccess ;
}

// バイナリが登録されていないか？
//////////////////////////////////////////////////////////////////////////////
bool S3DShaderBinaryLibrary::IsEmpty( void ) const
{
	return	(m_arrBinary.GetLength() == 0) ;
}

// 変更されたか？
//////////////////////////////////////////////////////////////////////////////
bool S3DShaderBinaryLibrary::IsModified( void ) const
{
	return	m_flagModified ;
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinaryLibrary::Load
	( SSystem::SFileInterface& file, uint32_t nShaderVer )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_arrDevice.RemoveAll() ;
	m_arrBinary.RemoveAll() ;
	m_flagModified = false ;
	//
	// アプリケーション・シェーダー・バージョン
	//
	uint32_t	nSaveShaderVer ;
	if ( file.Read( &nSaveShaderVer, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	if ( nSaveShaderVer != nShaderVer )
	{
		return	sglErrFailed ;
	}
	//
	// デバイス情報取得
	//
	uint32_t	nDevInfos ;
	if ( file.Read( &nDevInfos, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( uint32_t i = 0; i < nDevInfos; i ++ )
	{
		uint32_t	typeDev ;
		SString		strVender, strRenderer, strVersion ;
		if ( (file.Read( &typeDev, sizeof(uint32_t) ) < sizeof(uint32_t))
			|| file.ReadString( strVender )
			|| file.ReadString( strRenderer )
			|| file.ReadString( strVersion ) )
		{
			return	sglErrFailed ;
		}
		DeviceInfo *	pdi = new DeviceInfo ;
		pdi->m_typeDev = (S3DShaderBinary::DeivceType) typeDev ;
		pdi->m_strVender = strVender ;
		pdi->m_strRenderer = strRenderer ;
		pdi->m_strVersion = strVersion ;
		m_arrDevice.Add( pdi ) ;
	}
	//
	// プログラムバイナリ
	//
	for ( ; ; )
	{
		S3DShaderBinary *	psb = new S3DShaderBinary ;
		if ( psb->Load( file ) )
		{
			Trace( "failed to load shader binary.\n" ) ;
			delete	psb ;
			break ;
		}
		m_arrBinary.Add( psb ) ;
	}
	return	sglErrSuccess ;
}

// 書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShaderBinaryLibrary::Save
	( SSystem::SFileInterface& file, uint32_t nShaderVer )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	// アプリケーション・シェーダー・バージョン
	//
	file.Write( &nShaderVer, sizeof(uint32_t) ) ;
	//
	// デバイス情報取得
	//
	uint32_t	nDevInfos = (uint32_t) m_arrDevice.GetLength() ;
	if ( file.Write( &nDevInfos, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( size_t i = 0; i < nDevInfos; i ++ )
	{
		DeviceInfo *	pdi = m_arrDevice.GetAt( i ) ;
		ESLAssert( pdi != NULL ) ;
		uint32_t	typeDev = (uint32_t) pdi->m_typeDev ;
		file.Write( &typeDev, sizeof(uint32_t) ) ;
		file.WriteString( pdi->m_strVender ) ;
		file.WriteString( pdi->m_strRenderer ) ;
		file.WriteString( pdi->m_strVersion ) ;
	}
	//
	// プログラムバイナリ
	//
	for ( size_t i = 0; i < m_arrBinary.GetLength(); i ++ )
	{
		S3DShaderBinary *	psb = m_arrBinary.GetAt( i ) ;
		if ( psb )
		{
			psb->Save( file ) ;
		}
	}
	m_flagModified = false ;
	return	sglErrSuccess ;
}

// グローバルインスタンス
//////////////////////////////////////////////////////////////////////////////
S3DShaderBinaryLibrary * S3DShaderBinaryLibrary::GetInstance( void )
{
	return	m_pInstance ;
}

void S3DShaderBinaryLibrary::SetInstance( S3DShaderBinaryLibrary * pInstance )
{
	m_pInstance = pInstance ;
}



//////////////////////////////////////////////////////////////////////////////
// 抽象カスタムシェーダー
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCustomShader, SObject )

const size_t	S3DCustomShader::m_bytesUniformType
						[S3DCustomShader::uniformTypeCount] =
{
	sizeof(int32_t), sizeof(float32_t), 2*sizeof(float32_t),
	3*sizeof(float32_t), 4*sizeof(float32_t),
	2*2*sizeof(float32_t), 3*3*sizeof(float32_t),
	4*4*sizeof(float32_t), sizeof(SGLImageObject*),
	sizeof(SGLImageObject*), sizeof(SGLImageObject*), sizeof(SGLImageObject*),
} ;

bool S3DCustomShader::IsUniformTypeTexture( UniformType type )
{
	return	(type == uniformTexture)
			|| (type == uniformImageRead)
			|| (type == uniformImageWrite)
			|| (type == uniformImageReadWrite) ;
}

// カスタムシェーダーパラメータ
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader::UniformData::UniformData( void )
{
	m_type = S3DCustomShader::uniformInt ;
	m_nLength = 0 ;
	m_pData = NULL ;
	m_flagOwnData = false ;
	m_flagUpdateData = false ;
}

S3DCustomShader::UniformData::UniformData( const S3DCustomShader::UniformData& ud )
{
	m_type = S3DCustomShader::uniformInt ;
	m_nLength = 0 ;
	m_pData = nullptr ;
	m_flagOwnData = false ;
	m_flagUpdateData = false ;
	//
	if ( (ud.m_pData != nullptr) && (ud.m_nLength > 0) )
	{
		SetType( ud.m_type, ud.m_nLength ) ;
		SetData( ud.m_type, ud.m_pData, ud.m_nLength ) ;
	}
}

S3DCustomShader::UniformData::~UniformData( void )
{
	if ( m_flagOwnData )
	{
		ESLAssert( m_pData != (void*) &m_bufData[0] ) ;
		esl_free( m_pData ) ;
		m_pData = NULL ;
		m_flagOwnData = false ;
	}
}

void S3DCustomShader::UniformData::SetType
	( S3DCustomShader::UniformType type, size_t nLength )
{
	size_t	nBytes = m_bytesUniformType[type] * nLength ;
	if ( nBytes <= sizeof(m_bufData) )
	{
		if ( m_flagOwnData && (m_pData != nullptr) )
		{
			esl_free( m_pData ) ;
		}
		m_pData = &m_bufData[0] ;
		m_flagOwnData = false ;
	}
	else
	{
		if ( !m_flagOwnData
			|| (m_pData == nullptr) || (m_nLength < nLength) )
		{
			if ( m_flagOwnData )
			{
				esl_free( m_pData ) ;
			}
			m_pData = esl_malloc( nBytes ) ;
			m_flagOwnData = true ;
		}
	}
	m_type = type ;
	m_nLength = nLength ;
	eslFillMemory( m_pData, 0, GetDataBytes() ) ;
	m_flagUpdateData = true ;
}

bool S3DCustomShader::UniformData::SetData
	( S3DCustomShader::UniformType type,
		const void * pData, size_t nLength )
{
	if ( (m_type != type) || (m_nLength != nLength) )
	{
		SetType( type, nLength ) ;
	}
	ESLAssert( m_type == type ) ;
	size_t	nDataBytes = m_bytesUniformType[type] * nLength ;
	size_t	nBufBytes = m_bytesUniformType[m_type] * m_nLength ;
	ESLAssert( nDataBytes <= nBufBytes ) ;
	if ( nDataBytes > nBufBytes )
	{
		nDataBytes = nBufBytes ;
	}
	if ( eslCompareMemory( m_pData, pData, nDataBytes ) == 0 )
	{
		return	false ;
	}
	eslCopyMemory( m_pData, pData, nDataBytes ) ;
	m_flagUpdateData = true ;
	return	true ;
}

bool S3DCustomShader::UniformData::UpdateData
	( S3DCustomShader::UniformType type,
		const void * pData, size_t nLength )
{
	if ( (m_type != type)
		&& !IsUniformTypeTexture(type)
		&& !IsUniformTypeTexture(m_type) )
	{
		return	false ;
	}
	ESLAssert( m_type == type ) ;
	size_t	nDataBytes = m_bytesUniformType[type] * nLength ;
	size_t	nBufBytes = m_bytesUniformType[m_type] * m_nLength ;
	ESLAssert( nDataBytes <= nBufBytes ) ;
	if ( nDataBytes > nBufBytes )
	{
		nDataBytes = nBufBytes ;
	}
	if ( eslCompareMemory( m_pData, pData, nDataBytes ) == 0 )
	{
		return	false ;
	}
	eslCopyMemory( m_pData, pData, nDataBytes ) ;
	m_flagUpdateData = true ;
	return	true ;
}

bool S3DCustomShader::UniformData::SetDataInt( const int32_t * pData, size_t nLength )
{
	return	SetData( uniformInt, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataFloat( const float32_t * pData, size_t nLength )
{
	return	SetData( uniformFloat, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataVector2D( const S2DVector * pData, size_t nLength )
{
	return	SetData( uniformVector2D, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataVector3D( const S3DVector * pData, size_t nLength )
{
	return	SetData( uniformVector3D, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataVector4D( const S4DVector * pData, size_t nLength )
{
	return	SetData( uniformVector4D, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataMatrix3x3( const S3DMatrix * pData, size_t nLength )
{
	if ( nLength == 1 )
	{
		float32_t	mat3[3][3] ;
		for ( int i = 0; i < 3; i ++ )
		{
			mat3[i][0] = pData->m[i][0] ;
			mat3[i][1] = pData->m[i][1] ;
			mat3[i][2] = pData->m[i][2] ;
		}
		return	SetData( uniformMatrix3x3, &mat3[0][0], nLength ) ;
	}
	else
	{
		SArray<float32_t>	bufMatrix ;
		float32_t *	pMatrix = bufMatrix.GetArray( nLength * (3 * 3) ) ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			for ( int j = 0; j < 3; j ++ )
			{
				pMatrix[0] = pData->m[j][0] ;
				pMatrix[1] = pData->m[j][1] ;
				pMatrix[2] = pData->m[j][2] ;
				pMatrix += 3 ;
			}
			pData ++ ;
		}
		bufMatrix.FinishArray() ;
		return	SetData( uniformMatrix3x3, bufMatrix.GetConstArray(), nLength ) ;
	}
}

bool S3DCustomShader::UniformData::SetDataMatrix4x4( const S4DMatrix * pData, size_t nLength )
{
	return	SetData( uniformMatrix4x4, pData, nLength ) ;
}

bool S3DCustomShader::UniformData::SetDataTexture( const SGLImageObject* pImage )
{
	return	SetData( uniformTexture, &pImage, 1 ) ;
}

bool S3DCustomShader::UniformData::SetDataImage( UniformType type, const SGLImageObject* pImage )
{
	return	SetData( type, &pImage, 1 ) ;
}

// Uniform データ設定
//////////////////////////////////////////////////////////////////////////////
bool S3DCustomShader::UniformSet::SetDataAs
	( const wchar_t * pwszID,
		S3DCustomShader::UniformType type,
		const void * pData, size_t nLength )
{
	UniformData *	pud = SStrSortObjectArray<UniformData>::GetAs( pwszID ) ;
	if ( pud == nullptr )
	{
		pud = new UniformData ;
		SStrSortObjectArray<UniformData>::SetAs( pwszID, pud ) ;
	}
	return	pud->SetData( type, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataIntAs
	( const wchar_t * pwszID, const int32_t * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformInt, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataFloatAs
	( const wchar_t * pwszID, const float32_t * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformFloat, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataVector2DAs
	( const wchar_t * pwszID,
		const S2DVector * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformVector2D, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataVector3DAs
	( const wchar_t * pwszID,
		const S3DVector * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformVector3D, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataVector4DAs
	( const wchar_t * pwszID,
		const S4DVector * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformVector4D, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataMatrix3x3As
	( const wchar_t * pwszID,
		const S3DMatrix * pData, size_t nLength )
{
	if ( nLength == 1 )
	{
		float32_t	mat3[3][3] ;
		for ( int i = 0; i < 3; i ++ )
		{
			mat3[i][0] = pData->m[i][0] ;
			mat3[i][1] = pData->m[i][1] ;
			mat3[i][2] = pData->m[i][2] ;
		}
		return	SetDataAs( pwszID, uniformMatrix3x3, &mat3[0][0], nLength ) ;
	}
	else
	{
		SArray<float32_t>	bufMatrix ;
		float32_t *	pMatrix = bufMatrix.GetArray( nLength * (3 * 3) ) ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			for ( int j = 0; j < 3; j ++ )
			{
				pMatrix[0] = pData->m[j][0] ;
				pMatrix[1] = pData->m[j][1] ;
				pMatrix[2] = pData->m[j][2] ;
				pMatrix += 3 ;
			}
			pData ++ ;
		}
		bufMatrix.FinishArray() ;
		return	SetDataAs( pwszID, uniformMatrix3x3, bufMatrix.GetConstArray(), nLength ) ;
	}
}

bool S3DCustomShader::UniformSet::SetDataMatrix4x4As
	( const wchar_t * pwszID,
		const S4DMatrix * pData, size_t nLength )
{
	return	SetDataAs( pwszID, uniformMatrix4x4, pData, nLength ) ;
}

bool S3DCustomShader::UniformSet::SetDataTextureAs
	( const wchar_t * pwszID, const SGLImageObject* pImage )
{
	return	SetDataAs( pwszID, uniformTexture, &pImage, 1 ) ;
}

bool S3DCustomShader::UniformSet::SetDataImageAs
	( const wchar_t * pwszID,
		UniformType type, const SGLImageObject* pImage )
{
	return	SetDataAs( pwszID, type, &pImage, 1 ) ;
}


// ユニフォーム指標定義
//////////////////////////////////////////////////////////////////////////////
void S3DCustomShader::RegisterCustomUniforms
	( const S3DCustomShader::UniformEntry * pUniformEntries, size_t nEntryCount )
{
	for ( size_t i = 0; i < nEntryCount; i ++ )
	{
		RegisterCustomUniform
			( pUniformEntries[i].id,
				pUniformEntries[i].type, pUniformEntries[i].count ) ;
	}
}

// ユニフォーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCustomShader::SetCustomUniformAs
	( const wchar_t * pszUniform,
		S3DCustomShader::UniformType type,
		const void * pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ), type, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformInt
	( size_t iUniform, const int32_t* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformInt, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformIntAs
	( const wchar_t * pszUniform,
		const int32_t* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformInt, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformFloat
	( size_t iUniform, const float32_t* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformFloat, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformFloatAs
	( const wchar_t * pszUniform,
		const float32_t* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformFloat, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector2D
	( size_t iUniform, const S2DVector* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformVector2D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector2DAs
	( const wchar_t * pszUniform,
		const S2DVector* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformVector2D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector3D
	( size_t iUniform, const S3DVector* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformVector3D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector3DAs
	( const wchar_t * pszUniform,
		const S3DVector* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformVector3D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector4D
	( size_t iUniform, const S4DVector* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformVector4D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformVector4DAs
	( const wchar_t * pszUniform, const S4DVector* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformVector4D, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformMatrix3x3
	( size_t iUniform,
		const S3DMatrix* pData, size_t nCount )
{
	if ( nCount == 1 )
	{
		float32_t	mat3[3][3] ;
		for ( int i = 0; i < 3; i ++ )
		{
			mat3[i][0] = pData->m[i][0] ;
			mat3[i][1] = pData->m[i][1] ;
			mat3[i][2] = pData->m[i][2] ;
		}
		return	SetCustomUniform
					( iUniform, uniformMatrix3x3, &mat3[0][0], nCount ) ;
	}
	else
	{
		SArray<float32_t>	bufMatrix ;
		float32_t *	pMatrix = bufMatrix.GetArray( nCount * (3 * 3) ) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			for ( int j = 0; j < 3; j ++ )
			{
				pMatrix[0] = pData->m[j][0] ;
				pMatrix[1] = pData->m[j][1] ;
				pMatrix[2] = pData->m[j][2] ;
				pMatrix += 3 ;
			}
			pData ++ ;
		}
		bufMatrix.FinishArray() ;
		return	SetCustomUniform
					( iUniform, uniformMatrix3x3,
							bufMatrix.GetConstArray(), nCount ) ;
	}
}

SGLError S3DCustomShader::SetCustomUniformMatrix3x3As
	( const wchar_t * pszUniform,
		const S3DMatrix* pData, size_t nCount )
{
	return	SetCustomUniformMatrix3x3
		( (size_t) FindCustomUniform( pszUniform ), pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformMatrix4x4
	( size_t iUniform, const S4DMatrix* pData, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformMatrix4x4, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformMatrix4x4As
	( const wchar_t * pszUniform,
		const S4DMatrix* pData, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformMatrix4x4, pData, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformImage
	( size_t iUniform, const SGLImageObject* pImage )
{
	return	SetCustomUniform( iUniform, uniformTexture, &pImage, 1 ) ;
}

SGLError S3DCustomShader::SetCustomUniformImages
	( size_t iUniform, const SGLImageObject** ppImages, size_t nCount )
{
	return	SetCustomUniform( iUniform, uniformTexture, ppImages, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformImageAs
	( const wchar_t * pszUniform, const SGLImageObject * pImage )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformTexture, &pImage, 1 ) ;
}

SGLError S3DCustomShader::SetCustomUniformImagesAs
	( const wchar_t * pszUniform,
		const SGLImageObject** ppImages, size_t nCount )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ),
							uniformTexture, ppImages, nCount ) ;
}

SGLError S3DCustomShader::SetCustomUniformImage
	( size_t iUniform,
		S3DCustomShader::UniformType type, const SGLImageObject* pImage )
{
	return	SetCustomUniform( iUniform, type, &pImage, 1 ) ;
}

SGLError S3DCustomShader::SetCustomUniformImageAs
	( const wchar_t * pszUniform,
		S3DCustomShader::UniformType type, const SGLImageObject * pImage )
{
	return	SetCustomUniform
		( (size_t) FindCustomUniform( pszUniform ), type, &pImage, 1 ) ;
}

SGLError S3DCustomShader::SetCustomUniformSet
	( const S3DCustomShader::UniformSet& unis )
{
	SGLError	err = sglErrSuccess ;
	for ( size_t i = 0; i < unis.GetLength(); i ++ )
	{
		const SString *	pstrID = unis.GetTagAt(i) ;
		UniformData *	pud = unis.GetAt( i ) ;
		ESLAssert( (pstrID != nullptr) && (pud != nullptr) ) ;
		if ( (pstrID == nullptr) || (pud == nullptr) )
		{
			continue ;
		}
		ssize_t	iUniform = FindCustomUniform( *pstrID ) ;
		if ( iUniform < 0 )
		{
			err = sglErrInvalidParam ;
			continue ;
		}
		SGLError	erri =
			SetCustomUniform
				( iUniform, pud->m_type, pud->m_pData, pud->m_nLength ) ;
		if ( erri )
		{
			err = erri ;
		}
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// 抽象レンダリングデバイス
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DRenderDevice::DefaultShaderId::NonShading = L"DEFAULT_NON_SHADING" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::Gouraud = L"DEFAULT_GOURAUD" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::Phong = L"DEFAULT_PHONG" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SimpleWireFrame = L"SIMPLE_WIRE_FRAME" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SimpleUVWireFrame = L"SIMPLE_UV_WIRE_FRAME" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::DrawWithDepth = L"DRAW_WITH_DEPTH" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::DelayLight = L"DELAY_LIGHT" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SSGISampler = L"DEFAULT_SSGI_SAMPPLER" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SSGIComposer = L"DEFAULT_SSGI_COMPOSER" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::GaussianBlur = L"GAUSSIAN_BLUR_1X9" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::RadialGaussianBlur = L"GAUSSIAN_RADIAL_BLUR_X9" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::DepthBlender = L"DEPTH_BLENDER" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SimpleMosaic = L"SIMPLE_MOSAIC" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::SimpleWater = L"SIMPLE_WATER" ;
const wchar_t *	S3DRenderDevice::DefaultShaderId::ShadowmapFilter = L"SHADOWMAP_FILTER_5X5" ;

// 少なくとも１つのデバイス・シェーダーが複数形状描画に対応している
bool	S3DRenderDevice::m_availableMultiShapeVB = false ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderDevice, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice::S3DRenderDevice( void )
{
	m_pShdCompileListener = NULL ;
}

// 通知オブジェクトを追加する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::AddNotifyObject( S3DRenderDevice::Notify * pNotify )
{
	QuickLock() ;
	m_ntfFirst.InsertAfter( pNotify ) ;
	QuickUnlock() ;
}

// 通知オブジェクトを分離する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::DetachNotifyObject( S3DRenderDevice::Notify * pNotify )
{
	QuickLock() ;
	pNotify->DetachNotify() ;
	QuickUnlock() ;
}

// デバイスの削除を通知する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::NotifyDeviceRelease( void )
{
	Notify *	pNotify ;
	QuickLock() ;
	pNotify = m_ntfFirst.m_pntfNext ;
	while ( pNotify != NULL )
	{
		Notify *	pNext = pNotify->m_pntfNext ;
		pNotify->OnReleaseDevice( this ) ;
		pNotify = pNext ;
	}
	QuickUnlock() ;
}

// デバイスの再生成を通知する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::NotifyDeviceReset( void )
{
	Notify *	pNotify ;
	QuickLock() ;
	pNotify = m_ntfFirst.m_pntfNext ;
	while ( pNotify != NULL )
	{
		Notify *	pNext = pNotify->m_pntfNext ;
		pNotify->OnResetDevice( this ) ;
		pNotify = pNext ;
	}
	QuickUnlock() ;
}

// シェーダーコンパイルリスナ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::AttachShaderCompileListener
		( S3DCustomShader::CompileListener * pListener )
{
	m_pShdCompileListener = pListener ;
}

// カスタムシェーダーコンパイル／ロードと登録
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderDevice::MakeCustomShader
		( const wchar_t * pwszID,
			S3DCustomShader * pShader,
			const ShaderSourceInfo& src,
			S3DCustomShader::CompileListener * pListener )
{
	if ( GetShaderProgramAs( pwszID ) != NULL )
	{
		ESLTrace( "custom shader \'%s\' is already registered.\n",
							SString(pwszID).ToCharArray().GetConstArray() ) ;
		return	sglErrFailed ;
	}
	if ( pListener == NULL )
	{
		pListener = m_pShdCompileListener ;
	}
	S3DShaderBinaryLibrary *
			plibShader = S3DShaderBinaryLibrary::GetInstance() ;
	if ( plibShader != NULL )
	{
		S3DShaderBinary *	pBinary = NULL ;
		S3DShaderBinary::DeivceType
						typeDev = S3DShaderBinary::deviceNothing ;
		switch ( src.typeSource )
		{
		case	shaderGLSL:
			typeDev = S3DShaderBinary::deviceOpenGL ;
			break ;
		}
		QuickLock() ;
		pBinary = plibShader->GetBinary( pwszID, typeDev ) ;
		QuickUnlock() ;
		//
		if ( pBinary != NULL )
		{
			SGLError	err = LoadCustomShader( pShader, *pBinary, pListener ) ;
			if ( !err )
			{
				RegisterShaderProgram( pwszID, pShader ) ;
				return	err ;
			}
			pShader->Release() ;
		}
	}
	Trace( "compiling custom shader \'%s\'...\n",
					SString(pwszID).ToCharArray().GetConstArray() ) ;
	SGLError	err = CompileCustomShader( pShader, src, pListener ) ;
	if ( !err )
	{
		RegisterShaderProgram( pwszID, pShader ) ;
		//
		if ( plibShader != NULL )
		{
			S3DShaderBinary *	pBinary = new S3DShaderBinary ;
			if ( !SaveCustomShaderBinary( *pBinary, pShader ) )
			{
				pBinary->SetIdentity( pwszID ) ;
				plibShader->AddBinary( pBinary ) ;
			}
			else
			{
				delete	pBinary ;
			}
		}
	}
	else
	{
		Trace( "failed to make custom shader \'%s\'\n",
					SString(pwszID).ToCharArray().GetConstArray() ) ;
	}
	return	err ;
}

// カスタムシェーダーの登録
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderDevice::RegisterShaderProgram
		( const wchar_t * pwszID, S3DCustomShader * pShader )
{
	QuickLock() ;
	m_ssoaShader.Add( pwszID, pShader ) ;
	QuickUnlock() ;
	return	sglErrSuccess ;
}

// カスタムシェーダーの取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderDevice::GetShaderProgramAs( const wchar_t * pwszID ) const
{
	S3DCustomShader *	pShader = NULL ;
	QuickLock() ;
	pShader = m_ssoaShader.GetAs( pwszID ) ;
	QuickUnlock() ;
	return	pShader ;
}

// カスタムシェーダ―の削除
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::RemoveCustomShaderAs( const wchar_t * pwszID )
{
	m_ssoaShader.RemoveAs( pwszID ) ;
}

// カスタムシェーダーの削除
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::RemoveAllCustomShaders( void )
{
	m_ssoaShader.RemoveAll() ;
}


// カスタムシェーダーコンパイル／ロードと登録
// 登録済みの場合にはそのシェーダ―を取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderDevice::BuildCustomShader
		( const wchar_t * pwszID,
			const S3DRenderDevice::ShaderDescriptor * pShdDsc,
			S3DCustomShader::CompileListener * pListener,
			SSystem::SString * pstrErrorMessage,
			SSystem::SString * pstrErrorSource )
{
	S3DCustomShader *	pShader = GetShaderProgramAs( pwszID ) ;
	if ( pShader != NULL )
	{
		return	pShader ;
	}
	MakeShaderProc	msp
		( this, pwszID, pShdDsc, pListener, pstrErrorMessage, pstrErrorSource ) ;
	if ( IsOnRenderThread() )
	{
		Procedure( &msp, procedureSync ) ;
	}
	else
	{
		Procedure( &msp, procedureNoRender ) ;
	}
	msp.Wait() ;
	return	msp.GetResult() ;
}


S3DRenderDevice::MakeShaderProc::MakeShaderProc
	( S3DRenderDevice * pDevice,
		const wchar_t * pwszID,
		const S3DRenderDevice::ShaderDescriptor * pShdDsc,
		S3DCustomShader::CompileListener * pListener,
		SSystem::SString * pstrErrorMessage,
		SSystem::SString * pstrErrorSource )
	: m_errResult( sglErrFailed ),
		m_pDevice( pDevice ), m_pShader( NULL ),
		m_pwszID( pwszID ), m_pShdDsc( pShdDsc ), m_pListener( pListener ),
		m_pstrErrorMessage( pstrErrorMessage ),
		m_pstrErrorSource( pstrErrorSource )
{
	m_eventDone.Initialize( false ) ;
}

void S3DRenderDevice::MakeShaderProc::Run( void )
{
	if ( m_pShdDsc == nullptr )
	{
		m_pShader = m_pDevice->GetDefaultShaderProgramAs( m_pwszID ) ;
		return ;
	}
	SGLError	err ;
	m_pShader = m_pDevice->NewCustomShader( m_pShdDsc->m_ssi.typeProgram ) ;
	err = m_pDevice->MakeCustomShader
			( m_pwszID, m_pShader, m_pShdDsc->m_ssi, m_pListener ) ;
	if ( err )
	{
		if ( m_pstrErrorMessage != NULL )
		{
			*m_pstrErrorMessage = m_pShader->GetCompileErrorLog() ;
		}
		if ( m_pstrErrorSource != NULL )
		{
			*m_pstrErrorSource = m_pShader->GetLastErrorShaderSource() ;
		}
		delete	m_pShader ;
		m_pShader = NULL ;
		return ;
	}
	for ( size_t i = 0; i < m_pShdDsc->m_uniforms.GetLength(); i ++ )
	{
		UniformDescriptor *	pUniform = m_pShdDsc->m_uniforms.GetAt( i ) ;
		ESLAssert( pUniform != NULL ) ;
		if ( pUniform == NULL )
		{
			continue ;
		}
		m_pShader->RegisterCustomUniform
			( pUniform->m_name, pUniform->m_type, pUniform->m_count ) ;
	}
}

void S3DRenderDevice::MakeShaderProc::Finalize( void )
{
	m_eventDone.SetSignal() ;
}

SSystem::SError S3DRenderDevice::MakeShaderProc::Wait( int64_t msecTimeout )
{
	return	m_eventDone.Wait( msecTimeout ) ;
}

S3DCustomShader * S3DRenderDevice::MakeShaderProc::GetResult( void ) const
{
	return	m_pShader ;
}



//////////////////////////////////////////////////////////////////////////////
// シェーダー・ソース
//////////////////////////////////////////////////////////////////////////////

// クリア
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::ShaderSource::Clear( void )
{
	pszPlaneSrc = nullptr ;
	pbytEncodedSrc = nullptr ;
	nEncodedBytes = 0 ;
}

// ソース取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString S3DRenderDevice::ShaderSource::DecodeSource( void ) const
{
	if ( pbytEncodedSrc != nullptr )
	{
		//
		// ERISA-N 符号デコード
		//
		SMemoryReferenceFile	mfSrc ;
		mfSrc.AttachMemory( (void*) pbytEncodedSrc, nEncodedBytes ) ;
		//
		ERISA::SGLDecodeBitStream	bstream( 0x1000 ) ;
		bstream.AttachInputStream( &mfSrc ) ;
		//
		ERISA::SGLERISANDecodeContext	decoder( &bstream ) ;
		decoder.PrepareToDecodeERISANCode() ;
		//
		SSmartBuffer	sbuf ;
		sbuf.ReadFromStream( decoder ) ;
		//
		// UTF-8 デコード
		//
		SStringParser	sparsSrc ;
		sparsSrc.ReadTextFile( sbuf, Charset::encodingUTF8 ) ;
		//
		return	sparsSrc ;
	}
	else
	{
		return	SString( pszPlaneSrc ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// シェーダ―定義
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderDevice::ShaderDescriptor, SObject )

// ShaderDescriptor 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice::ShaderDescriptor::ShaderDescriptor( void )
{
	eslFillMemory( &m_ssi, 0, sizeof(m_ssi) ) ;
	m_ssi.typeProgram = programShader ;
	m_ssi.typeSource = shaderGLSL ;
	m_ssi.versionType = 0 ;
	m_ssi.nBaseShader = shadingMethodPhong ;
	m_ssi.flagsFeature = shaderUseStandardShader
						| shaderLimitLight
						| shaderEnableInstancedDraw | shaderMultiRenderTarget ;
	m_ssi.nAvailableMRT = renderTargetCount ;
	m_ssi.nBoneLimit = 16 ;
	m_ssi.nLightLimit = 1 ;
	m_ssi.nShadowmapLimit = 2 ;
	m_ssi.nUserLighting = 0 ;
	m_ssi.srcLighting.pszPlaneSrc = NULL ;
	m_ssi.srcLighting.pbytEncodedSrc = NULL ;
	m_ssi.srcLighting.nEncodedBytes = 0 ;
	m_ssi.srcVertex.pszPlaneSrc = NULL ;
	m_ssi.srcVertex.pbytEncodedSrc = NULL ;
	m_ssi.srcVertex.nEncodedBytes = 0 ;
	m_ssi.srcFragment.pszPlaneSrc = NULL ;
	m_ssi.srcFragment.pbytEncodedSrc = NULL ;
	m_ssi.srcFragment.nEncodedBytes = 0 ;
	m_ssi.srcGeometry.pszPlaneSrc = NULL ;
	m_ssi.srcGeometry.pbytEncodedSrc = NULL ;
	m_ssi.srcGeometry.nEncodedBytes = 0 ;
}

// ShaderDescriptor 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice::ShaderDescriptor::~ShaderDescriptor( void )
{
}

// XML 記述 <shader>
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderDevice::ShaderDescriptor::
				ParseDescriptor( const SSystem::SXMLDocument& xmlShader )
{
	SXMLDocument *	pxmlFeature = xmlShader.GetElementTagAs( L"feature" ) ;
	if ( pxmlFeature != NULL )
	{
		static const SXMLDocument::AttrInteger	aiPrograms[] =
		{
			{ L"shader", programShader },
			{ L"compute", programCompute },
			{ NULL, 0 },
		} ;
		m_ssi.typeProgram =
			(ShaderProgramType) pxmlFeature->GetAttrSymbolizedIntegerAs
						( L"program_type", aiPrograms, m_ssi.typeProgram ) ;
		//
		const SString *	pstrLang = pxmlFeature->GetAttributeAs( L"language" ) ;
		if ( pstrLang && (*pstrLang == L"glsl") )
		{
			m_ssi.typeSource = shaderGLSL ;
		}
		m_ssi.versionType = (int) pxmlFeature->GetAttrIntegerAs( L"version" ) ;
		//
		static const SXMLDocument::AttrInteger	aiShaders[] =
		{
			{ L"non_shading", shadingMethodNothing },
			{ L"gouraud", shadingMethodGouraud },
			{ L"phong", shadingMethodPhong },
			{ L"nothing", -1 },
			{ NULL, 0 },
		} ;
		m_ssi.nBaseShader =
			(int) pxmlFeature->GetAttrSymbolizedIntegerAs
						( L"base_shader", aiShaders, m_ssi.nBaseShader ) ;
		if ( m_ssi.nBaseShader == -1 )
		{
			m_ssi.nBaseShader = shadingMethodNothing ;
			m_ssi.flagsFeature &= ~shaderUseStandardShader ;
		}
		else
		{
			m_ssi.flagsFeature |= shaderUseStandardShader ;
		}
		//
		static const SXMLDocument::AttrInteger	aiTextureTypes[] =
		{
			{ L"luminous", shaderUseLuminousTexture },
			{ L"emission", shaderUseLuminousTexture },
			{ L"global_ao", shaderUseGlobalAOTexture },
			{ L"normal", shaderUseNormalTexture },
			{ L"alpha", shaderUseAlphaTexture },
			{ L"height", shaderUseHeightTexture },
			{ L"specular", shaderUseSpecularTexture },
			{ L"environment", shaderUseEnvMapping },
			{ L"refraction", shaderUseRefraction },
			{ L"cubemap", shaderUseCubemap },
			{ L"spheremap", shaderUseEnvSphere },
			{ L"viewportmap", shaderUseRefViewport },
			{ L"all_environment", shaderUseAllEnvMapping },
			{ L"3d", shaderEnableTexture3D },
			{ L"array", shaderEnableTextureArray },
			{ NULL, 0 },
		} ;
		const uint32_t	maskAllTexture =
							shaderUseAllTexture
							| shaderUseAllEnvMapping
							| shaderEnableTexture3D
							| shaderEnableTextureArray ;
		m_ssi.flagsFeature = (m_ssi.flagsFeature & ~maskAllTexture)
			| (uint32_t) pxmlFeature->GetAttrComplexIntegerAs
				( L"texture", aiTextureTypes,
								(m_ssi.flagsFeature & maskAllTexture) ) ;
		//
		const SString *	pstrLimitBone =
						pxmlFeature->GetAttributeAs( L"limit_bone" ) ;
		if ( pstrLimitBone != NULL )
		{
			m_ssi.flagsFeature |= shaderLimitBone ;
			m_ssi.nBoneLimit = (int) pstrLimitBone->AsInteger() ;
		}
		else
		{
			m_ssi.flagsFeature &= ~shaderLimitBone ;
		}
		//
		const SString *	pstrLimitLight =
						pxmlFeature->GetAttributeAs( L"limit_light" ) ;
		if ( pstrLimitLight != NULL )
		{
			m_ssi.flagsFeature |= shaderLimitLight ;
			m_ssi.nLightLimit = (int) pstrLimitLight->AsInteger() ;
		}
		else
		{
			m_ssi.flagsFeature &= ~shaderLimitLight ;
		}
		//
		const SString *	pstrLimitShadowMap =
						pxmlFeature->GetAttributeAs( L"limit_shadowmap" ) ;
		if ( pstrLimitShadowMap != NULL )
		{
			m_ssi.flagsFeature |= shaderLimitShadowmap ;
			m_ssi.nShadowmapLimit = (int) pstrLimitShadowMap->AsInteger() ;
		}
		else
		{
			m_ssi.flagsFeature &= ~shaderLimitShadowmap ;
		}
		//
		const SString *	pstrMRT =
						pxmlFeature->GetAttributeAs( L"multi_render_target" ) ;
		if ( pstrMRT != NULL )
		{
			m_ssi.nAvailableMRT = (int) pstrMRT->AsInteger() ;
			if ( m_ssi.nAvailableMRT >= 2 )
			{
				m_ssi.flagsFeature |= shaderMultiRenderTarget ;
			}
			else
			{
				m_ssi.flagsFeature &= ~shaderMultiRenderTarget ;
			}
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"morphing", L"true" ) == L"false" )
		{
			m_ssi.flagsFeature |= shaderDisableMorphing ;
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"instanced_draw", NULL ) == L"true" )
		{
			m_ssi.flagsFeature |= shaderEnableInstancedDraw ;
		}
		else
		{
			m_ssi.flagsFeature &= ~shaderEnableInstancedDraw ;
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"multi_shaped_draw", NULL ) == L"true" )
		{
			m_ssi.flagsFeature |= shaderEnableInstancedDraw ;
			m_ssi.flagsFeature &= ~shaderDisableMultiShapedDraw ;
		}
		else
		{
			m_ssi.flagsFeature |= shaderDisableMultiShapedDraw ;
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"std_geometry_shader", NULL ) == L"false" )
		{
			m_ssi.flagsFeature |= shaderWithoutStdGeometry ;
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"ex_attr_elements", nullptr ) == L"true" )
		{
			m_ssi.flagsFeature |= shaderExAttrElements ;
		}
		//
		if ( pxmlFeature->GetAttrStringAs( L"compute_depth", nullptr ) == L"true" )
		{
			m_ssi.flagsFeature |= shaderComputeDepth ;
		}
	}
	SXMLDocument *	pxmlUniforms = xmlShader.GetElementTagAs( L"uniforms" ) ;
	if ( pxmlUniforms != NULL )
	{
		for ( size_t i = 0; i < pxmlUniforms->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlUniform = pxmlUniforms->GetElementAt( i ) ;
			if ( (pxmlUniform == NULL)
				|| (pxmlUniform->GetTag() != L"uniform") )
			{
				continue ;
			}
			static const SXMLDocument::AttrInteger	aiTypes[] =
			{
				{ L"int", S3DCustomShader::uniformInt },
				{ L"float", S3DCustomShader::uniformFloat },
				{ L"vec2", S3DCustomShader::uniformVector2D },
				{ L"vec3", S3DCustomShader::uniformVector3D },
				{ L"vec4", S3DCustomShader::uniformVector4D },
				{ L"mat2", S3DCustomShader::uniformMatrix2x2 },
				{ L"mat3", S3DCustomShader::uniformMatrix3x3 },
				{ L"mat4", S3DCustomShader::uniformMatrix4x4 },
				{ L"sampler", S3DCustomShader::uniformTexture },
				{ L"image_read", S3DCustomShader::uniformImageRead },
				{ L"image_r", S3DCustomShader::uniformImageRead },
				{ L"image_write", S3DCustomShader::uniformImageWrite },
				{ L"image_w", S3DCustomShader::uniformImageWrite },
				{ L"image_read_write", S3DCustomShader::uniformImageReadWrite },
				{ L"image_rw", S3DCustomShader::uniformImageReadWrite },
				{ NULL, 0 },
			} ;
			UniformDescriptor *	pDesc = new UniformDescriptor ;
			pDesc->m_type = (S3DCustomShader::UniformType)
				pxmlUniform->GetAttrSymbolizedIntegerAs
					( L"type", aiTypes, S3DCustomShader::uniformInt ) ;
			pDesc->m_count =
				(size_t) pxmlUniform->GetAttrIntegerAs( L"count", 1 ) ;
			pDesc->m_name =
				pxmlUniform->GetAttrStringAs( L"name", NULL ) ;
			pDesc->m_xmlDesc = *pxmlUniform ;
			m_uniforms.Add( pDesc ) ;
		}
	}
	SXMLDocument *	pxmlLighting = xmlShader.GetElementTagAs( L"lighting_shader" ) ;
	if ( pxmlLighting != NULL )
	{
		m_srcLighting.m_path = pxmlLighting->GetAttrStringAs( L"src", NULL ) ;
		if ( m_srcLighting.m_path.IsEmpty() )
		{
			const SString *	pstrLightingSrc = pxmlLighting->GetTextElement() ;
			if ( pstrLightingSrc != NULL )
			{
				m_ssi.srcLighting.pszPlaneSrc =
					pstrLightingSrc->EncodeDefaultTo( m_srcLighting.m_source ) ;
				//
				static const SXMLDocument::AttrInteger	aiSubShade[] =
				{
					{ L"lighting", lightingUserLighting },
					{ L"shading", lightingUserShading },
					{ L"fogging", lightingUserFogging },
					{ L"sampling_diffusion", samplingUserDiffusion },
					{ L"sampling_emission", samplingUserEmission },
					{ L"sampling_normal", samplingUserNormal },
					{ NULL, 0 },
				} ;
				m_ssi.nUserLighting =
					(uint32_t) pxmlLighting->GetAttrComplexIntegerAs
											( L"sub_shader", aiSubShade, 0 ) ;
			}
		}
	}
	SXMLDocument *	pxmlVertex = xmlShader.GetElementTagAs( L"vertex_shader" ) ;
	if ( pxmlVertex != NULL )
	{
		m_srcVertex.m_path = pxmlVertex->GetAttrStringAs( L"src", NULL ) ;
		if ( m_srcVertex.m_path.IsEmpty() )
		{
			const SString *	pstrVertexSrc = pxmlVertex->GetTextElement() ;
			if ( pstrVertexSrc != NULL )
			{
				m_ssi.srcVertex.pszPlaneSrc =
					pstrVertexSrc->EncodeDefaultTo( m_srcVertex.m_source ) ;
			}
		}
	}
	SXMLDocument *	pxmlFrag = xmlShader.GetElementTagAs( L"fragment_shader" ) ;
	if ( pxmlFrag != NULL )
	{
		m_srcFragment.m_path = pxmlFrag->GetAttrStringAs( L"src", NULL ) ;
		if ( m_srcFragment.m_path.IsEmpty() )
		{
			const SString *	pstrFragSrc = pxmlFrag->GetTextElement() ;
			if ( pstrFragSrc != NULL )
			{
				m_ssi.srcFragment.pszPlaneSrc =
					pstrFragSrc->EncodeDefaultTo( m_srcFragment.m_source ) ;
			}
		}
	}
	SXMLDocument *	pxmlGeometry = xmlShader.GetElementTagAs( L"geometry_shader" ) ;
	if ( pxmlGeometry != NULL )
	{
		m_srcGeometry.m_path = pxmlGeometry->GetAttrStringAs( L"src", NULL ) ;
		if ( m_srcGeometry.m_path.IsEmpty() )
		{
			const SString *	pstrGeomSrc = pxmlGeometry->GetTextElement() ;
			if ( pstrGeomSrc != NULL )
			{
				m_ssi.srcGeometry.pszPlaneSrc =
					pstrGeomSrc->EncodeDefaultTo( m_srcGeometry.m_source ) ;
			}
		}
		m_ssi.flagsFeature |= shaderWithGeometry ;
	}
	SXMLDocument *	pxmlCompute = xmlShader.GetElementTagAs( L"compute_shader" ) ;
	if ( pxmlCompute != NULL )
	{
		m_ssi.dimLocalSize.x =
			(size_t) pxmlCompute->GetAttrIntegerAs
							( L"local_size_x", m_ssi.dimLocalSize.x ) ;
		m_ssi.dimLocalSize.y =
			(size_t) pxmlCompute->GetAttrIntegerAs
							( L"local_size_y", m_ssi.dimLocalSize.y ) ;
		m_ssi.dimLocalSize.z =
			(size_t) pxmlCompute->GetAttrIntegerAs
							( L"local_size_z", m_ssi.dimLocalSize.z ) ;
		//
		m_srcCompute.m_path = pxmlCompute->GetAttrStringAs( L"src", NULL ) ;
		if ( m_srcCompute.m_path.IsEmpty() )
		{
			const SString *	pstrCompSrc = pxmlCompute->GetTextElement() ;
			if ( pstrCompSrc != NULL )
			{
				m_ssi.srcCompute.pszPlaneSrc =
					pstrCompSrc->EncodeDefaultTo( m_srcCompute.m_source ) ;
			}
		}
		m_ssi.typeProgram = programCompute ;
	}
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// コンピュート・シェーダー・インターフェース
//////////////////////////////////////////////////////////////////////////////

// Execute() 呼び出し
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DComputeShaderInterface::ExecuterProc, SProcedure )

void S3DComputeShaderInterface::ExecuterProc::Run( void )
{
	m_errResult = m_pShader->Execute( m_pDevice, m_param ) ;
}

// S3DComputeShaderInterface クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DComputeShaderInterface, ESLObject )

// 実行（呼び出しスレッドは任意）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DComputeShaderInterface::SyncExecute
	( S3DRenderDevice * pDevice, const ExecuteParam& param )
{
	ExecuterProc	xproc( this, pDevice, param ) ;
	SGLError	err = pDevice->Procedure( &xproc, S3DRenderDevice::procedureSync ) ;
	if ( err )
	{
		return	err ;
	}
	return	xproc.GetResult() ;
}

SGLError S3DComputeShaderInterface::SyncExecute
	( S3DRenderDevice * pDevice,
		const DimSize& dimWorkGroups,
		S3DCustomShader::UniformSet * pUniforms, uint32_t nBarrierFlags )
{
	ExecuteParam	xparam ;
	xparam.dimWorkGroups = dimWorkGroups ;
	xparam.pUniforms = pUniforms ;
	xparam.nBarrierFlags = nBarrierFlags ;
	return	SyncExecute( pDevice, xparam ) ;
}



//////////////////////////////////////////////////////////////////////////////
// シェーダー・パラメーター・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DShaderParameterInterface, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// コンピュート・シェーダー・パラメーター・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DComputeShaderParameterInterface, S3DShaderParameterInterface )

// 実行（呼び出しスレッドは任意）
SGLError S3DComputeShaderParameterInterface::SyncExecute
	( S3DRenderDevice * pDevice, uint32_t nBarrierFlags )
{
	S3DComputeShaderInterface *
		pComputeShader = ESLTypeCast<S3DComputeShaderInterface>( this ) ;
	if ( pComputeShader == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DCustomShader::UniformSet				unis ;
	S3DComputeShaderInterface::ExecuteParam	xparam ;
	LockParameter() ;
	SetShaderUniformsTo( unis ) ;
	xparam.dimWorkGroups = CalcWorkGroupSize() ;
	UnlockParameter() ;
	//
	xparam.pUniforms = &unis ;
	xparam.nBarrierFlags = nBarrierFlags ;
	//
	return	pComputeShader->SyncExecute( pDevice, xparam ) ;
}


//////////////////////////////////////////////////////////////////////////////
// DrawWithDepth シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDrawWithDepthShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// GaussianBlur シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DGaussianBlurShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// GaussianBlur（放射状）シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DGaussianRadialBlurShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// DepthBlender シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDepthBlenderShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// DelayLight シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDelayLightShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// SSGI シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSSGISamplerInterface, S3DShaderParameterInterface )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSSGIComposerInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// 水面描画シェーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSimpleWaterShaderInterface, S3DShaderParameterInterface )



//////////////////////////////////////////////////////////////////////////////
// Shadowmap 用 5x5 ループフィルタ (Compute Shader)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DShadowmapDepthFilter5x5Interface, S3DComputeShaderParameterInterface )

// ガウスフィルター設定
void S3DShadowmapDepthFilter5x5Interface::SetGaussianFilter( float32_t g )
{
	//
	// ガウス計算
	//
	float32_t	fpGauss[3] ;
	float32_t	sum = 0.0f ;
	float32_t	c = (float32_t) (g * g) ;
	for ( int i = 0; i < 3; i ++ )
	{
		float32_t	x = (float32_t) (1.0f + i * 2.0f) ;
		float32_t	w = (float32_t) exp( -0.5 * (x * x) / c ) ;
		fpGauss[i] = w ;
		sum += w ;
		if ( i > 0 )
		{
			sum += w ;		// 中央以外は左右対称なので重みは2倍
		}
	}
	if ( sum <= 0.0 )
	{
		fpGauss[0] = 1.0f ;
	}
	else
	{
		float32_t	r = 1.0f / sum ;
		for ( int i = 0; i < 3; i ++ )
		{
			fpGauss[i] *= r ;
		}
	}
	//
	// ２次元に拡張
	//
	float32_t	fpKernel2[5][5] ;
	sum = 0.0 ;
	for ( int i = -2; i <= 2; i ++ )
	{
		for ( int j = -2; j <= 2; j ++ )
		{
			fpKernel2[i+2][j+2] = fpGauss[abs(i)] * fpGauss[abs(j)] ;
			sum += fpKernel2[i+2][j+2] ;
		}
	}
	float32_t	r = 1.0f / sum ;
	for ( int i = 0; i < 5; i ++ )
	{
		for ( int j = 0; j < 5; j ++ )
		{
			fpKernel2[i][j] *= r ;
		}
	}
	SetFilterKernel( &fpKernel2[0][0] ) ;
}


