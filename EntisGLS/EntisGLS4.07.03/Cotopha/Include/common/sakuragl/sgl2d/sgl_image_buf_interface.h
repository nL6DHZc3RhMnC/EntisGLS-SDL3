
#if	!defined(__SAKURAGL_IMAGE_BUF_INTERFACE_H__)
#define	__SAKURAGL_IMAGE_BUF_INTERFACE_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// SGLImageBuffer - Image 変換 SGLImageBufferInterface
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	EntisGLS4ImageBufferInterface	: public SGLImageBufferInterface
	{
	public:
		Image *	m_pImage ;
		bool	m_fUpdate ;
		SGLRect	m_rctUpdate ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( EntisGLS4ImageBufferInterface, SGLImageBufferInterface )
		// 構築関数
		EntisGLS4ImageBufferInterface( void ) ;
		// 消滅関数
		virtual ~EntisGLS4ImageBufferInterface( void ) ;
		// 初期化
		SGLError Initialize( SGLImageBuffer * pImageBuf ) ;
	public:
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;
	} ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// SGLImageBuffer - SGLImageObject 変換 SGLImageBufferInterface
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageObjectBufferInterface	: public SGLImageBufferInterface
	{
	public:
		SSystem::SSmartReference<SGLImageObject>	m_refImage ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageObjectBufferInterface, SGLImageBufferInterface )
		// 構築関数
		SGLImageObjectBufferInterface( void ) ;
		// 消滅関数
		virtual ~SGLImageObjectBufferInterface( void ) ;
	public:
		// 参照先設定
		void SetImageReference( SGLImageObject * pImage ) ;
		void SetSmartImageReference( SGLImageObject * pImage ) ;
		// 参照先取得
		SGLImageObject * GetImageReference( void ) const ;
	public:
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 画像バッファの再確保通知
		virtual bool OnImageReBuffered( SGLImageBuffer * pImageBuf ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;
	} ;

}

#endif

