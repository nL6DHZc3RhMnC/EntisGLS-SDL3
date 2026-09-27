
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_vr_view_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// VR 出力基底インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLVRViewProducer, SGLSecondaryViewProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::SGLVRViewProducer( void )
{
	m_flagDrawToPrimary = false ;
	m_fpScaleHMDToModel = 1.0 ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLVRViewProducer::IsStereoDisplayMode( void )
{
	return	true ;
}

// プライマリウィンドウへの描画も行うか？
//////////////////////////////////////////////////////////////////////////////
bool SGLVRViewProducer::DoesDrawToPrimaryWindow( void )
{
	return	m_flagDrawToPrimary ;
}

// デバイス状態同期処理
//////////////////////////////////////////////////////////////////////////////
void SGLVRViewProducer::LockForDeviceState( void ) const
{
	m_csDevState.Lock() ;
}

void SGLVRViewProducer::UnlockForDeviceState( void ) const
{
	m_csDevState.Unlock() ;
}

// プライマリウィンドウへの描画設定
//////////////////////////////////////////////////////////////////////////////
void SGLVRViewProducer::SetDrawingToPrimaryWindow( bool fDrawToPrimary )
{
	m_flagDrawToPrimary = fDrawToPrimary ;
}

// HMD スケール
//////////////////////////////////////////////////////////////////////////////
void SGLVRViewProducer::SetScaleHMDToModel( double fpScale )
{
	m_fpScaleHMDToModel = fpScale ;
}

// アプリ側で補正した基準 HMD 位置を設定（手の取得位置に反映）
//////////////////////////////////////////////////////////////////////////////
void SGLVRViewProducer::SetHandsBasePosture
		( const SGLVRViewProducer::Posture& postureBase )
{
	m_csDevState.Lock() ;
	m_postureHandsBase.matOrientation = postureBase.matOrientation.Inverse() ;
	m_postureHandsBase.vPosition = postureBase.vPosition ;
	m_csDevState.Unlock() ;
}

// 現在のフレームの手（コントローラー）の位置取得
SGLVRViewProducer::Posture
	SGLVRViewProducer::GetHandPosture( SGLVRViewProducer::HandIndex iHand ) const
{
	Posture	posture ;
	m_csDevState.Lock() ;
	posture.nFlags = m_postureHands[iHand].nFlags ;
	posture.matOrientation = m_postureHandsBase.matOrientation
							* m_postureHands[iHand].matOrientation ;
	posture.vPosition = m_postureHandsBase.matOrientation
							* (m_postureHands[iHand].vPosition
									- m_postureHandsBase.vPosition) ;
	m_csDevState.Unlock() ;
	return	posture ;
}

// 視点毎の位置計算
//////////////////////////////////////////////////////////////////////////////
const SGLVRViewProducer::Posture&
	SGLVRViewProducer::CalcEyePosture
		( SGLVRViewProducer::Posture& postureEye,
			SGLVRViewProducer::EyeIndex index ) const
{
	m_csDevState.Lock() ;
	postureEye.vPosition =
		m_postureHead.matOrientation
			* m_postureEyes[index].vPosition + m_postureHead.vPosition ;
	postureEye.matOrientation =
		m_postureHead.matOrientation
			* m_postureEyes[index].matOrientation ;
	m_csDevState.Unlock() ;
	return	postureEye ;
}

// バイブレーション開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVRViewProducer::StartVibration
	( SGLVRViewProducer::ControllerIndex iCtrl,
		const SGLVRViewProducer::VibrationParam& vibParam )
{
	return	sglErrNotSupported ;
}

// バイブレーション即時停止
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVRViewProducer::StopVibration( ControllerIndex iCtrl )
{
	return	sglErrNotSupported ;
}


