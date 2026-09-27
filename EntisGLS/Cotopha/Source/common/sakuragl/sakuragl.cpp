
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d/sgl_image_encoder.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/window/sgl_window_menu.h>

#if	!defined(__COTOPHA__)
	#include <sakura/ssys_module.h>
#endif

#if	defined(__PLATFORM_WINDOWS__)
	#if	!defined(ENTISGLS_WITH_MFC_LIB) && !defined(ENTISGLS4_DLL_IMPORT)
//		#define COMPILE_MULTIMON_STUBS
//		#include <multimon.h>
	#endif
	#include <sakuragl/sgl_direct_sound_player.h>
#endif

#if	defined(__PLATFORM_ANDROID__)
	#include <sakura/ssys_android_file.h>
#endif


//////////////////////////////////////////////////////////////////////////////
// ライブラリ初期化
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( atomic_int_t	SakuraGL::g_countRefSakuraGL = 0 ) ;
ESL_DLL_DECL( SSystem::SSyncBufferedFile	SakuraGL::cout ) ;
ESL_DLL_DECL( SSystem::SSyncBufferedFile	SakuraGL::cin ) ;

void SakuraGL::Initialize( void )
{
	SSystem::Initialize() ;
	//
#if	!defined(ENTISGLS4_DLL_IMPORT)
	if ( SSystem::AtomicAdd( &g_countRefSakuraGL, 1 ) == 1 )
	{
		#if	!defined(__COTOPHA__)
		ECSSakura2Processor::Initialize() ;
		#endif
		//
		ERISA::sclfInitializeMatrix() ;
		SGLImageDecoderManager::Initialzie() ;
		SGLImageEncoderManager::Initialzie() ;
		SGLAudioDecoderManager::Initialzie() ;
		//
		#if	defined(__PLATFORM_ANDROID__)
			cout.AttachFile( new SSystem::SConsoleFile, true ) ;
			cout.SetCharsetEncoding( SSystem::Charset::encodingUTF8 ) ;
			//
			cin.AttachFile( new SSystem::SConsoleFile, true ) ;
			cin.SetCharsetEncoding( SSystem::Charset::encodingUTF8 ) ;
		#else
			SSystem::SFile *	pConOut = new SSystem::SFile ;
			pConOut->Open
				( SSystem::SFile::DefaultName::StandardOutput,
										SSystem::SFile::modeWrite ) ;
			cout.AttachFile( pConOut, true ) ;
			cout.SetCharsetEncoding( SSystem::Charset::encodingShiftJIS ) ;
			//
			SSystem::SFile *	pConIn = new SSystem::SFile ;
			pConIn->Open
				( SSystem::SFile::DefaultName::StandardInput,
										SSystem::SFile::modeRead ) ;
			cin.AttachFile( pConIn, true ) ;
			cin.SetCharsetEncoding( SSystem::Charset::encodingShiftJIS ) ;
		#endif
		//
		SGLFont::InitializeRemapFontTable() ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		SGLDirectSoundPlayer::Initialize() ;
		#endif
		//
		SGLAudioPlayer::InitializeStatic() ;
	}
#endif
}

void SakuraGL::Finalize( void )
{
#if	!defined(ENTISGLS4_DLL_IMPORT)
	if ( SSystem::AtomicSub( &g_countRefSakuraGL, 1 ) == 0 )
	{
		SGLFont::FinalizeRemapFontTable() ;
		SGLImageDecoderManager::Finalize() ;
		SGLImageEncoderManager::Finalize() ;
		SGLAudioDecoderManager::Finalize() ;
		//
		cin.Close() ;
		cout.Close() ;
		//
		SGLAudioPlayer::ReleaseStatic() ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		SGLDirectSoundPlayer::Finalize() ;
		#endif
		//
		#if	!defined(__COTOPHA__)
		SGLWindowMenu::FinalizeWindowMenu() ;
		ECSSakura2Processor::Close() ;
		#endif
		//
		SGLObject::UnregisterAllObjectCreator() ;
	}
#endif
	SSystem::Finalize() ;
}



//////////////////////////////////////////////////////////////////////////////
// 二次元変換行列
//////////////////////////////////////////////////////////////////////////////

