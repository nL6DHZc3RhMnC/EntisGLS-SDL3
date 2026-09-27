
#if	!defined(__SAKURAGLX_SPRITE_MULTI_VIEW_H__)
#define	__SAKURAGLX_SPRITE_MULTI_VIEW_H__	1

namespace	SakuraGL
{
	class SGLVRViewProducer ;

	//////////////////////////////////////////////////////////////////////////
	// 出力先ごとの表示パラメータを持つスプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMultiView	: public SGLSprite
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMultiView, SGLSprite )
		// 構築関数
		SGLSpriteMultiView( void ) ;
		SGLSpriteMultiView( const SGLSpriteMultiView& src ) ;
		// 消滅関数
		virtual ~SGLSpriteMultiView( void ) ;

	public:
		// 描画バッファ
		class	SecondaryBuffer	: public Buffer
		{
		public:
			bool								m_buffered ;
			bool								m_visible ;
			UpdateStatus						m_statusUpdate ;
			SGLRect								m_rectUpdate ;
			uint32_t							m_nTransparency ;
			SGLAffine							m_affine[2] ;
			SSystem::SSmartPointer<ESLObject>	m_pUserLayer ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( SecondaryBuffer, Buffer )
			// 構築関数
			SecondaryBuffer( SGLPaintContextType type = typePaintEntisGLS ) ;
			SecondaryBuffer( const SecondaryBuffer& buf ) ;
			// 消滅関数
			virtual ~SecondaryBuffer( void ) ;
			// バッファ生成
			SGLError CreateBuffer
				( uint32_t width, uint32_t height,
					uint32_t format = formatImageARGB, uint32_t depth = 32,
					uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
					bool flagZBuffer = false, bool flagStereo3D = false ) ;
			SGLError CreateStereoBuffer
				( const SGLSize& sizeRight,
					const SGLSize& sizeLeft,
					uint32_t format = formatImageARGB, uint32_t depth = 32,
					uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
					bool flagZBuffer = false ) ;
			// 表示状態
			bool IsVisible( void ) const ;
			void SetVisible( bool fVisible ) ;
			// 透明度
			uint32_t GetTransparency( void ) const ;
			void SetTransparency( uint32_t nTransparency ) ;
			// 変換行列
			const SGLAffine& GetAffine( size_t iView ) const ;
			SGLError SetAffine( size_t iView, const SGLAffine & affine ) ;
			// 複製
			virtual SGLObject * DuplicateObject( void ) ;
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

			friend class SGLSpriteMultiView ;
		} ;

	protected:
		SSystem::SPtrSortObjectArray
			<SGLSecondaryViewProducer,SecondaryBuffer>
										m_psoaFrameBuffers ;
		SGLSecondaryViewProducer *		m_pSelectView ;
		SecondaryBuffer *				m_pSelectBuffer ;

	public:
		// 出力先バッファ選択
		virtual SGLError SelectSecondaryView( SGLSecondaryViewProducer * psvp ) ;
		// 出力先バッファを取得
		SecondaryBuffer * GetSecondaryBufferOf
					( SGLSecondaryViewProducer * psvp ) const ;
		// 選択中の出力ビュー取得
		SGLSecondaryViewProducer * GetSelectedSecondaryView( void ) const ;
		// バッファを取得する
		SecondaryBuffer * GetSelectedFrameBuffer( void ) const ;
		SecondaryBuffer * CreateSelectedFrameBuffer
						( SGLPaintContextType type = typePaintEntisGLS ) ;
		// ユーザー定義バッファ取得
		ESLObject * GetSelectedUserLayerBuffer( void ) const ;
		// レンダリングデバイスの設定
		virtual SGLError SetRenderDevice( S3DRenderDevice * pDevice ) ;
		// ビュー固有可視状態
		SGLError SetVisibleOfView( bool fVisible ) ;
		bool IsVisibleOfView( void ) const ;
		// ビュー固有透明度
		SGLError SetTransparencyOfView( uint32_t nTransparency ) ;
		uint32_t GetTransparencyOfView( void ) const ;
		// ビュー固有座標変換
		SGLError SetAffineOfView
			( const SGLAffine * affine, size_t nCount ) ;
		size_t GetAffineOfView( SGLAffine * affine, size_t nCount ) const ;
		// VR HMD 表示で任意距離のスクリーンとしてパラメータを設定する
		SGLError SetViewSettingForVRHMD
			( const SGLVRViewProducer * pVR,
				const S3DVector& vScreenDstPos /* 表示する３次元座標 */,
				const S2DVector& vScreenCenter /* 表示画像の中心座標 */,
				const S2DVector& vScreenZoom /* 見かけ上の拡大率 */ ) ;
		// スクリーン表示パラメータ計算
		void CalcViewSettingForVRHMD
			( S3DVector& vScreenDstPos,
				S2DVector& vScreenCenter,
				S2DVector& vScreenZoom,
				const SGLVRViewProducer * pVR,
				double tanHFOV2, /* 左右視野角90度に対する拡大率 */
				double zScreenPos /* スクリーン設置ｚ座標 */ ) ;

	public:
		// 描画前処理
		virtual void BeforeDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// 描画後処理
		virtual void AfterDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// 更新領域通知
		virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
		// 描画パラメータ取得
		virtual bool GetPaintParam
			( SGLPaintParam& pp, SGLAffine& affine,
				const Virtual3DParam* pV3D = NULL,
				Stereo3DView s3dView = s3dMonoview ) const ;

	public:
		// バッファ生成
		virtual SGLError CreateBuffer
			( uint32_t width, uint32_t height,
				uint32_t format = formatImageDefaultRGBA,
				uint32_t depth = 32,
				uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
				bool flagZBuffer = false, bool flagStereo3D = false,
				SGLPaintContextType type = typePaintEntisGLS ) ;
		virtual SGLError CreateStereoBuffer
			( const SGLSize& sizeRight,
				const SGLSize& sizeLeft,
				uint32_t format = formatImageARGB, uint32_t depth = 32,
				uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
				bool flagZBuffer = false,
				SGLPaintContextType type = typePaintEntisGLS ) ;
		// バッファ解放
		virtual void ReleaseBuffer( void ) ;
		// バッファを取得する
		virtual Buffer * GetFrameBuffer( void ) const ;

	protected:
		// ユーザー定義バッファ生成
		virtual void OnCreateUserLayerBuffer
			( SecondaryBuffer * pBuffer,
				const SGLSize& sizeRight, const SGLSize& sizeLeft,
				uint32_t format, uint32_t depth,
				uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D ) ;
		// ユーザー定義バッファ解放時処理
		virtual void OnReleaseUserLayerBuffer( ESLObject * pUserLayer ) ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;

}

#endif

