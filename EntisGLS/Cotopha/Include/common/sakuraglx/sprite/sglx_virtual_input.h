
#if	!defined(__SAKURAGLX_VIRTUAL_INPUT_H__)
#define	__SAKURAGLX_VIRTUAL_INPUT_H__	1

#include <sakuragl/sgl_vr_view_producer.h>
#include <sakuraglx/ui/sglx_joy_stick.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 仮想入力
	//////////////////////////////////////////////////////////////////////////

	class	SGLVirtualInput	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLVirtualInput, SGLObject )
		// 構築関数
		SGLVirtualInput( void ) ;
		// 消滅関数
		virtual ~SGLVirtualInput( void ) ;

	public:
		// デバイス
		enum	DeviceType
		{
			deviceKeyboard,
			deviceMouse,
			deviceJoyStick,
			deviceCommand,
			deviceSignal,
		} ;
		// ジョイスティックデバイス
		enum	JoyStickDevice
		{
			joyStickId1,
			joyStickId2,
			joyStickXInput1,
			joyStickXInput2,
			joyStickXInput3,
			joyStickXInput4,
			joyStickUser1,
			joyStickUser2,
			joyStickUser3,
			joyStickUser4,
			joyStickMaxCount,
			joyStickDevCount	= joyStickXInput4 + 1,
		} ;
		// ジョイスティックボタン
		enum	JoyButton
		{
			// 汎用ジョイスティック
			joyUp,	joyDown,	joyLeft,	joyRight,
			joyButton1,	joyButton2,	joyButton3,	joyButton4,
			joyButton5,	joyButton6,	joyButton7,	joyButton8,
			joyButtonCount	= joyButton1 + 32,

			// XInput ボタン
			xinputDPadUp	= 0,
			xinputDPadDown,
			xinputDPadLeft,
			xinputDPadRight,
			xinputStart,
			xinputBack,
			xinputLeftThumb,
			xinputRightThumb,
			xinputLeftShoulder,
			xinputRightShoulder,
			xinputButtonA	= 12,
			xinputButtonB,
			xinputButtonX,
			xinputButtonY,
			xinputLeftTrigger,
			xinputRightTrigger,

			// Android GamePad ボタン
			androidPadUp	= 0,
			androidPadDown,
			androidPadLeft,
			androidPadRight,
			androidButtonA,
			androidButtonB,
			androidButtonC,
			androidButtonX,
			androidButtonY,
			androidButtonZ,
			androidButtonL1,
			androidButtonR1,
			androidButtonL2,
			androidButtonR2,
			androidButtonStart,
			androidButtonSelect,

			// VR コントローラー
			vrLCtrlUp = 0, vrLCtrlDown, vrLCtrlLeft, vrLCtrlRight,
			vrLCtrlButton1, vrLCtrlButton2, vrLCtrlButton3, vrLCtrlButton4,
			vrRCtrlUp = 20, vrRCtrlDown, vrRCtrlLeft, vrRCtrlRight,
			vrRCtrlButton1, vrRCtrlButton2, vrRCtrlButton3, vrRCtrlButton4,
			vrCtrlSystem = 40, vrCtrlAppMenu, vrCtrlGrip,
			vrLAxis0 = 48, vrLAxis1, vrLAxis2, vrLAxis3, vrLAxis4,
			vrRAxis0 = 56, vrRAxis1, vrRAxis2, vrRAxis3, vrRAxis4,
			ovrButtonA	= vrRCtrlButton1,
			ovrButtonB	= vrRCtrlButton2,
			ovrButtonX	= vrLCtrlButton1 + 4,
			ovrButtonY	= vrLCtrlButton1 + 5,
		} ;
		// 動作フラグ
		enum	BehaviorFlag
		{
			behaviorStickToDPad		= 0x0001,	// アナログスティックの入力を DPad 入力としても処理する
		} ;
		// 入力イベント
		class	InputEvent
		{
		public:
			DeviceType			typeDevice ;
			int64_t				numDevice ;
			int64_t				codeKey ;
			SSystem::SString	strCommand ;

			typedef	int		TypeDigest ;

		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			InputEvent( void )
				: typeDevice(deviceKeyboard), numDevice(0), codeKey(0) {}
			InputEvent( const InputEvent& src )
				: typeDevice(src.typeDevice),
					numDevice(src.numDevice),
					codeKey(src.codeKey), strCommand(src.strCommand) {}
			const InputEvent& operator = ( const InputEvent& ie )
			{
				typeDevice = ie.typeDevice ;
				numDevice = ie.numDevice ;
				codeKey = ie.codeKey ;
				strCommand = ie.strCommand ;
				return	*this ;
			}
			int64_t Compare( const InputEvent & ie ) const
			{
				int64_t	c = (int) typeDevice - (int) ie.typeDevice ;
				if ( c != 0 )
					return	c ;
				c = typeDevice - ie.typeDevice ;
				if ( c != 0 )
					return	c ;
				c = numDevice - ie.numDevice ;
				if ( c != 0 )
					return	c ;
				c = codeKey - ie.codeKey ;
				if ( c != 0 )
					return	c ;
				return	strCommand.Compare( ie.strCommand ) ;
			}
			TypeDigest GetDigest( void ) const
			{
				return	(int) typeDevice ;
			}
			bool operator == ( const InputEvent & ie ) const
			{
				return	(Compare(ie) == 0) ;
			}
			bool operator != ( const InputEvent & ie ) const
			{
				return	(Compare(ie) != 0) ;
			}
			bool operator < ( const InputEvent & ie ) const
			{
				return	(Compare(ie) < 0) ;
			}
			bool operator <= ( const InputEvent & ie ) const
			{
				return	(Compare(ie) <= 0) ;
			}
			bool operator > ( const InputEvent & ie ) const
			{
				return	(Compare(ie) > 0) ;
			}
			bool operator >= ( const InputEvent & ie ) const
			{
				return	(Compare(ie) >= 0) ;
			}
		} ;
		// コマンド
		class	Command
		{
		public:
			SSystem::SString	strFullID ;
			SSystem::SString	strID ;
			int64_t				nParam ;
			int64_t				nCode ;
			int					nPriority ;
			bool				fOverwritable ;

		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			Command( void )
				: nParam(0), nCode(0), nPriority(0), fOverwritable(false) {}
			Command( const Command& cmd )
				: strFullID(cmd.strFullID), strID(cmd.strFullID),
					nParam(cmd.nParam), nCode(cmd.nCode),
					nPriority(cmd.nPriority), fOverwritable(cmd.fOverwritable) {}
			const Command& operator = ( const Command& cmd )
			{
				strFullID = cmd.strFullID ;
				strID = cmd.strID ;
				nParam = cmd.nParam ;
				nCode = cmd.nCode ;
				nPriority = cmd.nPriority ;
				fOverwritable = cmd.fOverwritable ;
				return	*this ;
			}
		} ;

	protected:
		class	InputFilter	: public SGLSpriteMouseListener,
								public SGLSpriteKeyListener
		{
		public:
			SGLVirtualInput *					m_pQueue ;
			SSystem::SSmartReference<SGLSprite>	m_refRedirect ;
			SSystem::SSortArray
				< SSystem::SGenSortElement
					<InputEvent,InputEvent> >	m_filterEvent ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( InputFilter, SGLSpriteMouseListener, SGLSpriteKeyListener )
			// 構築関数
			InputFilter( void ) : m_pQueue(NULL) {}
			// イベントフィルタ取得
			const InputEvent* GetFilterAs( const InputEvent& evIn ) const ;
			// フィルタ処理
			bool FilterInputEvent( const InputEvent& evIn, bool fKeyPress ) ;

		public:	// SGLSpriteMouseListener オーバーライド
			// ホイール回転
			virtual bool OnMouseWheel
				( SGLSprite& sprite, int32_t zDelta,
					double xPos, double yPos, int64_t nFlags ) ;
			// 左ボタン
			virtual bool OnLButtonDown
				( SGLSprite& sprite,
					double xPos, double yPos, int64_t nFlags ) ;
			virtual bool OnLButtonUp
				( SGLSprite& sprite,
					double xPos, double yPos, int64_t nFlags ) ;
			// 右ボタン
			virtual bool OnRButtonDown
				( SGLSprite& sprite,
					double xPos, double yPos, int64_t nFlags ) ;
			virtual bool OnRButtonUp
				( SGLSprite& sprite,
					double xPos, double yPos, int64_t nFlags ) ;

		public:	// SGLSpriteKeyListener オーバーライド
			// キー入力
			virtual bool OnKeyDown
				( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
			virtual bool OnKeyUp
				( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
			// コマンド
			virtual bool OnCommand
				( SGLSprite& sprite, const wchar_t * pszCmd,
					int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable ) ;
		} ;

	public:
		class	JoyButtonListener	: public SGLSpriteButtonListener,
										public SGLBasicForm::ItemInteractive
		{
		protected:
			SSystem::SSmartReference<SGLVirtualInput>	m_refInput ;
			int64_t	m_numDevice ;
			int64_t	m_codeKey ;
			bool	m_flagPushing ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( JoyButtonListener, SGLSpriteButtonListener, ItemInteractive )
			// 構築関数
			JoyButtonListener
				( SGLVirtualInput * pInput,
					int64_t codeKey, int64_t numDevice = 0 ) ;
		public:	// SGLSpriteButtonListener
			// ボタンが押された
			virtual bool OnButtonPushed( SGLSpriteButton& button, bool fRepeat ) ;
			// ボタンのステータスが変化した
			virtual bool OnChangedButtonStatus( SGLSpriteButton& button ) ;
		public:	// SGLBasicForm::ItemInteractive
			// 通知
			virtual void PostMessage
				( SGLBasicForm::Item * pItem, int nParam, const wchar_t * pwszOpt = NULL ) ;
		} ;

	protected:
		// 前置フィルタ
		InputFilter		m_filterInput ;

		// 後置マップ
		SSystem::SSortArray
			< SSystem::SGenSortElement
				<InputEvent,InputEvent> >
								m_mapEvent ;

	public:
		// ジョイスティック・ポーリング用タイマ
		class	PollingTimer	: public SGLSpriteTimer
		{
		protected:
			SSystem::SSmartReference<SGLVirtualInput>	m_refInput ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( PollingTimer, SGLSpriteTimer )
			// 構築関数
			PollingTimer( void ) ;
			PollingTimer( SGLVirtualInput * pInput ) ;
			// タイマー処理
			virtual bool OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;
		} ;

	protected:
		// ジョイスティック・デバイス
		bool					m_flagBeginJoyStick ;
		bool					m_flagPendingJoyStick ;
		bool					m_flagJoyState[joyStickMaxCount] ;
		UI::SGLJoyStick			m_joyStick ;
		UI::SGLJoyStickState	m_joyState[joyStickMaxCount] ;
		UI::SGLJoyStickState	m_joyPhysState[joyStickMaxCount] ;
		SSystem::SSmartReference<SGLVRViewProducer>
								m_refVR ;
		size_t					m_iVRJoyStick ;
		uint32_t				m_maskJoyDevices ;
		int64_t					m_timeLastPollJoy ;
		PollingTimer *			m_pPollingTimer ;
		uint64_t				m_flagsBehavior ;

		// ボタンマスク
		enum	ButtonMask
		{
			bmPushed		= 0x00000001,
			bmPushedMask	= 0x7FFFFFFF,
			bmPushing		= 0x80000000
		} ;
		// ボタン状態
		uint32_t	m_joyStatus[joyStickMaxCount][joyButtonCount] ;

		// 入力キュー
		SSystem::SObjectArray<InputEvent>	m_queueInput ;
		size_t								m_limInputQueue ;

		// コマンドキュー
		SSystem::SObjectArray<Command>	m_queueCommand ;

	public:
		// マウス・キー入力リスナ関連付け
		void AttachPostListenerToWindow
			( SGLWindowSprite * pWindow, uint32_t maskJoyDev = 0xFFFFFFFF ) ;
		// マウス・キー入力リスナ解除
		void DetachPostListenerToWindow( SGLWindowSprite * pWindow ) ;
		// VR コントローラーを仮想ジョイスティックにマップ
		void AttachVRController( SGLVRViewProducer * pVR, size_t iJoyStick ) ;
		// 動作フラグ取得
		uint64_t GetBehaviorFlags( void ) const ;
		// 動作フラグ設定
		void SetBehaviorFlags( uint64_t nFlags ) ;

	public:	// 仮想ジョイスティック取得
		// ジョイスティックをポーリング
		virtual SGLError PollJoyStick( void ) ;
		// ジョイスティック接続状態取得
		uint32_t GetJoyStickDeviceMask( void ) const ;
		// アナログスティック状態取得
		virtual bool GetAnalogJoyPosition
			( S3DVector4& vPos, size_t joyStick = 0 ) ;
		virtual bool GetJoyStickState
			( UI::SGLJoyStickState& joyState, size_t joyStick = 0 ) ;
		// ボタン状態取得
		virtual bool IsJoyButtonPushing
			( size_t joyButton, size_t joyStick = 0 ) const ;
		// ボタン押下回数取得
		size_t GetJoyButtonPushed
			( size_t joyButton, size_t joyStick = 0 ) const ;
		// ボタン押下回数リセット
		void ResetJoyButtonPushed
			( size_t joyButton, size_t joyStick = 0 ) ;
		// 全ボタン押下回数リセット
		void ResetAllJoyButtonPushed( void ) ;

	public:	// 仮想ジョイスティック状態／入力キューへ反映
		// 入力押下イベント
		void PressInputEvent( const InputEvent& evIn ) ;
		// 入力解放イベント
		void ReleaseInputEvent( const InputEvent& evIn ) ;
		// ソフトウェア・アナログスティック入力
		void SetSoftwareAnalogPosition
				( S3DVector4& vPos, size_t joyStick = 0 ) ;

	public:	// キューへの操作
		// 入力イベントをキューから取得
		SGLError GetInputEvent( InputEvent& ev ) ;
		// 入力イベントキューの最大数を設定
		//（溢れたものは古いものから削除される／デフォルト：16）
		void SetInputQueueLimit( size_t nLimit ) ;
		// 入力イベントキューを全削除
		void FlushInputQueue( void ) ;
		// コマンドキューへ追加
		void AddCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0,
				int nPriority = SGLSprite::commandNormal,
				bool fOverwritable = false ) ;
		// コマンドをキューから取得
		SGLError GetCommand( Command& cmd ) ;
		// 入力コマンドキューを全削除
		void FlushCommandQueue( void ) ;

	public:
		// 前置フィルタ追加
		void AddFilter
			( const InputEvent& evIn, const InputEvent& evOut ) ;
		// 前置フィルタ削除
		void RemoveFilter( const InputEvent& evIn ) ;
		// 前置フィルタ全削除
		void RemoveAllFilter( void ) ;
		// 前置フィルタ
		const InputEvent* GetFilterAs( const InputEvent& evIn ) const ;

	public:
		// 後置フィルタ追加
		void AddInputMap
			( const InputEvent& evIn, const InputEvent& evOut ) ;
		// 後置フィルタ削除
		void RemoveInputMap( const InputEvent& evIn ) ;
		// 後置フィルタ全削除
		void RemoveAllInputMap( void ) ;
		// 後置フィルタ
		const InputEvent* GetInputMapAs( const InputEvent& evIn ) const ;

	public:
		// 前置フィルタ読み込み
		SGLError LoadPrefilter( const wchar_t * pwszFilterFile ) ;
		SGLError ParsePrefilter( SSystem::SXMLDocument& xmlDoc ) ;
		// 後置フィルタ読み込み
		SGLError LoadPostfilter( const wchar_t * pwszFilterFile ) ;
		SGLError ParsePostfilter( SSystem::SXMLDocument& xmlDoc ) ;

	protected:
		// フィルタ解釈
		SGLError ParseXMLFilter
			( SSystem::SSortArray
				< SSystem::SGenSortElement
					<InputEvent,InputEvent> >& mapFilter,
								SSystem::SXMLDocument& xmlDoc ) ;
		// InputEvent 解釈
		SGLError ParseXMLInputEvent
			( InputEvent& iev, SSystem::SXMLDocument& xmlTag ) ;

	protected:
		SSystem::SMutex *	m_pMutexUI ;

	public:
		// スレッド排他処理用
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError LockTrace
			( const char * pszSource,
				size_t nLineNum,
				int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t UnlockAll( void ) const ;
		virtual SSystem::SError Relock( atomic_int_t nLock ) const ;
		virtual atomic_int_t TestLocked( void ) const ;
		// スレッド排他オブジェクト設定
		void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;
	} ;


}

#endif