bool SakuraGL::SGLAffine::MeshMapping
	( const SakuraGL::S2DVector * pvMesh,
		int wMesh, int hMesh,
		SakuraGL::S2DVector& vDstPos, const SakuraGL::S2DVector& vSrcPos )
{
	int	ix = (int) eslRoundR32ToInt( (float32_t) floor( vSrcPos.x ) ) ;
	int	iy = (int) eslRoundR32ToInt( (float32_t) floor( vSrcPos.y ) ) ;
	if ( (ix < 0) || (ix >= wMesh)
		|| (iy < 0) || (iy >= hMesh) )
	{
		return	false ;
	}
	const int	iMesh = ix + iy * (wMesh + 1) ;
	S2DVector	vLocal
		( vSrcPos.x - (float32_t) ix, vSrcPos.y - (float32_t) iy ) ;
	SGLAffine	affine ;
	S2DVector	vMapSrc[3] ;
	S2DVector	vMapDst[3] ;
	vMapSrc[0].x = 0.0f ;
	vMapSrc[0].y = 0.0f ;
	vMapSrc[1].x = 1.0f ;
	vMapSrc[1].y = 1.0f ;
	vMapDst[0] = pvMesh[iMesh] ;
	vMapDst[1] = pvMesh[iMesh + wMesh + 2] ;
	if ( vLocal.y >= vLocal.x )
	{
		vMapSrc[2].x = 0.0f ;
		vMapSrc[2].y = 1.0f ;
		vMapDst[2] = pvMesh[iMesh + wMesh + 1] ;
	}
	else
	{
		vMapSrc[2].x = 1.0f ;
		vMapSrc[2].y = 0.0f ;
		vMapDst[2] = pvMesh[iMesh + 1] ;
	}
	affine.MappingOf( vMapDst, vMapSrc ) ;
	vDstPos = affine * vLocal ;
	return	true ;
}

bool SakuraGL::SGLAffine::InverseMeshMapping
	( const SakuraGL::S2DVector * pvMesh,
		int wMesh, int hMesh,
		SakuraGL::S2DVector& vSrcPos, const SakuraGL::S2DVector& vDstPos )
{
	const int	wMeshStep = wMesh + 1 ;
	const int	hMeshStep = hMesh + 1 ;
	const int	nMeshVertex = wMeshStep * hMeshStep ;
	float32_t	fpNearest = (float32_t) (vDstPos - pvMesh[0]).Absolute() ;
	int			iNearest = 0 ;
	for ( int i = 1; i < nMeshVertex; i ++ )
	{
		float32_t	r = (float32_t) (vDstPos - pvMesh[i]).Absolute() ;
		if ( r < fpNearest )
		{
			fpNearest = r ;
			iNearest = i ;
		}
	}
	SGLAffine	affine ;
	S2DVector	vMapSrc[3] ;
	S2DVector	vMapDst[3] ;
	S2DVector	vTemp ;
	int	xNear = iNearest % wMeshStep ;
	int	yNear = (iNearest - xNear) / wMeshStep ;
	for ( int y = 0; y < 2; y ++ )
	{
		if ( (yNear + y - 1 < 0)
			|| (yNear + y >= hMeshStep) )
		{
			continue ;
		}
		for ( int x = 0; x < 2; x ++ )
		{
			if ( (xNear + x - 1 < 0)
				|| (xNear + x >= wMeshStep) )
			{
				continue ;
			}
			const int	iMesh = (xNear + x - 1)
								+ (yNear + y - 1) * wMeshStep ;
			vMapSrc[0] = pvMesh[iMesh] ;
			vMapSrc[1] = pvMesh[iMesh + wMeshStep + 1] ;
			vMapSrc[2] = pvMesh[iMesh + wMeshStep] ;
			vMapDst[0].x = 0.0f ;
			vMapDst[0].y = 0.0f ;
			vMapDst[1].x = 1.0f ;
			vMapDst[1].y = 1.0f ;
			vMapDst[2].x = 0.0f ;
			vMapDst[2].y = 1.0f ;
			vTemp = affine.MappingOf( vMapDst, vMapSrc ) * vDstPos ;
			if ( (vTemp.y >= vTemp.x)
				&& (vTemp.x >= 0.0f) && (vTemp.x <= 1.0f)
				&& (vTemp.y >= 0.0f) && (vTemp.y <= 1.0f) )
			{
				vSrcPos.x = (float32_t) (xNear + x - 1) + vTemp.x ;
				vSrcPos.y = (float32_t) (yNear + y - 1) + vTemp.y ;
				return	true ;
			}
			vMapSrc[2] = pvMesh[iMesh + 1] ;
			vMapDst[2].x = 1.0f ;
			vMapDst[2].y = 0.0f ;
			vTemp = affine.MappingOf( vMapDst, vMapSrc ) * vDstPos ;
			if ( (vTemp.y <= vTemp.x)
				&& (vTemp.x >= 0.0f) && (vTemp.x <= 1.0f)
				&& (vTemp.y >= 0.0f) && (vTemp.y <= 1.0f) )
			{
				vSrcPos.x = (float32_t) (xNear + x - 1) + vTemp.x ;
				vSrcPos.y = (float32_t) (yNear + y - 1) + vTemp.y ;
				return	true ;
			}
		}
	}
	return	false ;
}

