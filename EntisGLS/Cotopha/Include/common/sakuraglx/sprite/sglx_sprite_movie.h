
#if	!defined(__SAKURAGLX_SPRITE_MOVIE_H__)
#define	__SAKURAGLX_SPRITE_MOVIE_H__	1

#include <sakuragl/sgl_media.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 動画再生スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMovie	: public SGLSprite,
								public SGLMediaPlayerFrameNotification
	{
	protected:
		SSystem::SSmartPointer<SGLMediaPlayer>	m_player ;
		SSystem::SString	m_strMovieFile ;
		bool				m_flagEndOfDuration ;
		bool				m_flagRestorePlaying ;
		bool				m_flagRestorePaused ;
		bool				m_flagDrawDirect ;
		bool				m_flagMovieLoop ;
		int64_t				m_nMovieLoopStart ;
		int64_t				m_nMovieLoopEnd ;
		SGLSize				m_sizeMovieFrame ;

		SGLWindowSprite *	m_pVideoViewWindow ;
		SGLImageRect		m_rectVideoView ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMovie, SGLSprite )
		// 構築関数
		SGLSpriteMovie( void ) ;
		SGLSpriteMovie( const SGLSpriteMovie& movie ) ;
		// 消滅関数
		virtual ~SGLSpriteMovie( void ) ;

	public:
		// 動画ファイルを開く
		SGLError OpenMovieFile( const wchar_t * pwszMoviePath ) ;
		// 動画ファイルを閉じる
		SGLError CloseMovieFile( void ) ;
		// 再生フラグ
		enum	PlayFlag
		{
			playDrawDirect	= 0x0001,
		} ;
		// 再生を開始する
		SGLError PlayMovie( uint64_t nFlags = 0 ) ;
		// 停止する
		SGLError StopMovie( void ) ;
		// ループポイント[/frame] を設定する
		SGLError SetMovieLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
		// ループ設定取得
		bool IsMovieLoop
			( int64_t * pStart = NULL, int64_t * pEnd = NULL ) const ;
		// 再生を一時停止する
		SGLError PauseMovie( void ) ;
		// 再生を再開する
		SGLError RestartMovie( void ) ;
		// 音量取得 [L/R]
		SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 音量を反映させるラインをクリアする（lineTotal,lineTotal2nd 以外）
		void ClearAllVolumeLines( void ) ;
		// 音量ライン追加設定
		void SetVolumeLine( size_t iLine ) ;
		// 再生中か？
		bool IsMoviePlaying( void ) const ;
		// 一時停止中か？
		bool IsMoviePaused( void ) const ;
		// メディアのサンプル周波数を取得する
		uint32_t GetMovieFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		uint64_t GetMovieLength( void ) const ;
		// 再生位置 [/sample] を取得する
		uint64_t GetMoviePosition( void ) const ;
		// 再生位置 [/sample] を変更する
		void SeekMovie( uint64_t nPos ) ;
		// ビデオサイズを取得する
		const SGLSize& GetMovieSize( void ) const ;
		// 再生終端到達判定
		bool HasEndedOfDuration( void ) const ;
		// 終端到達フラグをクリア
		void ResetEndOfDuration( void ) ;

	protected:
		// ビデオの表示設定
		void UpdateVideoView( void ) ;
		// ビデオの表示領域取得
		void GetVideoRectOnWindow( SGLImageRect& rectVideo ) const ;

	public:	// 画像表示インターフェース
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const Virtual3DParam* pV3D = NULL,
				Stereo3DView s3dView = s3dMonoview ) const ;
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;

	public:
		// フレーム更新通知
		virtual void OnFrameUpdate( SGLMediaPlayerInterface * player ) ;
		// 再生区間終端到達
		virtual void OnEndOfDuration( SGLMediaPlayerInterface * player ) ;

	public:
		// Rosetta 用クラス
		virtual const wchar_t * GetRSClassName( void ) const ;
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		// 復元後処理
		virtual SGLError OnAfterRestore( void ) ;

	} ;

}

#endif

