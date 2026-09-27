
#if	!defined(__SAKURAGL_BITMAP_FONT_H__)
#define	__SAKURAGL_BITMAP_FONT_H__

#include <sakura/ssys_linked_list.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像フォントオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLBitmapFontLoader	: public SGLFontObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLBitmapFontLoader, SGLFontObject )
		// 構築関数
		SGLBitmapFontLoader( void ) ;
		// 消滅関数
		virtual ~SGLBitmapFontLoader( void ) ;

	public:
		struct	FontEntry
		{
			uint32_t		sizeFont ;
			uint32_t		stylesFont ;
			SGLFontMetrics	metricsFont ;
			uint32_t		formatBitmap ;		// = formatImageGray
			uint32_t		depthBitmap ;		// = 8
			uint32_t		countCharacters ;
		} ;
		struct	CharacterEntry
		{
			uint32_t		codeChar ;
			uint32_t		offsetAddr ;
		} ;
		struct	CharacterHeader
		{
			int32_t			pitchChar ;
			SGLImageRect	rctExterior ;
		} ;
		struct	CharacterGryph
		{
			CharacterHeader	chdr ;
			uint8_t			bmp[1] ;
		} ;
		enum	ConstantValue
		{
			bufferPageBits			= 16,
			bufferPageSize			= (1 << bufferPageBits),
			bufferPageOffsetMask	= bufferPageSize - 1,
		} ;
		struct	GrphBufferCache
		{
			uint32_t		addrPage ;
			atomic_int_t	countRef ;
			uint8_t			memory[bufferPageSize] ;
		} ;
		class	FontSet
		{
		public:
			FontEntry								m_style ;
			SSystem::SArray<CharacterEntry>			m_entries ;
			SSystem::SPointerArray<CharacterEntry>	m_aptrEntries ;
		} ;

	protected:
		// スレッド排他処理用
		SSystem::SCriticalSection				m_csSync ;

		// プリレンダフォント画像ファイル
		SSystem::SChunkFile						m_cfFont ;

		// フォントセットリスト
		SSystem::SObjectArray<FontSet>			m_arrayFontSets ;

		// フォント画像バッファ
		size_t									m_countLoaded ;
		size_t									m_limitLoaded ;
		SSystem::SLinkedList<GrphBufferCache>	m_listLoaded ;

		// 選択しているデフォルトスタイル
		SGLFontStyle							m_fsSelStyle ;
		FontSet *								m_pSelFontSet ;

	public:
		// ファイルを開く
		SGLError OpenFontFile( const wchar_t * pwszFilePath ) ;
		SGLError OpenFontFile
			( SSystem::SFileInterface * pFile, bool flagOwner = true ) ;
		// ファイルを閉じる
		virtual void Close( void ) ;
		// メモリキャッシュ最大サイズ [bytes] を設定
		void SetCacheLimit( size_t limitCache ) ;
		// メモリキャッシュ最大サイズ [bytes] を取得
		size_t GetCacheLimit( void ) const ;

	protected:
		// メモリをロード
		GrphBufferCache * LoadGrphBuffer( uint32_t addrEntry ) ;
		// メモリをロード＆ロック（解放されないように参照カウンタ＋１）
		GrphBufferCache * LockGrphBuffer( uint32_t addrEntry ) ;
		// メモリをアンロック（解放できるように参照カウンタ－１）
		void UnlockGrphBuffer( GrphBufferCache * pCache ) ;

	public:
		// FontSet 総数
		size_t GetFontSetCount( void ) const ;
		// FontSet 取得
		FontSet * GetFontSetAt( size_t i ) const ;
		// 最も近いサイズの FontSet を取得
		FontSet * GetNearestFontSet( const SGLFontStyle& style ) const ;
		// フォント画像を取得
		SGLError GetFontMetrics
			( FontSet* pFontSet,
				uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, wchar_t wch ) ;
		// リサンプリング
		static void ResampleGrayscaleFont
			( uint8_t* pbytDst, const SGLSize& sizeDst,
				const uint8_t* pbytSrc, const SGLSize& sizeSrc,
				float x0, float y0, float xz, float yz ) ;

	public:
		// フォントオブジェクト生成
		virtual SGLFontObject * NewFont( const SGLFontStyle& style ) ;
		// スタイル設定
		virtual SGLError SetStyle( const SGLFontStyle& style ) ;
		// フォント情報取得・ラスタライズ
		virtual SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, uint32_t wch ) ;

	protected:
		// フォント参照インターフェース
		class	SGLReferenceFont	: public SGLFontObject
		{
		protected:
			SSystem::SSmartReference<SGLBitmapFontLoader>
											m_refFont ;
			SGLBitmapFontLoader::FontSet *	m_pFontSet ;
			SGLFontStyle					m_style ;
			SSystem::SArray<uint8_t>		m_bufGrph ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLReferenceFont, SGLFontObject )
			// 構築関数
			SGLReferenceFont( SGLBitmapFontLoader * pbmFont ) ;
			// フォントオブジェクト生成
			virtual SGLFontObject * NewFont( const SGLFontStyle& style ) ;
			// スタイル設定
			virtual SGLError SetStyle( const SGLFontStyle& style ) ;
			// フォント情報取得・ラスタライズ
			virtual SGLError GetMetrics
				( uint8_t* pbytRasterized, size_t nBufBytes,
							SGLFontMetrics& metrics, uint32_t wch ) ;
		} ;
	} ;


}

#endif
