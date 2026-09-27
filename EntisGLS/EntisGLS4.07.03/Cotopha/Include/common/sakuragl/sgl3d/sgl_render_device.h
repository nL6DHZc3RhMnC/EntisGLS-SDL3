
#if	!defined(__SAKURAGL_RENDER_DEVICE_H__)
#define	__SAKURAGL_RENDER_DEVICE_H__

namespace	SakuraGL
{
	class	S3DVertexBufferInterface ;
	class	S3DRenderContextInterface ;

	//////////////////////////////////////////////////////////////////////////
	// シェーダー・シリアライザ
	//////////////////////////////////////////////////////////////////////////

	class	S3DShaderBinary	: public ESLObject
	{
	protected:
		SSystem::SString			m_strID ;		// プログラム識別子
		SSystem::SArray<uint8_t>	m_bufFormat ;	// バイナリフォーマット
		SSystem::SArray<uint8_t>	m_bufBinary ;	// シェーダーバイナリ
		SSystem::SStrSortObjectArray<SSystem::SString>
									m_ssoaAttr ;	// カスタムシェーダー拡張属性値
		bool						m_flagAttrModified ;
		SSystem::SArray<uint16_t>	m_bufAttr ;		// ※シリアライズ化された属性値

	public:
		// バイナリフォーマット
		enum	DeivceType
		{
			deviceNothing	= 0,
			deviceOpenGL	= 1,
		} ;
		struct	Format
		{
			uint32_t		typeDev ;			// enum DeivceType
			uint32_t		nReserved ;			// must be 0
			union	FormatId
			{
				uint64_t	glType ;			// GLSL フォーマット (GLenum)
			}				idFormat ;
		} ;

	protected:
		// シリアライズヘッダ
		struct	Header
		{
			uint32_t	nIDLength ;
			uint32_t	nFormatBytes ;
			uint32_t	nBinaryBytes ;
			uint32_t	nAttrLength ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DShaderBinary, ESLObject )
		// 構築関数
		S3DShaderBinary( void ) ;
		// 消滅関数
		virtual ~S3DShaderBinary( void ) ;

	public:
		// プログラム識別子取得
		const SSystem::SString& GetIdentity( void ) const ;
		// プログラム識別子設定
		void SetIdentity( const wchar_t * pwszID ) ;

	public:
		// バイナリ設定
		void SetBinary
			( const Format * pFormat, size_t nFormatBytes,
				const void * pProgram, size_t nProgramBytes ) ;
		// フォーマット取得
		const Format * GetFormat( size_t& nBytes ) const ;
		// バイナリ取得
		const void * GetBinrary( size_t& nBytes ) const ;

	public:
		// 属性値取得
		const wchar_t * GetAttrStringAs
			( const wchar_t * pwszName, const wchar_t * pwszDefValue ) const ;
		int64_t GetAttrIntegerAs
			( const wchar_t * pwszName, int64_t nDefValue ) const ;
		// 属性値設定
		void SetAttrStringAs
			( const wchar_t * pwszName, const wchar_t * pwszValue ) ;
		void SetAttrIntegerAs
			( const wchar_t * pwszName, int64_t nValue ) ;

