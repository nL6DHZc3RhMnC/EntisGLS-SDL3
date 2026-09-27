
/*****************************************************************************
                  Adobe(R) Photoshop(R) ファイルローダー
 *****************************************************************************/

#if	!defined(__SAKURAGLX_PSD_LIBRARY_H__)
#define	__SAKURAGLX_PSD_LIBRARY_H__	1

namespace	PSD
{

//////////////////////////////////////////////////////////////////////////////
// バイトアライメント／ビッグエンディアン／リテラル型
//////////////////////////////////////////////////////////////////////////////

struct	uint16_be_t
{
public:
	uint8_t	d[2] ;
public:
	uint16_be_t( void )
	{
		d[0] = 0 ;
		d[1] = 0 ;
	}
	uint16_be_t( uint16_t v )
	{
		d[0] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[1] = (uint8_t) (v & 0xFF) ;
	}
	operator uint16_t ( void ) const
	{
		return	((uint16_t) d[0] << 8) | d[1] ;
	}
	uint16_be_t& operator = ( uint16_t v )
	{
		d[0] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[1] = (uint8_t) (v & 0xFF) ;
		return	*this ;
	}
	uint16_t GetUInt16( void ) const
	{
		return	((uint16_t) d[0] << 8) | d[1] ;
	}
	void SetUInt16( uint16_t v )
	{
		d[0] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[1] = (uint8_t) (v & 0xFF) ;
	}
} ;

struct	int16_be_t	: public uint16_be_t
{
public:
	int16_be_t( int16_t v = 0 )
		: uint16_be_t( (uint16_t) v ) {}
	operator int16_t ( void ) const
	{
		return	(int16_t) GetUInt16() ;
	}
	int16_be_t& operator = ( int16_t v )
	{
		SetUInt16( (uint16_t) v ) ;
		return	*this ;
	}
	int16_t GetInt16( void ) const
	{
		return	(int16_t) GetUInt16() ;
	}
	void SetInt16( int16_t v )
	{
		SetUInt16( (uint16_t) v ) ;
	}
} ;

struct	uint32_be_t
{
public:
	uint8_t	d[4] ;
public:
	uint32_be_t( void )
	{
		d[0] = 0 ;
		d[1] = 0 ;
		d[2] = 0 ;
		d[3] = 0 ;
	}
	uint32_be_t( uint32_t v )
	{
		d[0] = (uint8_t) ((v >> 24) & 0xFF) ;
		d[1] = (uint8_t) ((v >> 16) & 0xFF) ;
		d[2] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[3] = (uint8_t) (v & 0xFF) ;
	}
	operator uint32_t ( void ) const
	{
		return	((uint32_t) d[0] << 24)
					| ((uint32_t) d[1] << 16)
					| ((uint32_t) d[2] << 8) | d[3] ;
	}
	uint32_be_t& operator = ( uint32_t v )
	{
		d[0] = (uint8_t) ((v >> 24) & 0xFF) ;
		d[1] = (uint8_t) ((v >> 16) & 0xFF) ;
		d[2] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[3] = (uint8_t) (v & 0xFF) ;
		return	*this ;
	}
	uint32_t GetUInt32( void ) const
	{
		return	((uint32_t) d[0] << 24)
					| ((uint32_t) d[1] << 16)
					| ((uint32_t) d[2] << 8) | d[3] ;
	}
	void SetUInt32( uint32_t v )
	{
		d[0] = (uint8_t) ((v >> 24) & 0xFF) ;
		d[1] = (uint8_t) ((v >> 16) & 0xFF) ;
		d[2] = (uint8_t) ((v >> 8) & 0xFF) ;
		d[3] = (uint8_t) (v & 0xFF) ;
	}
} ;

struct	int32_be_t	: public uint32_be_t
{
public:
	int32_be_t( int32_t v = 0 )
		: uint32_be_t( (uint32_t) v ) {}
	operator int32_t ( void ) const
	{
		return	(int32_t) GetUInt32() ;
	}
	int32_be_t& operator = ( int32_t v )
	{
		SetUInt32( (uint32_t) v ) ;
		return	*this ;
	}
	int32_t GetInt32( void ) const
	{
		return	(int32_t) GetUInt32() ;
	}
	void SetInt32( int32_t v )
	{
		SetUInt32( (uint32_t) v ) ;
	}
} ;

typedef	uint16_be_t	WORD_BE ;
typedef	uint32_be_t	DWORD_BE ;



//////////////////////////////////////////////////////////////////////////////
// 構造体
//////////////////////////////////////////////////////////////////////////////

//#pragma pack( push, __PSDFILE_ALIGN__, 1 )

enum	ImageMode
{
	modeBitmap,
	modeGrayscale,
	modeIndexed,
	modeRGB,
	modeCMYK,
	modeMultichannel,
	modeDuotone,
	modeLab
} ;

struct	FILE_HEADER
{
	DWORD_BE	dwSignature ;		// must be "8BPS"
	WORD_BE		wVersion ;			// must be 1
	BYTE		bytReserved[6] ;	// must be zero
	WORD_BE		wChannels ;			// チャネル数 [1,24]
	DWORD_BE	dwHeight ;			// 高さ
	DWORD_BE	dwWidth ;			// 幅
	WORD_BE		wDepth ;			// ビット深度 {1,8,16}
	WORD_BE		wMode ;				// 画像モード (ImageMode)
} ;

struct	COLOR_MODE_DATA				// modeIndexed, modeDuotone 以外では０
{
	DWORD_BE	dwLength ;
	BYTE		bytData[1] ;
} ;

struct	IMAGE_RESOURCE_SECTION
{
	DWORD_BE	dwLength ;
	BYTE		bytData[1] ;
} ;

enum	ResolutionUnit
{
	runitPixelPerInch	= 1,
	runitPixelPerCM		= 2
} ;

enum	DisplayUnit
{
	dunitInches			= 1,
	dunitCM				= 2,
	dunitPoints			= 3,
	dunitPicas			= 4,
	dunitColumns		= 5
} ;

struct	ResolutionInfo
{
	DWORD_BE	fxHorzRes ;
	WORD_BE		wHorzResUnit ;		// ResolutionUnit
	WORD_BE		wWidthUnit ;		// DisplayUnit
	DWORD_BE	fxVertRes ;
	WORD_BE		wVertResUnit ;		// ResolutionUnit
	WORD_BE		wHeightUnit ;		// DisplayUnit
} ;

enum	ChannelID
{
	chidRed = 0, chidGreen = 1, chidBlue = 2,
	chidTransparencyMask = -1, chidUserSuppliedMask = -2
} ;

struct	CHANNEL_LENGTH_INFO
{
	WORD_BE		wChannelID ;		// enum ChannelID
	DWORD_BE	dwLength ;			// チャネルデータ長
} ;

struct	ADJUST_LAYER_DATA
{
	DWORD_BE	dwSize ;
	DWORD_BE	dwTop ;				// Rectangle enclosing layer mask
	DWORD_BE	dwLeft ;
	DWORD_BE	dwBottom ;
	DWORD_BE	dwRight ;
	BYTE		bytDefColor ;		// 0 or 255
	BYTE		Flags ;				// bit 0 = position relative to layer
									// bit 1 = layer mask disabled
									// bit 2 = inver layer mask when blending
	WORD_BE		wPadding ;			// Zeros
} ;

#define	PSD_CHUNK_SIGNATURE(a,b,c,d)	(((a)<<24)|((b)<<16)|((c)<<8)|(d))
enum	LayerBlendMode
{
	blendPass			= PSD_CHUNK_SIGNATURE('p','a','s','s'),
	blendNormal			= PSD_CHUNK_SIGNATURE('n','o','r','m'),
	blendDarken			= PSD_CHUNK_SIGNATURE('d','a','r','k'),
	blendLighten		= PSD_CHUNK_SIGNATURE('l','i','t','e'),
	blendHue			= PSD_CHUNK_SIGNATURE('h','u','e',' '),
	blendSat			= PSD_CHUNK_SIGNATURE('s','a','t',' '),
	blendColor			= PSD_CHUNK_SIGNATURE('c','o','l','r'),
	blendLuminosity		= PSD_CHUNK_SIGNATURE('l','u','m',' '),
	blendMultiply		= PSD_CHUNK_SIGNATURE('m','u','l',' '),
	blendDivision		= PSD_CHUNK_SIGNATURE('d','i','v',' '),
	blendAddition		= PSD_CHUNK_SIGNATURE('l','d','d','g'),
	blendSubtraction	= PSD_CHUNK_SIGNATURE('f','s','u','b'),
	blendScreen			= PSD_CHUNK_SIGNATURE('s','c','r','n'),
	blendDissolve		= PSD_CHUNK_SIGNATURE('d','i','s','s'),
	blendOverlay		= PSD_CHUNK_SIGNATURE('o','v','e','r'),
	blendHardLight		= PSD_CHUNK_SIGNATURE('h','L','i','t'),
	blendSoftLight		= PSD_CHUNK_SIGNATURE('s','L','i','t'),
	blendDifference		= PSD_CHUNK_SIGNATURE('d','i','f','f'),
} ;

enum	LayerFlag
{
	flagTransparencyProtected	= 0x01,
	flagInvisible				= 0x02
} ;

struct	LAYER_RECORD
{
	DWORD_BE			dwTop ;				// レイヤー領域
	DWORD_BE			dwLeft ;
	DWORD_BE			dwBottom ;
	DWORD_BE			dwRight ;
	WORD_BE				wChannels ;			// チャネル数
	CHANNEL_LENGTH_INFO	chlen[24] ;			// チャネル長
	DWORD_BE			dwSignature ;		// must be "8BIM"
	DWORD_BE			dwBlendMode ;		// 'norm' etc...
	BYTE				bytOpacity ;		// 0:透明 ～ 255:不透明
	BYTE				bytClipping ;		// 0:base, 1:それ以外
	BYTE				bytFlags ;			// bit0 : 透明部分の保護
											// bit1 : 不可視
											// bit4 : クリッピングマスク（旧レイヤーグループ）
	BYTE				bytFilter ;			// must be zero
	DWORD_BE			dwExtraSize ;		// これ以降のバイト数
	ADJUST_LAYER_DATA	adjdata ;
} ;

class	LayerRecord	: public LAYER_RECORD
{
public:
	enum	LayerType
	{
		layerNormal,
		layerBackground,
		layerGroup,
		layerEndOfGroup,
	} ;
	LayerType			typeLayer ;
	DWORD_BE			dwGroupBlendMode ;
	SSystem::SString	strLayerName ;
} ;

enum	GlobalLayerMaskKind
{
	glmkindColorSelected			= 0,
	glmkindColorProtected			= 1,
	glmkindUseValueStoredPerLayer	= 128,
} ;

struct	GLOBAL_LAYER_MASK_INFO
{
	DWORD_BE	dwSize ;					// =14
	WORD_BE		wOverlayColorSpace ;		// =0
	BYTE		bytColorComponents[2][4] ;	// =0
	WORD_BE		wOpacity ;					// [0,100]
	BYTE		bytKind ;					// enum GlobalLayerMaskKind
	BYTE		bytFiller ;					// =0
} ;

//#pragma pack( pop, __PSDFILE_ALIGN__ )


//////////////////////////////////////////////////////////////////////////////
// エンディアン変換関数
//////////////////////////////////////////////////////////////////////////////

inline DWORD dwswap( DWORD dwData )
{
#if	defined(_MSC_VER) && defined(__PROCESSOR_INTEL_X86__)
	__asm
	{
		mov	eax, dwData
		bswap	eax
		mov	dwData, eax
	}
	return	dwData ;
#else
	return	(DWORD) ((dwData >> 24) | ((dwData >> 8) & 0xFF00)
					| ((dwData << 8) & 0x00FF0000) | (dwData << 24)) ;
#endif
}

inline WORD wswap( WORD wData )
{
	return	(WORD) ((wData << 8) | (wData >> 8)) ;
}


//////////////////////////////////////////////////////////////////////////////
// PSD ファイル・リーダー
//////////////////////////////////////////////////////////////////////////////

class	FileReadListener ;

class	FileReader	: public ESLObject
{
protected:
	SSystem::SFileInterface *	m_pfile ;			// PSD ファイル
	bool						m_flagFileOwner ;
	FILE_HEADER					m_fhHeader ;		// ファイルヘッダ
	SSystem::SArray<uint8_t>	m_bufColorMode ;	// Color Mode Data
	SSystem::SObjectArray<LayerRecord>
								m_lstLayer ;		// レイヤー情報
	uint32_t					m_dwLayerDataPos ;	// レイヤー画像の位置
	uint32_t					m_dwBaseDataPos ;	// ベース画像の位置

public:
	class	LayerInfo
	{
	public:
		SakuraGL::SGLImageRect	rectImage ;
		SakuraGL::SGLImageRect	rectMask ;
		size_t					nChannels ;
		CHANNEL_LENGTH_INFO		chlen[24] ;
		uint32_t				dwBlendMode ;
		uint32_t				nTransparency ;
		uint32_t				dwFlags ;
		uint32_t				dwAdjFlags ;
		uint32_t				dwDefColor ;
		LayerRecord::LayerType	typeLayer ;
		uint32_t				dwGroupBlendMode ;
		SSystem::SString		strLayerName ;
	} ;

public:
	// 構築関数
	FileReader( void ) ;
	// 消滅関数
	virtual ~FileReader( void ) ;
	// クラス情報
	ESL_DECLARE_CLASS_INFO( PSD::FileReader, ESLObject )

public:
	// ファイルを開く
	SSystem::SError Open
		( SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
	// ファイルを閉じる
	void Close( void ) ;

public:
	// ヘッダ情報を取得する
	void GetFileHeader( FILE_HEADER & fhHeader ) const ;
	// カラーモード情報を取得する
	void GetColorModeData
			( SSystem::SArray<uint8_t>& bufColorMode ) const ;
	// レイヤーの総数を取得する
	size_t GetLayerCount( void ) const ;
	// レイヤー情報を取得する
	SSystem::SError GetLayerInfo
			( LayerInfo & liLayer, size_t iLayer ) const ;
	// レイヤー画像を取得する
	SSystem::SError LoadLayerImage
			( SakuraGL::SGLImageObject& imgBuf, size_t iLayer,
							FileReadListener * pListener = nullptr ) ;
	// ベース画像（合成済み）を取得する
	SSystem::SError LoadBaseImage
			( SakuraGL::SGLImageObject & imgBuf,
							FileReadListener * pListener = nullptr ) ;

public:
	// RLE の展開
	static void UnpackBits
		( uint8_t * ptrBuf, size_t nBufSize,
			const uint8_t * ptrRLE, size_t nRLESize ) ;

} ;

class	FileReadListener
{
public:
	// クラス情報
	ESL_DECLARE_NV_CLASS_INFO( FileReadListener )
	// 進行状況
	virtual SSystem::SError
		OnReadImage( FileReader * pfr, size_t current, size_t total ) ;
} ;



//////////////////////////////////////////////////////////////////////////////
// PSD ファイル・ライター
//////////////////////////////////////////////////////////////////////////////

class	FileWriteListener ;

class	FileWriter	: public ESLObject
{
public:
	class	LayerInfo	: public LayerRecord
	{
	public:
		SakuraGL::SGLImageObject *	pImage ;
		SSystem::SArray<uint8_t>	bufExtra ;
	} ;

protected:
	SSystem::SFileInterface *	m_pfile ;			// PSD ファイル
	bool						m_flagFileOwner ;
	FILE_HEADER					m_fhHeader ;
	int64_t						m_fpLayerMaskInfo ;

