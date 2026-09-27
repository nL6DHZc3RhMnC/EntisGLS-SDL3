
#if	!defined(__SAKURAGLX_IMAGE_COMPOSITION_H__)
#define	__SAKURAGLX_IMAGE_COMPOSITION_H__	1

#include <sakuraglx/extra/psdlib.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像コンポジション
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageComposition	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageComposition, SObject )
		// 構築関数
		SGLImageComposition( void ) ;
		// 消滅関数
		virtual ~SGLImageComposition( void ) ;

	public:
		// レイヤー
		class	Layer	: public SGLImage
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Layer, SGLImage )
			// 構築関数
			Layer( void ) ;
			Layer( const Layer& layer ) ;
			// 消滅関数
			virtual ~Layer( void ) ;
			// 子レイヤーの総数（孫レイヤー以下を含む）
			size_t GetTotalLayerCount( void ) const ;

		public:
			Layer *							m_parent ;
			SSystem::SString				m_name ;
			PSD::LayerRecord::LayerType		m_type ;
			SGLPoint						m_position ;
			uint32_t						m_blend ;
			bool							m_visible ;
			uint32_t						m_transparency ;
			SSystem::SPointerArray<Layer>	m_layers ;
		} ;

	public:
		PSD::ImageMode					m_mode ;
		SGLSize							m_sizeCanvas ;
		size_t							m_nChannels ;
		SSystem::SArray<uint8_t>		m_bufColorMode ;
		SSystem::SObjectArray<Layer>	m_layers ;
		SSystem::SPointerArray<Layer>	m_grouped ;

	public:
		// 読み込み
		SGLError LoadPSDFile( const wchar_t * pwszFilePath ) ;
		SGLError ReadPSDFile( SSystem::SFileInterface& file ) ;
		// 書き出し
		SGLError SavePSDFile( const wchar_t * pwszFilePath ) ;
		SGLError WritePSDFile( SSystem::SFileInterface& file ) ;
		// キャンバス情報
		const SGLSize& GetCanvasSize( void ) const ;
		PSD::ImageMode GetImageMode( void ) const ;
		size_t GetChannelCount( void ) const ;
		void SetCanvasInfo
			( const SGLSize& sizeCanvas,
				size_t nChannels, PSD::ImageMode mode = PSD::modeRGB ) ;
		// 合成イメージ生成
		SGLError CreateBlendedImage( SGLImageObject& imgBlended ) const ;

	public:
		// レイヤー数
		size_t GetLayerCount( void ) const ;
		// レイヤー取得
		Layer * GetLayerAt( size_t iLayer ) const ;
		// 階層状のレイヤー数
		size_t GetTreeLayerCount( void ) const ;
		// 階層状のレイヤー取得
		Layer * GetTreeLayerAt( size_t iLayer ) const ;
		// レイヤー追加
		void InsertLayerAt( size_t iLayer, Layer * pLayer, Layer * pParent = nullptr ) ;
		void AppendLayer( Layer * pLayer, Layer * pParent = nullptr ) ;
		Layer * AppendGroupLayer( const wchar_t * pwszLayerName, Layer * pParent = nullptr ) ;
		// 階層内のレイヤー番号をフラットなレイヤー番号へ変換
		ssize_t LocalIndexToGlobal( Layer * pParent, size_t iLayer ) const ;
		// レイヤー削除
		SGLError RemoveLayer( Layer * pLayer ) ;
		SGLError DetachLayer( Layer * pLayer ) ;

	public:
		// レイヤーサイズ計算
		static bool LayerSizeOf
			( SGLRect& rect, Layer& layer, int nThreshold = 0 ) ;
		// 切り出し画像生成
		static SGLError LayerCutBack
			( SGLImageObject& imgCutted,
					Layer& layer, const SGLRect& rectCut ) ;
		// レイヤー描画
		static SGLError DrawLayer
			( SGLPaintContextInterface& paint, Layer& layer ) ;

	} ;

}

#endif

