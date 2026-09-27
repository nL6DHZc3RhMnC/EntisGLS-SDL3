
#if	!defined(__SAKURAGL_RENDER_SOFTWARE_SHADER_H__)
#define	__SAKURAGL_RENDER_SOFTWARE_SHADER_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ソフトウェア・シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderingShader : public ESLObject
	{
	protected:
		// 色情報
		struct	RGB_INT32
		{
			int32_t	Red ;
			int32_t	Green ;
			int32_t	Blue ;
		} ;
		// 光源情報
		struct	LIGHT
		{
			S3DVector	vPosition ;		// 光源ベクトル
			S3DVector	vDirection ;
			uint32_t	typeLight ;		// 種類
			float32_t	fpBrightness ;	// 輝度（点光源の場合）
			float32_t	fpAttenuationPower ;	// 点光源減衰力 (1/r^x)
			float32_t	fpAngle ;		// 範囲角 cosθ
			float32_t	fpGradation ;	// ぼかし範囲 -cosθ
			RGB_INT32	rgbColor ;		// 光源色[×輝度]
		} ;

		// 光源情報
		RGB_INT32				m_rgbAmbient ;		// 環境光
		SSystem::SArray<LIGHT>	m_arrVectorLights ;	// 平行光源
		SSystem::SArray<LIGHT>	m_arrPointLights ;	// 点光源
		SSystem::SArray<LIGHT>	m_arrFogLights ;	// 擬似フォッグ

		// 擬似フォッグ
		bool		m_flagFog ;
		SGLPalette	m_rgbFogColor ;
		float32_t	m_zFogNear ;
		float32_t	m_zFogDistance ;

		// シェーディングパラメータ
		struct	SHADING_PARAMETER
		{
			uint32_t	maskDoubleSide ;	// 両面ポリゴンマスク
			int32_t		nAmbient ;			// 加算光成分
			float32_t	fpDiffusion ;		// 拡散反射光成分
			float32_t	fpSpecular ;		// 鏡面反射光成分
			float32_t	fpSpecularSize ;
			float32_t	fpDeepness ;		// 透明深度係数
			int32_t		nAlpha ;			// 不透明度
			RGB_INT32	rgbLight ;			// 光源加算効果
			RGB_INT32	rgbDiffusion ;		// 光源拡散反射効果
		} ;

		// 32ビット共用体
		union	FLOAT_UINT32
		{
			float32_t	fp32 ;
			uint32_t	ui32 ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DRenderingShader, ESLObject )
		// 構築関数
		S3DRenderingShader( void ) ;
		// 消滅関数
		virtual ~S3DRenderingShader( void ) ;

	public:
		// 光源を設定する
		void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// 疑似フォッグを設定
		void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		void EnableFog( bool fFog ) ;

	public:
		// シェーディング
		virtual void ShadeVertexColors
			( S3DColor * pColorLooks,
				const S3DSurfaceAttribute & attr,
				const S3DVector4 * pNormals,
				const S3DVector4 * pVertices,
				const S3DColor * pVertexColors, size_t nCount ) ;
		virtual void ShadeVectors
			( const S3DSurfaceAttribute & attr,
				const S3DVector4 * pNormals,
				const S3DVector4 * pVertices,
				S3DColor * pColorLooks, size_t nCount ) ;
		// 光源効果計算
		void CalculateLightEffect
			( SHADING_PARAMETER & sdp, const LIGHT & light,
				const S3DVector & vNormal,
				const S3DVector & vVertex, float32_t fpFocusParam ) ;
	} ;

}

#endif