	SSystem::SObjectArray<LayerInfo>	m_lstLayer ;

public:
	// 構築関数
	FileWriter( void ) ;
	// 消滅関数
	virtual ~FileWriter( void ) ;
	// クラス情報
	ESL_DECLARE_CLASS_INFO( PSD::FileWriter, ESLObject )

public:
	struct	CanvasInfo
	{
		ImageMode		mode ;
		uint32_t		nWidth ;
		uint32_t		nHeight ;
		uint32_t		nChannels ;
		uint32_t		nDepth ;		// ビット深度 {1,8,16}
		size_t			nColorDataByes ;
		const void *	pColorModeData ;
	} ;
	// ファイルを開く
	SSystem::SError Open
		( const CanvasInfo& canvasInfo,
			SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
	// ファイルを閉じる
	void Close( void ) ;
	// レイヤー情報追加
	SSystem::SError AppendLayerInfo
		( const wchar_t * pwszLayerName,
			SakuraGL::SGLImageObject * pImage,
			int xOffset = 0, int yOffset = 0,
			PSD::LayerBlendMode blendMode = blendNormal,
			uint32_t nOpacity = 0xFF,
			bool flagClipping = false, int nLayerFlags = 0,
			LayerRecord::LayerType typeLayer = LayerRecord::layerNormal,
			PSD::LayerBlendMode blendGroup = blendNormal ) ;
	SSystem::SError AppendGroupLayer
		( const wchar_t * pwszLayerName,
			PSD::LayerBlendMode blendGroup = blendPass ) ;
	SSystem::SError AppendEndOfGroup
		( const wchar_t * pwszLayerName = L"</Layer set>" ) ;
	// レイヤー画像データを出力
	SSystem::SError WriteAllLayerImages( FileWriteListener * pListener = nullptr ) ;
	// ベース画像（合成済み）を出力
	SSystem::SError WriteBaseImage( SakuraGL::SGLImageObject & imgBuf ) ;

} ;

class	FileWriteListener
{
public:
	// クラス情報
	ESL_DECLARE_NV_CLASS_INFO( FileWriteListener )
	// 進行状況
	virtual SSystem::SError OnWriteLayer
		( FileWriter * pfr,
			FileWriter::LayerInfo * pli, size_t iLayer, size_t nLayerCount ) ;
	virtual SSystem::SError OnFinishedLayer
		( FileWriter * pfr,
			FileWriter::LayerInfo * pli, size_t iLayer, size_t nLayerCount ) ;
} ;


} ;


#endif

