
#if	!defined(__SAKURAGLX_SPRITE_CURSOR_H__)
#define	__SAKURAGLX_SPRITE_CURSOR_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// マウスカーソル表示用スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteCursor	: public SGLSprite
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteCursor, SGLSprite )
		// 構築関数
		SGLSpriteCursor( void ) ;
		// 消滅関数
		virtual ~SGLSpriteCursor( void ) ;

	public:
		enum	CursorControlFlag
		{
			cursorDefaultArrow		= 0x0001,
			cursorPositionPolling	= 0x0002,
			cursorImagePolling		= 0x0004,
			cursorAutoHidding		= 0x0008,
		} ;

	protected:
		uint32_t			m_nCursorFlags ;
		SGLAbstractWindow *	m_pWindow ;
		SGLImage			m_imgCursor ;
		SGLPoint			m_ptCursorPos ;
		uint32_t			m_msecStaying ;
		bool				m_flagAutoHidding ;
		uint32_t			m_msecHiddenTime ;

	#if	defined(__PLATFORM_WINDOWS__)
		HCURSOR				m_hCursor ;
		HCURSOR				m_hArrowCursor ;
	#endif

	public:
		// 動作フラグ
		uint32_t GetCursorControlFlags( void ) const
		{
			return	m_nCursorFlags ;
		}
		void SetCursorControlFlags( uint32_t nFlags ) ;
		uint32_t ModifyCursorControlFlags
					( uint32_t nAddFlags, uint32_t nRemoveFlags ) ;
		// ターゲットウィンドウ
		SGLAbstractWindow * GetTargetWindow( void ) const
		{
			return	m_pWindow ;
		}
		void AttachTargetWindow( SGLAbstractWindow * pWindow ) ;
		// 自動非表示時間
		uint32_t GetAutoHiddenTime( void ) const
		{
			return	m_msecHiddenTime ;
		}
		void SetAutoHiddenTime( uint32_t msecTime ) ;

	public:
		// ウィンドウローカル座標をスプライト空間に変換する
		void WindowPointToCursorSpace
			( S2DDVector& vCursor, const SGLPoint& ptCursor ) const ;
	protected:
		void WindowPointToCursorSpace
			( SGLSprite * pParentSpace, S2DDVector& vCursor ) const ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	public:
	#if	defined(__PLATFORM_WINDOWS__)
		// HCURSOR を画像データに変換
		static SGLImageBuffer *
					ConvertHCURSORtoImageBuffer( HCURSOR hCursor ) ;
		// HBITMAP を画像データに変換
		static SGLImageBuffer *
					ConvertHBITMAPtoImageBuffer( HBITMAP hBitmap ) ;
		// カーソルのカラー画像にマスク画像を合成
		static void MakeBlendCursorImage
			( SGLImageBuffer * pimgColor, SGLImageBuffer * pimgMask ) ;
	#endif

	} ;

}

#endif