	public:
		// 読み込み
		SGLError Load( SSystem::SFileInterface& file ) ;
		// 書き出し
		SGLError Save( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// シェーダー・バイナリ・ライブラリ
	//////////////////////////////////////////////////////////////////////////

	class	S3DShaderBinaryLibrary	: public ESLObject
	{
	public:
		class	DeviceInfo
		{
		public:
			S3DShaderBinary::DeivceType	m_typeDev ;
			SSystem::SString			m_strVender ;
			SSystem::SString			m_strRenderer ;
			SSystem::SString			m_strVersion ;
		} ;

	protected:
		SSystem::SCriticalSection				m_csSync ;
		SSystem::SObjectArray<DeviceInfo>		m_arrDevice ;
		SSystem::SObjectArray<S3DShaderBinary>	m_arrBinary ;
		bool									m_flagModified ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DShaderBinaryLibrary, ESLObject )
		// 構築関数
		S3DShaderBinaryLibrary( void ) ;
		// 消滅関数
		virtual ~S3DShaderBinaryLibrary( void ) ;

	public:
		// デバイス情報取得
		const DeviceInfo * GetDeviceInfo
				( S3DShaderBinary::DeivceType typeDev ) const ;
		// デバイス情報設定
		void SetDeviceInfo
			( S3DShaderBinary::DeivceType typeDev,
				const wchar_t * pwszVender,
				const wchar_t * pwszRenderer,
				const wchar_t * pwszVersion ) ;

	public:
		// バイナリ検索
		ssize_t FindBinary
			( const wchar_t * pwszID,
				S3DShaderBinary::DeivceType typeDev ) const ;
		// バイナリ取得
		S3DShaderBinary * GetBinary
			( const wchar_t * pwszID,
				S3DShaderBinary::DeivceType typeDev ) const ;
		// バイナリ追加
		SGLError AddBinary( S3DShaderBinary * pBinary ) ;
		// バイナリ削除
		SGLError RemoveDeviceBinary
				( S3DShaderBinary::DeivceType typeDev ) ;
		// バイナリが登録されていないか？
		bool IsEmpty( void ) const ;
		// 変更されたか？
		bool IsModified( void ) const ;

	public:
		// 読み込み
		SGLError Load
			( SSystem::SFileInterface& file, uint32_t nShaderVer ) ;
		// 書き出し
		SGLError Save
			( SSystem::SFileInterface& file, uint32_t nShaderVer ) ;

	protected:
		static S3DShaderBinaryLibrary *	m_pInstance ;

	public:
		// グローバルインスタンス
		static S3DShaderBinaryLibrary * GetInstance( void ) ;
		static void SetInstance( S3DShaderBinaryLibrary * pInstance ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 抽象カスタムシェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCustomShader	: public SSystem::SObject
	{
	protected:
		SSystem::SCriticalSection	m_mutexUniform ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::S3DCustomShader, SObject )

	public:
		// カスタムシェーダーパラメータ
		enum	UniformType
		{
			uniformInt,					// int32_t
			uniformFloat,				// float32_t
			uniformVector2D,			// float32_t[2] / S2DVector
			uniformVector3D,			// float32_t[3] / S3DVector
			uniformVector4D,			// float32_t[4] / S4DVector
			uniformMatrix2x2,			// float32_t[2][2] : [行][列] / SGL2DMatrix<float32_t>
			uniformMatrix3x3,			// float32_t[3][3] : [行][列] / SGL3DMatrix<float32_t,3>
			uniformMatrix4x4,			// float32_t[4][4] : [行][列] / S4DMatrix
			uniformTexture,				// SGLImageObject*
			uniformImageRead,			// SGLImageObject*
			uniformImageWrite,			// SGLImageObject*
			uniformImageReadWrite,		// SGLImageObject*
			uniformTypeCount,
		} ;
		static bool IsUniformTypeTexture( UniformType type ) ;

		// コンパイルリスナ
		enum	CompileStage
		{
			compilePrepared,
			compileStartVertex,
			compileEndVertex,
			compileStartGeometry,
			compileEndGeometry,
			compileStartFragment,
			compileEndFragment,
			compileStartCompute,
			compileEndCompute,
			loadPrepared,
			linkStart,
			linkEnd,
			compiledProgram,
		} ;
		class	CompileListener
		{
		public:
			virtual void OnEvent
				( S3DCustomShader * shader,
					CompileStage stage, bool successed ) = 0 ;
		} ;

		// ユニフォームデータ
		class	UniformData
		{
		public:
			UniformType	m_type ;
			size_t		m_nLength ;
			void *		m_pData ;
			float32_t	m_bufData[4*4] ;
			bool		m_flagOwnData ;
			bool		m_flagUpdateData ;
		public:
			UniformData( void ) ;
			UniformData( const UniformData& ud ) ;
			~UniformData( void ) ;
			void SetType
				( UniformType type, size_t nLength ) ;
			bool SetData
				( UniformType type,
					const void * pData, size_t nLength ) ;
			bool UpdateData
				( UniformType type,
					const void * pData, size_t nLength ) ;
			size_t GetDataBytes( void ) const
			{
				return	m_bytesUniformType[m_type] * m_nLength ;
			}
			static size_t GetDataBytes( UniformType type, size_t nLength )
			{
				return	m_bytesUniformType[type] * nLength ;
			}
			bool SetDataInt( const int32_t * pData, size_t nLength ) ;
			bool SetDataFloat( const float32_t * pData, size_t nLength ) ;
			bool SetDataVector2D( const S2DVector * pData, size_t nLength ) ;
			bool SetDataVector3D( const S3DVector * pData, size_t nLength ) ;
			bool SetDataVector4D( const S4DVector * pData, size_t nLength ) ;
			bool SetDataMatrix3x3( const S3DMatrix * pData, size_t nLength ) ;
			bool SetDataMatrix4x4( const S4DMatrix * pData, size_t nLength ) ;
			bool SetDataTexture( const SGLImageObject* pImage ) ;
			bool SetDataImage( UniformType type, const SGLImageObject* pImage ) ;
		} ;
		class	UniformSet : public SSystem::SStrSortObjectArray<UniformData>
		{
		public:
			// 構築関数
			UniformSet( void ) { }
			UniformSet( const UniformSet& us )
				: SSystem::SStrSortObjectArray<UniformData>( us ) { }
			// データ設定
			bool SetDataAs
				( const wchar_t * pwszID,
					UniformType type,
					const void * pData, size_t nLength ) ;
			bool SetDataIntAs
				( const wchar_t * pwszID, const int32_t * pData, size_t nLength ) ;
			bool SetDataFloatAs
				( const wchar_t * pwszID, const float32_t * pData, size_t nLength ) ;
			bool SetDataVector2DAs
				( const wchar_t * pwszID,
					const S2DVector * pData, size_t nLength ) ;
			bool SetDataVector3DAs
				( const wchar_t * pwszID,
					const S3DVector * pData, size_t nLength ) ;
			bool SetDataVector4DAs
				( const wchar_t * pwszID,
					const S4DVector * pData, size_t nLength ) ;
			bool SetDataMatrix3x3As
				( const wchar_t * pwszID,
					const S3DMatrix * pData, size_t nLength ) ;
			bool SetDataMatrix4x4As
				( const wchar_t * pwszID,
					const S4DMatrix * pData, size_t nLength ) ;
			bool SetDataTextureAs
				( const wchar_t * pwszID, const SGLImageObject* pImage ) ;
			bool SetDataImageAs
				( const wchar_t * pwszID,
					UniformType type, const SGLImageObject* pImage ) ;
		} ;
		static const size_t	m_bytesUniformType[uniformTypeCount] ;

	public:
		// シェーダープログラム解放
		virtual void Release( void ) = 0 ;
		// コンパイル／リンクエラーログを取得
		virtual const SSystem::SString&
							GetCompileErrorLog( void ) const = 0 ;
		// エラーになったシェーダーソースを取得
		virtual const SSystem::SString&
							GetLastErrorShaderSource( void ) const = 0 ;
	public:
		// ユニフォーム指標定義
		struct	UniformEntry
		{
			const wchar_t *					id ;
			S3DCustomShader::UniformType	type ;
			size_t							count ;
		} ;
		virtual void RegisterCustomUniform
			( const wchar_t * pszUniformID,
				S3DCustomShader::UniformType type, size_t nCount ) = 0 ;
		void RegisterCustomUniforms
			( const UniformEntry * pUniformEntries, size_t nEntryCount ) ;
		// ユニフォーム指標取得
		virtual ssize_t FindCustomUniform( const wchar_t * pszUniform ) = 0 ;
		// ユニフォーム値設定
		virtual SGLError SetCustomUniform
			( size_t iUniform,
				S3DCustomShader::UniformType type,
				const void * pData, size_t nCount ) = 0 ;
		SGLError SetCustomUniformAs
			( const wchar_t * pszUniform,
				S3DCustomShader::UniformType type,
				const void * pData, size_t nCount ) ;
		SGLError SetCustomUniformInt
			( size_t iUniform, const int32_t* pData, size_t nCount ) ;
		SGLError SetCustomUniformIntAs
			( const wchar_t * pszUniform,
				const int32_t* pData, size_t nCount ) ;
		SGLError SetCustomUniformFloat
			( size_t iUniform, const float32_t* pData, size_t nCount ) ;
		SGLError SetCustomUniformFloatAs
			( const wchar_t * pszUniform,
				const float32_t* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector2D
			( size_t iUniform, const S2DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector2DAs
			( const wchar_t * pszUniform,
				const S2DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector3D
			( size_t iUniform, const S3DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector3DAs
			( const wchar_t * pszUniform,
				const S3DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector4D
			( size_t iUniform, const S4DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformVector4DAs
			( const wchar_t * pszUniform,
				const S4DVector* pData, size_t nCount ) ;
		SGLError SetCustomUniformMatrix3x3
			( size_t iUniform, const S3DMatrix* pData, size_t nCount ) ;
		SGLError SetCustomUniformMatrix3x3As
			( const wchar_t * pszUniform,
				const S3DMatrix* pData, size_t nCount ) ;
		SGLError SetCustomUniformMatrix4x4
			( size_t iUniform, const S4DMatrix* pData, size_t nCount ) ;
		SGLError SetCustomUniformMatrix4x4As
			( const wchar_t * pszUniform,
				const S4DMatrix* pData, size_t nCount ) ;
		SGLError SetCustomUniformImage
			( size_t iUniform, const SGLImageObject* pImage ) ;
		SGLError SetCustomUniformImages
			( size_t iUniform, const SGLImageObject** ppImages, size_t nCount ) ;
		SGLError SetCustomUniformImageAs
			( const wchar_t * pszUniform, const SGLImageObject * pImage ) ;
		SGLError SetCustomUniformImagesAs
			( const wchar_t * pszUniform,
				const SGLImageObject** ppImages, size_t nCount ) ;
		SGLError SetCustomUniformImage
			( size_t iUniform,
				UniformType type, const SGLImageObject* pImage ) ;
		SGLError SetCustomUniformImageAs
			( const wchar_t * pszUniform,
				UniformType type, const SGLImageObject * pImage ) ;
		SGLError SetCustomUniformSet
			( const S3DCustomShader::UniformSet& unis ) ;
		// ユニフォームへの排他処理
		// ※クラスメソッド内では同期しないので必要に応じて手動で呼び出す
		void LockUniform( void ) const
		{
			m_mutexUniform.Lock() ;
		}
		void UnlockUniform( void ) const
		{
			m_mutexUniform.Unlock() ;
		}
		const SSystem::SCriticalSection * GetUniformMutex( void ) const
		{
			return	&m_mutexUniform ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 抽象レンダリングデバイス
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderDevice	: public SSystem::SObject
	{
	public:
		class	Notify
		{
		protected:
			Notify *	m_pntfPrev ;
			Notify *	m_pntfNext ;

		public:
			// 構築関数
			Notify( void ) : m_pntfPrev(NULL), m_pntfNext(NULL) { }
			// 消滅関数
			~Notify( void ) ;
			// チェインを後ろに挿入
			void InsertAfter( Notify * pNotify ) ;
			// チェインを分離
			void DetachNotify( void ) ;

		public:
			// デバイスの削除前に呼び出される
			virtual void OnReleaseDevice( S3DRenderDevice * pDev ) = 0 ;
			// デバイスの再生成後に呼び出される
			virtual void OnResetDevice( S3DRenderDevice * pDev ) ;

			friend class S3DRenderDevice ;
		} ;

	protected:
		// 通知先頭チェイン
		class	FirstNotify	: public Notify
		{
		public:
			virtual void OnReleaseDevice( S3DRenderDevice * pDev ) { }
		} ;
		FirstNotify	m_ntfFirst ;

		// 登録シェーダー
		SSystem::SStrSortObjectArray<S3DCustomShader>	m_ssoaShader ;

		// シェーダーコンパイルリスナ
		S3DCustomShader::CompileListener *	m_pShdCompileListener ;

	public:
		// 少なくとも１つのデバイス・シェーダーが複数形状描画に対応している
		static bool	m_availableMultiShapeVB ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DRenderDevice, SObject )
		// 構築関数
		S3DRenderDevice( void ) ;
		// 通知オブジェクトを追加する
		void AddNotifyObject( Notify * pNotify ) ;
		// 通知オブジェクトを分離する
		void DetachNotifyObject( Notify * pNotify ) ;

	public:
		// デバイスの削除を通知する
		void NotifyDeviceRelease( void ) ;
		// デバイスの再生成を通知する
		void NotifyDeviceReset( void ) ;

	public:
		// レンダリングスレッドか判定
		virtual bool IsOnRenderThread( void ) = 0 ;
		// レンダリングスレッドで実行する
		enum	ProcedurePriority
		{
			procedureAsync,			// 非同期実行
			procedureSync,			// 同期実行
			procedureDelayable,		// 非同期実行（優先度低・画面描画優先）
			procedureLater,			// 非同期実行（即座に実行しない）
			procedureNoRender,		// 非レンダリングスレッドでレンダリング非同期実行（呼び出しは非同期／同期か不問・確実に同期を取る責任は呼び出し側にある）
		} ;
		virtual SGLError Procedure
			( SSystem::SProcedure* pProc, ProcedurePriority priority ) = 0 ;
		// レンダリングスレッドでの遅延実行が全て完了するまで待機
		virtual SGLError WaitUntilAsyncAllProcedures
					( int64_t msecTimeout = SSystem::Synchronism::Infinite ) = 0 ;
		// レンダラ生成
		virtual S3DRenderContextInterface * NewRenderer( void ) const = 0 ;
		// レンダリングデバイス用の画像インスタンスを生成／更新
		virtual SGLError CommitDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) = 0 ;
		// レンダリングデバイス用の VBO インスタンスを生成／更新
		virtual SGLError CommitDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) = 0 ;
		// レンダリングデバイス用の画像インスタンスを解放
		virtual SGLError ReleaseDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) = 0 ;
		// レンダリングデバイス用の VBO インスタンスを解放
		virtual SGLError ReleaseDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) = 0 ;

	public:
		// デバイス機能
		enum	FeatureFlag
		{
			// flagsFeatures[0]
			feature0_TextureNonPowerOf2	= 0x00000001,
			feature0_DepthTexture		= 0x00000002,
			feature0_CubemapTexture		= 0x00000004,
			feature0_MultisampleTexture	= 0x00000008,
			feature0_CompressionS3TC	= 0x00000010,
			feature0_ProgramBinary		= 0x00000100,
			feature0_MultiRenderTarget	= 0x00000200,
			feature0_GeometryShader		= 0x00000400,
			feature0_TextureFloat		= 0x00001000,
			feature0_ColorBufferFloat	= 0x00002000,
			feature0_InstancedDraw		= 0x00004000,
			feature0_MultiShapeDraw		= 0x00008000,
			feature0_CustomShader		= 0x00010000,
			feature0_ComputeShader		= 0x00020000,
		} ;
		struct	Features
		{
			uint32_t	flagsFeatures[4] ;
			uint32_t	maxTextureSize ;			// 最大テクスチャサイズ
			uint32_t	max3DTextureSize ;
			uint32_t	maxCubeTextureSize ;
			uint32_t	maxMultiTextureUnits ;		// 最大マルチテクスチャ（Fragment Shader）
			uint32_t	maxVSTexturesUnits ;		// 最大マルチテクスチャ（Vertex Shader）
			uint32_t	maxCombinedTextureUnits ;	// 最大マルチテクスチャ（全ての合計）
			uint32_t	maxVertexAttributes ;		// GLSL 最大 attribute 数 [vec4]
			uint32_t	maxVertexVaryings ;			// GLSL 最大 varying 数 [vec4]
			uint32_t	maxVertexUniforms ;			// GLSL 最大 uniform 数 [vec4]
			uint32_t	maxFragmentUniforms ;
			uint32_t	maxMutiRenderTarget ;		// 最大 Draw Buffer 数
			uint32_t	nReserved[19] ;
		} ;
		virtual SGLError GetDeviceFeatures( Features& features ) = 0 ;

	public:
		// カスタムシェーダー情報
		enum	ShaderProgramType
		{
			programShader	= 0,
			programCompute	= 1,
		} ;
		enum	ShaderSourceType
		{
			shaderGLSL	= 1,
		} ;
		enum	ShaderFeatureFlag
		{
			shaderUseLuminousTexture		= 0x00000001,
			shaderUseNormalTexture			= 0x00000002,
			shaderUseAlphaTexture			= 0x00000004,
			shaderUseHeightTexture			= 0x00000008,
			shaderUseSpecularTexture		= 0x00000010,
			shaderUseGlobalAOTexture		= 0x00000020,
			shaderUseAllTexture				= shaderUseLuminousTexture
												| shaderUseNormalTexture
												| shaderUseAlphaTexture
												| shaderUseHeightTexture
												| shaderUseSpecularTexture
												| shaderUseGlobalAOTexture,
			shaderUseStandardShader			= 0x00000100,
			shaderLimitBone					= 0x00000200,
			shaderLimitLight				= 0x00000400,
			shaderLimitShadowmap			= 0x00000800,
			shaderUseEnvMapping				= 0x00001000,
			shaderUseRefraction				= 0x00002000,
			shaderUseCubemap				= 0x00004000,
			shaderUseEnvSphere				= 0x00008000,
			shaderUseRefViewport			= 0x00010000,
			shaderUseAllEnvMapping			= shaderUseEnvMapping
												| shaderUseRefraction
												| shaderUseCubemap
												| shaderUseEnvSphere
												| shaderUseRefViewport,
			shaderEnableTexture3D			= 0x00020000,
			shaderEnableTextureArray		= 0x04000000,
			shaderDisableMorphing			= 0x00040000,
			shaderEnableInstancedDraw		= 0x00080000,
			shaderDisableMultiShapedDraw	= 0x00100000,
			shaderWithGeometry				= 0x00200000,
			shaderWithoutStdGeometry		= 0x00400000,
			shaderMultiRenderTarget			= 0x00800000,
			shaderExAttrElements			= 0x01000000,
			shaderComputeDepth				= 0x02000000,
		} ;
		enum	LightingShaderFlag
		{
			lightingUserLighting	= 0x00000001,
			lightingUserShading		= 0x00000002,
			lightingUserFogging		= 0x00000004,
			samplingUserDiffusion	= 0x00000010,
			samplingUserEmission	= 0x00000020,
			samplingUserNormal		= 0x00000040,
		} ;
		struct	ShaderSource
		{
			const char *	pszPlaneSrc ;		// 生ソース
			const uint8_t *	pbytEncodedSrc ;	// ERISA-N 符号済みソース
			size_t			nEncodedBytes ;

			ShaderSource( void )
				: pszPlaneSrc( nullptr ),
					pbytEncodedSrc( nullptr ),
					nEncodedBytes( 0 ) { }
			ShaderSource( const ShaderSource& ss )
				: pszPlaneSrc( ss.pszPlaneSrc ),
					pbytEncodedSrc( ss.pbytEncodedSrc ),
					nEncodedBytes( ss.nEncodedBytes ) { }
			// クリア
			void Clear( void ) ;
			// ソース取得
			SSystem::SString DecodeSource( void ) const ;
		} ;
		struct	DimSize
		{
			size_t	x, y, z ;

			DimSize( size_t ix = 1, size_t iy = 1, size_t iz = 1 )
					: x(ix), y(iy), z(iz) { }
			DimSize( const DimSize& dim )
					: x(dim.x), y(dim.y), z(dim.z) { }
		} ;
		struct	ShaderSourceInfo
		{
			ShaderProgramType	typeProgram ;
			ShaderSourceType	typeSource ;	// GLSL
			int					versionType ;	// GLSL バージョン
			int					nBaseShader ;	// enum S3DShadingFlags
			uint32_t			flagsFeature ;	// complex of enum ShaderFeature
			uint32_t			nAvailableMRT ;	// Shader が出力可能な最大 RT
			uint32_t			nBoneLimit ;
			uint32_t			nLightLimit ;
			uint32_t			nShadowmapLimit ;
			uint32_t			nUserLighting ;	// complex of enum LightingShaderFlag
			ShaderSource		srcLighting ;
			ShaderSource		srcVertex ;
			ShaderSource		srcFragment ;
			ShaderSource		srcGeometry ;
			ShaderSource		srcCompute ;
			DimSize				dimLocalSize ;
		} ;
		// 定義済み標準シェーダー
		class	DefaultShaderId
		{
		public:
			static const wchar_t *	NonShading ;	// シェーディング無し（主に2D描画用）
			static const wchar_t *	Gouraud ;		// グーローシェーディング
			static const wchar_t *	Phong ;			// フォンシェーディング
			static const wchar_t *	SimpleWireFrame ;	// 三角→ライン（シェーディング無し）
			static const wchar_t *	SimpleUVWireFrame ;	// 三角→UVライン（シェーディング無し）
			static const wchar_t *	DrawWithDepth ;	// 深度付描画
			static const wchar_t *	DelayLight ;	// 遅延シェーダー光源
			static const wchar_t *	SSGISampler ;	// 画面空間大域照明
			static const wchar_t *	SSGIComposer ;
			static const wchar_t *	GaussianBlur ;	// ガウスぼかし（1次元）
			static const wchar_t *	RadialGaussianBlur ;// ガウス放射状ブラー
			static const wchar_t *	DepthBlender ;	// 被写界深度用（ｚバッファをαテクスチャとして使用）
			static const wchar_t *	SimpleMosaic ;	// モザイク描画
			static const wchar_t *	SimpleWater ;	// 水面描画
			static const wchar_t *	ShadowmapFilter ;// Shadowmap 用フィルタ
		} ;
		// MRT コンポーネント（デフォルトシェーダー共通）
		enum	RenderTargetComponentIndex
		{
			renderTargetComposed,		// シェーディング結果
			renderTargetEmission,		// 発光成分（ポストエフェクト用）
			renderTargetNormal,			// 法線（遅延レンダリング用）
			renderTargetDiffusion,		// 拡散反射成分（遅延シェーディング用）
			renderTargetAmbient,		// 環境光成分（SSAO用）
			renderTargetSpecular,		// 鏡面反射成分（遅延シェーディング用）
			renderTargetCount,
		} ;
		// カスタムシェーダー・オブジェクト生成
		virtual S3DCustomShader * NewCustomShader( ShaderProgramType type ) = 0 ;
		// シェーダーコンパイルリスナ関連付け
		virtual void AttachShaderCompileListener
				( S3DCustomShader::CompileListener * pListener ) ;
		// 定義済み標準シェーダー生成／コンパイル／取得
		virtual S3DCustomShader *
				GetDefaultShaderProgramAs( const wchar_t * pwszID ) = 0 ;
		// カスタムシェーダーコンパイル／ロードと登録
		SGLError MakeCustomShader
				( const wchar_t * pwszID,
					S3DCustomShader * pShader,
					const ShaderSourceInfo& src,
					S3DCustomShader::CompileListener * pListener = NULL ) ;
		// カスタムシェーダーコンパイル
		virtual SGLError CompileCustomShader
				( S3DCustomShader * pShader,
					const ShaderSourceInfo& src,
					S3DCustomShader::CompileListener * pListener = NULL ) = 0 ;
		// カスタムシェーダー読み込み
		virtual SGLError LoadCustomShader
				( S3DCustomShader * pShader,
					const S3DShaderBinary& bin,
					S3DCustomShader::CompileListener * pListener = NULL ) = 0 ;
		// カスタムシェーダー・バイナリの保存
		virtual SGLError SaveCustomShaderBinary
			( S3DShaderBinary& bin, S3DCustomShader * pShader ) = 0 ;
		// カスタムシェーダーの登録
		SGLError RegisterShaderProgram
				( const wchar_t * pwszID, S3DCustomShader * pShader ) ;
		// カスタムシェーダーの取得
		S3DCustomShader * GetShaderProgramAs( const wchar_t * pwszID ) const ;
		// カスタムシェーダ―の削除
		void RemoveCustomShaderAs( const wchar_t * pwszID ) ;
		// カスタムシェーダーの削除
		void RemoveAllCustomShaders( void ) ;

	public:
		// パフォーマンスログ情報
		struct	PerformanceFrameLog
		{
			size_t		countDrawCall ;				// 描画回数
			size_t		countDrawInstance ;			// 描画インスタンス数
			size_t		countDrawVertex ;			// 描画頂点数
			size_t		countTransmitVertex ;		// CPU->VRAM 転送頂点数
			size_t		countTransmitPixel ;		// CPU<->VRAM 転送ピクセル数
			size_t		countComputeShapeByCPU ;	// CPU で実行したモーフィング・ボーン頂点数
			double		msecShapeByCPU ;			// CPU で頂点モーフィング・ボーン処理に費やした時間
			double		msecRenderingFrame ;		// フレーム描画時間
			size_t		countSwitchRenderer ;		// レンダラー切り替え回数
			size_t		countSwitchShader ;			// シェーダー切り替え回数
			size_t		countSwitchMaterial ;		// マテリアル切り替え回数
		} ;
		struct	PerformanceLogInfo
		{
			size_t				nFrameCount ;
			double				secInterval ;
			PerformanceFrameLog	pflogMax ;
			PerformanceFrameLog	pflogSum ;
			size_t				bytesUsedTexture ;	// テクスチャ使用容量（概算）
			size_t				bytesMaxUsedTexture ;
			size_t				bytesUsedVBO ;		// VBO 使用容量
			size_t				bytesMaxUsedVBO ;
		} ;
		// パフォーマンスログ・フレーム開始
		virtual void BeginFramePerformanceLog( void ) = 0 ;
		// パフォーマンスログ・フレーム終了
		virtual void EndFramePerformanceLog( void ) = 0 ;
		// パフォーマンスログ・デバッグ出力
		virtual void DebugTracePerformanceLog( PerformanceLogInfo * pli = nullptr ) = 0 ;

	public:
		// シェーダ―ソース
		class	ShaderSourceObject
		{
		public:
			SSystem::SArray<char>	m_source ;
			SSystem::SString		m_path ;
		} ;
		// ユニフォーム定義
		class	UniformDescriptor
		{
		public:
			S3DCustomShader::UniformType	m_type ;
			size_t							m_count ;
			SSystem::SString				m_name ;
			SSystem::SXMLDocument			m_xmlDesc ;
		} ;
		// シェーダ―定義
		class	ShaderDescriptor	: public SSystem::SObject
		{
		public:
			ShaderSourceInfo							m_ssi ;
			ShaderSourceObject							m_srcLighting ;
			ShaderSourceObject							m_srcVertex ;
			ShaderSourceObject							m_srcFragment ;
			ShaderSourceObject							m_srcGeometry ;
			ShaderSourceObject							m_srcCompute ;
			SSystem::SObjectArray<UniformDescriptor>	m_uniforms ;
		public:
			ESL_DECLARE_CLASS_INFO( ShaderDescriptor, SObject )
			// 構築関数
			ShaderDescriptor( void ) ;
			// 消滅関数
			~ShaderDescriptor( void ) ;
			// XML 記述 <shader>
			SGLError ParseDescriptor( const SSystem::SXMLDocument& xmlShader ) ;
		} ;
		// カスタムシェーダーコンパイル／ロードと登録
		// 登録済みの場合にはそのシェーダ―を取得
		// pShdDsc が nullptr の時にはデフォルトシェーダー
		S3DCustomShader * BuildCustomShader
				( const wchar_t * pwszID,
					const ShaderDescriptor * pShdDsc,
					S3DCustomShader::CompileListener * pListener = NULL,
					SSystem::SString * pstrErrorMessage = NULL,
					SSystem::SString * pstrErrorSource = NULL ) ;

	protected:
		class	MakeShaderProc	: public SSystem::SProcedure
		{
		protected:
			SSystem::SSignalEvent				m_eventDone ;
			SGLError							m_errResult ;
			S3DRenderDevice *					m_pDevice ;
			S3DCustomShader *					m_pShader ;
			const wchar_t *						m_pwszID ;
			const ShaderDescriptor *			m_pShdDsc ;
			S3DCustomShader::CompileListener *	m_pListener ;
			SSystem::SString *					m_pstrErrorMessage ;
			SSystem::SString *					m_pstrErrorSource ;

		public:
			MakeShaderProc
				( S3DRenderDevice * pDevice,
					const wchar_t * pwszID,
					const ShaderDescriptor * pShdDsc,
					S3DCustomShader::CompileListener * pListener = NULL,
					SSystem::SString * pstrErrorMessage = NULL,
					SSystem::SString * pstrErrorSource = NULL ) ;
			virtual void Run( void ) ;
			virtual void Finalize( void ) ;
			SSystem::SError Wait( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
			S3DCustomShader * GetResult( void ) const ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// コンピュート・シェーダー・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DComputeShaderInterface	: public ESLObject
	{
	public:
		typedef	S3DRenderDevice::DimSize	DimSize ;

		enum	BarrierFlag
		{
			barrierImage	= 0x0001,
			barrierVertex	= 0x0002,
			barrierElement	= 0x0004,
			barrierUniform	= 0x0008,
			barrierTexture	= 0x0010,
		} ;
		struct	ExecuteParam
		{
			DimSize							dimWorkGroups ;
			S3DCustomShader::UniformSet *	pUniforms ;
			uint32_t						nBarrierFlags ;
		} ;
		struct	SourceInfo
		{
			S3DRenderDevice::ShaderSourceType	typeSource ;	// GLSL
			int									versionType ;	// not used, must be zero
			DimSize								dimLocalSize ;	// local size
			S3DRenderDevice::ShaderSource		source ;		// not included header about version, local size
		} ;

	protected:
		// Execute() 呼び出し
		class	ExecuterProc	: public SSystem::SProcedure
		{
		protected:
			S3DComputeShaderInterface *	m_pShader ;
			S3DRenderDevice *			m_pDevice ;
			ExecuteParam				m_param ;
			SGLError					m_errResult ;
		public:
			ESL_DECLARE_CLASS_INFO( ExecuterProc, SProcedure )
			ExecuterProc
				( S3DComputeShaderInterface * pShader,
					S3DRenderDevice * pDevice, const ExecuteParam& param )
				: m_pShader( pShader ), m_pDevice( pDevice ),
					m_param( param ), m_errResult( sglErrFailed ) { }
			SGLError GetResult( void ) const
			{
				return	m_errResult ;
			}
			virtual void Run( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DComputeShaderInterface, ESLObject )
		// ローカルサイズ取得
		virtual DimSize GetWorkLocalSize( void ) const = 0 ;
		// 実行（レンダリングスレッド上で）
		virtual SGLError Execute
			( S3DRenderDevice * pDevice, const ExecuteParam& param ) = 0 ;
		// 実行（呼び出しスレッドは任意）
		virtual SGLError SyncExecute
			( S3DRenderDevice * pDevice, const ExecuteParam& param ) ;
		virtual SGLError SyncExecute
			( S3DRenderDevice * pDevice,
				const DimSize& dimWorkGroups,
				S3DCustomShader::UniformSet * pUniforms = nullptr,
				uint32_t nBarrierFlags = barrierImage | barrierTexture ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// シェーダー・パラメーター・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DShaderParameterInterface	: public	ESLObject
	{
	protected:
		SSystem::SCriticalSection	m_mutexShaderParameter ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DShaderParameterInterface, ESLObject )
		// シェーダーパラメータへの排他処理
		// ※クラスメソッド内では同期しないので必要に応じて手動で呼び出す
		void LockParameter( void ) const
		{
			m_mutexShaderParameter.Lock() ;
		}
		void UnlockParameter( void ) const
		{
			m_mutexShaderParameter.Unlock() ;
		}
		const SSystem::SCriticalSection * GetParameterMutex( void ) const
		{
			return	&m_mutexShaderParameter ;
		}
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) = 0 ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コンピュート・シェーダー・パラメーター・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DComputeShaderParameterInterface	: public	S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DComputeShaderParameterInterface, S3DShaderParameterInterface )

	public:
		// 現在のパラメータに合致する実行ワークグループサイズを計算
		virtual S3DComputeShaderInterface::DimSize
						CalcWorkGroupSize( void ) const = 0 ;
		// 実行（呼び出しスレッドは任意）
		virtual SGLError SyncExecute
			( S3DRenderDevice * pDevice,
				uint32_t nBarrierFlags = S3DComputeShaderInterface::barrierImage
										| S3DComputeShaderInterface::barrierTexture ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DrawWithDepth シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DDrawWithDepthShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DDrawWithDepthShaderInterface, S3DShaderParameterInterface )
		// ｚバッファ設定
		virtual void SetDepthBuffer( SGLImageObject * pDepthBuf ) = 0 ;
		// ｚバッファ透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// GaussianBlur シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DGaussianBlurShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DGaussianBlurShaderInterface, S3DShaderParameterInterface )
		// ガウス係数設定
		virtual void SetGauss( double g ) = 0 ;
		// サンプリング方向設定
		virtual void SetDirection( double x, double y ) = 0 ;
		// 輝度設定
		virtual void SetBrightness( double b ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// GaussianBlur（放射状）シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DGaussianRadialBlurShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DGaussianRadialBlurShaderInterface, S3DShaderParameterInterface )
		// ガウス係数設定
		virtual void SetGauss( double g ) = 0 ;
		// 放射中心設定
		virtual void SetCenter( double x, double y ) = 0 ;
		// 輝度設定
		virtual void SetBrightness( double b ) = 0 ;
		// サンプリング単位距離設定
		virtual void SetSamplingUnit( double d ) = 0 ;
		// ブラースケール : k = (r / x) ^ p,  r : 放射中心からの変位
		virtual void SetBlueScale( double x, double p ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DepthBlender シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DDepthBlenderShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DDepthBlenderShaderInterface, S3DShaderParameterInterface )
		// ｚバッファ設定
		virtual void SetDepthBuffer( SGLImageObject * pDepthBuf ) = 0 ;
		// 焦点深度値設定
		virtual void SetFocusDepth( double zFocus ) = 0 ;
		// ぼかし深度幅設定
		virtual void SetDepthRange( double zNearRange, double zFarRange ) = 0 ;
		// ｚ値変換用係数設定
		virtual void SetPersParameter( double m22, double m23 ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DelayLight シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DDelayLightShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DDelayLightShaderInterface, S3DShaderParameterInterface )
		// 光源情報設定（座標は透視空間）
		virtual void SetLightParam( const struct S3DLightEntry & light ) = 0 ;
		// 光源描画効果
		virtual void SetLightApplication
					( float32_t fpDiffusion, float32_t fpSpecular ) = 0 ;
		// 大気散乱効果
		virtual void SetAirScattering
					( float32_t fpScattering, float32_t fpDistanceUnit ) = 0 ;
		// 透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) = 0 ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SSGI シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DSSGISamplerInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSSGISamplerInterface, S3DShaderParameterInterface )
		// AO 到達距離
		virtual void SetAOReachDistance
				( float32_t fpReach, float32_t fpZProportion ) = 0 ;
		// AO サンプリング数
		virtual void SetAOSamplingCount( size_t nCount ) = 0 ;
		// GI サンプリング数
		virtual void SetGISamplingCount( size_t nCount ) = 0 ;
		// GI 輝度
		virtual void SetGILuminousness( float32_t fpDiffusion ) = 0 ;
		// 透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) = 0 ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) = 0 ;
		// 3面パノラマフレーム有効化
		virtual void Enable3WayPanoramaBuffer( bool fEnable ) = 0 ;
		// 3面パノラマ透視変換行列
		virtual void Set3WayPerspective( const S4DMatrix& matPers ) = 0 ;
		// レンダリング済み3面パノラマバッファ
		virtual void Set3WayPanoramaBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) = 0 ;
	} ;

	class	S3DSSGIComposerInterface	: public S3DShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSSGIComposerInterface, S3DShaderParameterInterface )
		// サンプリングスケール
		virtual void SetSamplingScale( const S2DVector& vScale ) = 0 ;
		// AO/GI 適用度
		virtual void SetBlendRatio( float32_t fpAO, float32_t fpGI ) = 0 ;
		// AO 影加算色
		virtual void SetAOShadeColor( const SGLPalette& rgbShade ) = 0 ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs, size_t nColorBufCount ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 水面描画シェーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DSimpleWaterShaderInterface	: public S3DShaderParameterInterface
	{
	public:
		struct	WaterParam
		{
			float32_t	fpAmplitude ;	// 振幅
			float32_t	fpFrequency ;	// 周波数 [Hz/2π]
			float32_t	radTime ;		// 時間 [rad]
			S2DVector	vDirection ;	// 方向
		} ;
		struct	Parameter
		{
			WaterParam	waterVertex[4] ;	// 頂点効果波形
			WaterParam	waterBump[4] ;		// 法線効果波形
			S3DVector	vLevelAxisX ;		// 水面基底ベクトル
			S3DVector	vLevelAxisY ;
			float32_t	fpAmplitude ;
			float32_t	fpNormalAmp ;
			float32_t	zCascadeFar ;
			float32_t	fpCascadePhase[1] ;
			float32_t	fpCascadeAmp[1] ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSimpleWaterShaderInterface, S3DShaderParameterInterface )
		// パラメータ設定
		virtual void SetParameter( const Parameter& param ) = 0 ;
		// 時間を更新
		virtual void SetTimeParameter
			( const float32_t * pTime, const float32_t * pBumpTime ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Shadowmap 用 5x5 ループフィルタ (Compute Shader)
	//////////////////////////////////////////////////////////////////////////

	class	S3DShadowmapDepthFilter5x5Interface	: public S3DComputeShaderParameterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DShadowmapDepthFilter5x5Interface, S3DComputeShaderParameterInterface )
		// 入力画像
		virtual void SetInputDepth( SGLImageObject * pInput ) = 0 ;
		// 出力画像
		virtual void SetOutputDepth( SGLImageObject * pOutput ) = 0 ;
		// フィルター行列 (5x5)
		virtual void SetFilterKernel( const float32_t * pKernel ) = 0 ;
		// ガウスフィルター設定
		void SetGaussianFilter( float32_t g ) ;
	} ;

}

#endif

