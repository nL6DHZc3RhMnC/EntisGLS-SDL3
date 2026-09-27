
#if	!defined(__SAKURAGL_OPENGL_CONTEXT_H__)
#define	__SAKURAGL_OPENGL_CONTEXT_H__

#include <sakuragl/sgl3d/sgl_render_buffer.h>
#include <sakuragl/sgl3d/sglh3d_stddef.h>
#include <sakuragl/sgl_opengl_extension.h>

namespace	SakuraGL
{
	class	SGLOpenGLContext ;
	class	SGLOpenGLView ;
	class	SGLOpenGLVertexBuffer ;
	class	SGLOpenGLFrameBuffer ;

	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLShaderProgram	: public S3DCustomShader
	{
	protected:
		SGLOpenGLContext *	m_pOpenGL ;
		GLuint				m_glProgram ;
		GLuint				m_glVertexShader ;
		GLuint				m_glGeometryShader ;
		GLuint				m_glFragmentShader ;
		GLuint				m_glComputeShader ;

		bool				m_flagComputeShader ;
		S3DComputeShaderInterface::DimSize
							m_dimComputeLocalSize ;

		GLfloat				m_fpAnisotropy ;

		bool						m_flagProgramBinary ;
		GLenum						m_glBinaryFormat ;
		SSystem::SArray<uint8_t>	m_bufProgramBinary ;

		SSystem::SArray<GLchar>	m_aSrcVertex ;
		SSystem::SArray<GLchar>	m_aSrcFragment ;
		SSystem::SArray<GLchar>	m_aSrcGeometry ;
		SSystem::SArray<GLchar>	m_aSrcCompute ;

		SSystem::SString	m_strShaderSrc ;
		SSystem::SString	m_strErrorLog ;

	public:
		// Uniform データ
		class	CustomUniform	: public S3DCustomShader::UniformData
		{
		public:
			GLint		m_glIndex ;
			size_t		m_iTexture ;
			GLuint		m_glBinding ;
			GLenum		m_glAccess ;
			GLenum		m_glFormat ;
		public:
			CustomUniform( void ) ;
			CustomUniform( const CustomUniform& cuni ) ;
			~CustomUniform( void ) ;
		} ;
		class	CustomUniformSet
					: public SSystem::SStrSortObjectArray<CustomUniform>
		{
		public:
			// 構築関数
			CustomUniformSet( void ) { }
			CustomUniformSet( const CustomUniformSet& cus )
				: SSystem::SStrSortObjectArray<CustomUniform>( cus ) { }
		} ;

	protected:
		CustomUniformSet	m_cusUniform ;
		size_t				m_nUseTextures ;
		size_t				m_nUseImages ;
		bool				m_flagUpdateUniform ;

	public:
		// ソースコード
		typedef	S3DRenderDevice::ShaderSource	Source ;
		// ロケーション
		struct	Location
		{
			GLint *			location ;
			const GLchar *	name ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLShaderProgram, S3DCustomShader )
		// 構築関数
		SGLOpenGLShaderProgram( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLShaderProgram( void ) ;
		// シェーダープログラム作成
		virtual SGLError CreateProgram
			( const Source * pVertexSrc, size_t nVertexSrcCount,
				const Source * pGeometrySrc, size_t nGeometrySrcCount,
				const Source * pFragmentSrc, size_t nFragmentSrcCount,
				CompileListener * pListener = NULL ) ;
		// コンピュートシェーダープログラム作成
		virtual SGLError CreateCumputeShader
			( const Source * pSrc, size_t nSrcCount,
				const S3DComputeShaderInterface::DimSize& dimLocalSize,
				CompileListener * pListener = NULL ) ;
		// シェーダープログラム保存
		virtual SGLError SaveProgramBinary( S3DShaderBinary& bin ) ;
		// シェーダープログラム復元
		virtual SGLError LoadProgramBinary
			( const S3DShaderBinary& bin,
				CompileListener * pListener = NULL ) ;
		// シェーダープログラム解放
		virtual void Release( void ) ;
		// シェーダープログラムが設定された（変更された）
		virtual void OnChangedProgram( void ) ;
		// シェーダープログラムが別のプログラムに変更される
		virtual void OnChangingProgram( void ) ;
		// ユニフォーム値更新
		virtual void UpdateCustomUniform( bool flagForceUpdate = false ) ;
		// Flush 処理
		virtual void OnFlushContext( void ) ;
		// 結合されたソース取得
		const SSystem::SArray<GLchar>& GetVertexShaderSource( void ) const ;
		const SSystem::SArray<GLchar>& GetFragmentShaderSource( void ) const ;
		const SSystem::SArray<GLchar>& GetGeometryShaderSource( void ) const ;
		const SSystem::SArray<GLchar>& GetComputeShaderSource( void ) const ;
		// コンパイル／リンクエラーログを取得
		virtual const SSystem::SString& GetCompileErrorLog( void ) const ;
		// エラーになったシェーダーソースを取得
		virtual const SSystem::SString& GetLastErrorShaderSource( void ) const ;

	protected:
		// シェーダーのコンパイル結果をチェック
		static bool IsShaderCompiled
			( GLuint glShader, SSystem::SString& strErrorLog ) ;
		// プログラムのリンク結果をチェック
		static bool IsProgramLinked
			( GLuint glProgram, SSystem::SString& strErrorLog ) ;
		// プログラムバイナリを取得
		static bool GetProgramBinary
			( GLuint glProgram, GLenum& glFormat,
					SSystem::SArray<uint8_t>& bufBinary ) ;

	public:
		// 属性ロケーション取得
		size_t GetAttributeLocations
			( const Location * pLocations, size_t nCount ) const ;
		// ユニフォームロケーション取得
		size_t GetUniformLocations
			( const Location * pLocations, size_t nCount ) const ;

	public:
		// 頂点属性配列有効化
		void EnableVertexAttribArray( GLuint iAttr ) ;
		// 頂点属性配列無効化
		void DisableVertexAttribArray( GLuint iAttr ) ;
		// 頂点属性配列設定
		void VertexAttribPointer
			( GLuint iAttr, GLint size, GLenum type,
				GLboolean normalized,
				GLsizei stride, const GLvoid* pointer, GLuint divisor = 0 ) ;

	public:
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix( const S4DMatrix& mat4 ) = 0 ;
		// 投影スクリーン座標設定
		virtual void SetProjectionScreen
			( float32_t xScreen, float32_t yScreen, float32_t zScreen ) = 0 ;
		// モデル変換行列設定
		virtual void SetModelViewMatrix
				( const S4DMatrix& mat4, bool fInverseNormal = false ) = 0 ;

		// 異方性フィルタ値取得
		float GetAnisotropy( void ) const ;
		// 異方性フィルタ値設定
		void SetAnisotropy( float anisotropy ) ;

	public:
		// 4x4 行列ユニフォーム設定 (OpenGL ES 互換ラッパ)
		static void glUniformMatrix4f
			( GLint location, GLboolean transpose, const S4DMatrix& mat4 ) ;
		static void UniformMatrix4fv
			( GLint location, GLsizei count,
				GLboolean transpose, const S4DMatrix * mat4s ) ;

	public:
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;
		// カスタムユニフォームテクスチャ数取得
		virtual size_t GetCustomTextureCount( void ) const ;
		// ユニフォーム指標取得（RegisterCustomUniform で変動）
		virtual ssize_t FindCustomUniform( const wchar_t * pszUniform ) ;
		// ユニフォーム値設定
		virtual SGLError SetCustomUniform
			( size_t iUniform,
				S3DCustomShader::UniformType type,
				const void * pData, size_t nCount ) ;
		// ユニフォーム指標定義
		virtual void RegisterCustomUniform
			( const wchar_t * pszUniformID,
				S3DCustomShader::UniformType type, size_t nCount ) ;
		// ユニフォーム数取得
		size_t GetCustomUniformCount( void ) const ;
		// ユニフォーム取得
		CustomUniform * GetCustomUniformAt( size_t nIndex ) const ;
		// ユニフォーム値反映
		SGLError ReflectCustomUniform( CustomUniform * pcu ) ;
		// ユーザーテクスチャ番号 → OpenGL バインドテクスチャ番号
		virtual int GLTextureNumAtUserTexture( size_t iUserTexture ) const ;
		// 画像フォーマット → GL画像フォーマット
		static GLenum GetGLImageFormatOf( SGLImageObject * pTexture ) ;
		// layered として画像をバインドするか？
		static GLboolean IsLayeredGLImage( SGLImageObject * pTexture ) ;

	protected:
		SSystem::SArray<GLfloat>	m_bufMatrixTranspose ;

		// 2x2 行列ユニフォーム反映
		void UniformMatrix2fv
			( GLint location, GLsizei count, const float32_t* value ) ;
		// 3x3 行列ユニフォーム反映
		void UniformMatrix3fv
			( GLint location, GLsizei count, const float32_t* value ) ;
		// 4x4 行列ユニフォーム反映
		void UniformMatrix4fv
			( GLint location, GLsizei count, const float32_t* value ) ;

		friend class SGLOpenGLContext ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// OpenGL VertexBufferObject
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLVertexBuffer	: public S3DVertexDeviceBufferInterface
	{
	public:
		// 頂点要素インターリーブ
		struct	VertexElementVNTC
		{
			S3DVector	vertex ;
			S3DVector	normal ;
			S2DVector	uv ;
			S3DColor	color ;
		} ;
		// ArrayBufferObject entry
		struct	ArrayBufferEntry
		{
			GLuint				m_glArrayBuffer ;
			bool				m_fUpdate ;
			ssize_t				m_iBaseMesh ;
			ssize_t				m_iMorphMesh ;
			size_t				m_nBoneCount ;
		} ;
		class	ArrayBufferList	: public SSystem::SArray<ArrayBufferEntry>
		{
		public:
			S3DCustomShader *	m_pLocShader ;
		} ;
		enum	ConstantValue
		{
			maxVAOShaderLRU	= 4,
		} ;
		enum	UpdateElementFlag
		{
			elementVertex			= 0x0001,
			elementNormal			= 0x0002,
			elementUVMap			= 0x0004,
			elementColor			= 0x0008,
			elementMorphVertex		= 0x0010,
			elementMorphNormal		= 0x0020,
			elementMorphUVMap		= 0x0040,
			elementMorphColor		= 0x0080,
			elementMorphing			= 0x00F0,
			elementIndex			= 0x0100,
			elementSubIndex0		= 0x0200,
			elementSubIndex1		= 0x0400,
			elementSubIndex2		= 0x0800,
			elementSubIndex3		= 0x1000,
			elementSubIndex			= 0x1E00,
			elementWeightMap		= 0x2000,
			elementExAttrElements	= 0x4000,
			elementAll				= 0xFFFF,
		} ;

		// OpenGL リソース（SGLOpenGLContext 毎）
		class	GLResource	: public S3DRenderDevice::Notify
		{
		public:
			SGLOpenGLVertexBuffer *	m_pVertexBuf ;
			GLResource *	m_pNextRsrc ;
			SSystem::SSmartReference<SGLOpenGLContext>
							m_refOpenGL ;
			uint32_t		m_maskUpdateElements ;	// complex of enum UpdateElementFlag
			bool			m_flagDynamicMesh ;
			bool			m_flagInterleaved ;		// 頂点要素をインターリーブする
			size_t			m_iFirstUpdate ;
			size_t			m_iEndUpdate ;
			GLuint			m_glVertexBuffer ;
			GLuint			m_glElementBuffer ;
			GLuint			m_glInstancingBuffer ;
			GLenum			m_typeElementIndex ;
			size_t			m_bytesAllocVertex ;
			size_t			m_bytesAllocElement ;
			size_t			m_bytesAllocInstancing ;
			//
			struct	InstanceEntry
			{
				GLfloat		matrix[3][4] ;
				S3DColor	color ;
			} ;
			SSystem::SArray<VertexElementVNTC>
							m_bufVertexElementVNTC ;
			SSystem::SArray<float32_t>
							m_bufWeightTempBuf ;
			SSystem::SArray<uint16_t>
							m_bufElementTempBuf ;
			SSystem::SArray<InstanceEntry>
							m_bufInstancingTempBuf ;
			SSystem::SObjectArray<ArrayBufferList>
							m_aVAOList ;
			SSystem::SObjectArray<SGLImageObject>
							m_aVertexTexture ;
		public:
			// 構築関数
			GLResource( SGLOpenGLVertexBuffer * pVertexBuf )
				: m_pVertexBuf(pVertexBuf),
					m_pNextRsrc(NULL),
					m_maskUpdateElements(0),
					m_flagDynamicMesh(false),
					m_flagInterleaved(false),
					m_iFirstUpdate(0), m_iEndUpdate(0),
					m_glVertexBuffer(0), m_glElementBuffer(0),
					m_glInstancingBuffer(0),
					m_typeElementIndex(GL_UNSIGNED_INT),
					m_bytesAllocVertex(0), m_bytesAllocElement(0),
					m_bytesAllocInstancing(0) { }
			// 消滅関数
			~GLResource( void ) ;
			// バッファ確保
			bool AllocateBuffer( size_t bytesVertex, size_t bytesElement ) ;
			// バッファ関連付け
			void BindBuffer( void ) ;
			// バッファ分離
			void UnbindBuffer( void ) ;
			// 頂点バッファ書き込み
			void WriteVertexBuffer
				( size_t iOffset, const void * ptrData, size_t bytesData ) ;
			void WriteComposedVertexBuffer
				( size_t ofsVertex,
					const S3DVector4 * pvVertex,
					const S3DVector4 * pvNormal,
					const S2DVector * pvUVMap,
					const S3DColor * pColor,
					size_t countVertex, size_t iSrcOffset = 0 ) ;
			// ボーンウェイトマップ書き込み
			void WriteBoneWeightBuffer
				( size_t iOffset, const float32_t * ptrData,
							size_t nVertexCount, size_t nBoneCount ) ;
			// 指標バッファ書き込み
			void WriteElementBuffer
				( size_t iOffset, const void * ptrData, size_t bytesData ) ;
			// 指標バッファ書き込み（m_typeElementIndex に合わせて自動変換）
			void WriteIndexedList
				( size_t iOffset, const uint32_t * pIndexedList, size_t nIndexCount ) ;
		public:
			// インスタンシングバッファ確保
			bool AllocateInstancingBuffer( size_t nCount ) ;
			// バッファ関連付け
			void BindInstancingBuffer( void ) ;
			// インスタンシングバッファ書き込み
			void WriteInstancingBuffer
				( const S4DMatrix * pMatrixs,
					const S3DColor * pColors, size_t nCount ) ;
		public:
			// 頂点要素をインターリーブするか？
			bool IsInterleaveVertexElement
				( const S3DRenderBuffer::RENDER_ENTRY * pre ) const ;
		public:
			// VAO エントリ取得
			ArrayBufferEntry * GetArrayBufferAt
				( S3DCustomShader * pShader, size_t i, size_t nEntryCount ) ;
			ArrayBufferList * GetArrayBufferListFor( S3DCustomShader * pShader ) ;
		public:
			// デバイスの削除前に呼び出される
			virtual void OnReleaseDevice( S3DRenderDevice * pDev ) ;
		} ;

	protected:
		// バッファ破棄
		class	BufferDestroyer	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *					m_pOpenGL ;
			SGLOpenGLVertexBuffer::GLResource *	m_pRsrc ;
			GLuint								m_glVertexBuffer ;
			GLuint								m_glElementBuffer ;
			GLuint								m_glInstancingBuffer ;
			SSystem::SArray<GLuint>				m_aVAOs ;
		public:
			// 構築関数
			BufferDestroyer
				( SGLOpenGLContext * pOpenGL,
					SGLOpenGLVertexBuffer::GLResource * pRsrc,
					GLuint glVertexBuffer, GLuint glElementBuffer,
					GLuint glInstancingBuffer,
					const SSystem::SObjectArray<ArrayBufferList>& lstVAO ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

	protected:
		SSystem::SCriticalSection	m_csSync ;
		GLResource *				m_pFirstGLRsrc ;
		S3DRenderBuffer *			m_pRenderBuf ;

	public:
		// エントリ情報
		typedef	S3DRenderBuffer::RENDER_ENTRY	RENDER_ENTRY ;
		struct	ENTRY_INFO
		{
			RENDER_ENTRY *	pre ;
			bool			flagMustShapeByCPU ;
			bool			flagVertexTexture ;
			size_t			ofsVertex ;
			size_t			ofsElement ;
			size_t			ofsSubElements[S3DRenderBuffer::countSubMesh] ;
			size_t			ofsMorph ;
			size_t			ofsBone ;
			SGLSize			sizeVertexTex ;
			size_t			yVTMorphVertex ;
			size_t			yVTMorphNormal ;
			size_t			yVTMorphStride ;
			size_t			yVTBoneWeight ;
			size_t			yVTBoneIndex ;
			size_t			yVTExAttrElements ;
			size_t			xVTExAttrStride ;
			size_t			yVTMorphInstance ;
			size_t			yVTBoneInstance ;
			size_t			nMaxInstance ;
		} ;
		enum	ConstantValues
		{
			OFFSET_VERTEX		= 0,
			OFFSET_NORMAL		= OFFSET_VERTEX + sizeof(S3DVector4),
			OFFSET_UV_MAP		= OFFSET_NORMAL + sizeof(S3DVector4),
			OFFSET_COLOR_MAP	= OFFSET_UV_MAP + sizeof(S2DVector),
			SIZEOF_ELEMENT		= sizeof(S3DVector4) * 4
									+ sizeof(S2DVector) + sizeof(S3DColor),
		} ;

	protected:
		SSystem::SArray<ENTRY_INFO>	m_arrEntryInfo ;
		SSystem::SObjectArray<SGLImageObject>
									m_arrVertexTexture ;
		size_t						m_bytesVertex ;
		size_t						m_bytesElement ;

 	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLVertexBuffer, S3DVertexDeviceBufferInterface )
		// 構築関数
		SGLOpenGLVertexBuffer( S3DRenderBuffer * prb ) ;
		// 消滅関数
		virtual ~SGLOpenGLVertexBuffer( void ) ;
		// VBO を OpenGL 用にコミット
		static SGLOpenGLVertexBuffer * Commit( S3DVertexBufferInterface * pVB ) ;

		// バッファ更新通知
		void NotifyUpdateBuffer
			( size_t iMeshFirst, size_t iMeshEnd,
						uint32_t maskUpdateElements ) ;
		// GLResource 取得（バッファ更新確定）
		GLResource * CommitResourceAs( SGLOpenGLContext * pOpenGL ) ;

	protected:
		void CommitVertexBuffer
			( GLResource * pRsrc,
				SGLImageObject * pVTex,
				const ENTRY_INFO& ei, RENDER_ENTRY * pre,
				uint32_t maskUpdateElements ) ;
		void CommitVertexTextureBuffer
			( GLResource * pRsrc,
				const SGLImageInfo& imginf, uint8_t * pbytBuf,
				const ENTRY_INFO& ei, RENDER_ENTRY * pre,
				uint32_t maskUpdateElements ) ;
		void WriteMorphVertexBuffer
			( const SGLImageInfo& imginf, uint8_t * pbytBuf,
				size_t yLine, const S3DVector4 * pvMorph,
				const float32_t * pfpWeight,
				const S3DVector4 * pvOrgVertex, size_t nCount ) ;
		void WriteWeightMapBuffer
			( const SGLImageInfo& imginf, uint8_t * pbytBuf,
				size_t yLine, float32_t *const* ppWeight,
				size_t nWeightCount, size_t nVertexCount ) ;
		void WriteJointMapBuffer
			( const SGLImageInfo& imginf, uint8_t * pbytBuf,
				size_t yLine, uint32_t *const* ppJoint,
				size_t nWeightCount, size_t nVertexCount ) ;
		void WriteExAttributeElementsBuffer
			( const SGLImageInfo& imginf, uint8_t * pbytBuf,
				size_t yLine, const float32_t * pfpElements,
				size_t nElementsCount, size_t nVertexCount ) ;
	public:
		// GLResource 取得
		static GLResource * GetResourceAs
			( SGLOpenGLVertexBuffer * pVertBuf, SGLOpenGLContext * pOpenGL ) ;
		// GLResource 削除
		void ReleaseResource( GLResource * pRsrc ) ;
		// エントリ情報取得
		ENTRY_INFO * GetEntryInfoAt( size_t iMesh ) ;
		SGLImageObject * GetVertexTextureAt( size_t iMesh ) ;
		// モーフィング・インスタンス書き込み
		static void WriteMorphInstanceToVertexTexture
			( SGLImageObject * pImage, const ENTRY_INFO& ei,
				const S3DVector4 * pvMorphInstance, size_t nCount ) ;
		static void MakeMorphInstance
			( S3DVector4 * pvMorphInstance,
				S3DVertexVariantBuffer * pvvb, size_t iMesh ) ;
		// ボーンインスタンスを頂点テクスチャに書き込む
		static void WriteBoneInstanceToVertexTexture
			( SGLImageObject * pImage, size_t iMesh,
				const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
				S3DMatrix * pMatrixBuf, S3DVector * pvTransBuf,
				S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount ) ;

	protected:
		void AllocVertexTextureSize( ENTRY_INFO& ei, RENDER_ENTRY * pre ) ;
		void AllocBufferOffset( ENTRY_INFO& ei, RENDER_ENTRY * pre ) ;

	public:
		// 描画の確定時処理
		virtual void OnFlush( S3DVertexBufferInterface * pVB ) ;
		// プリミティブリストを更新時処理
		virtual void OnUpdateIndexedPrimitiveList
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// サブメッシュ（ポリゴンリスト）を更新時処理
		virtual void OnUpdateSubIndexedTriangleList
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t iSubMesh, uint32_t nFlags,
				size_t countPolygon, const uint32_t * pIndexedList ) ;
		// 追加的な頂点属性を設定時処理
		virtual void OnSetExtendVertexAttribute
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t countElements,
				size_t countVertex, const float32_t * pfpAttrElements ) ;
		// メッシュにウェイトマップを設定時処理
		virtual void OnSetBoneWeightMap
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
		// メッシュにモーフターゲット枠を確保時処理
		virtual void OnAllocateMorphing
			( S3DVertexBufferInterface * pVB, size_t iMesh, size_t nCount ) ;
		// メッシュにモーフターゲットを設定時処理
		virtual void OnSetMorphingTargetMesh
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t iMorph, size_t countVertex,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// バッファ消去時処理
		virtual void OnClearBuffer( S3DVertexBufferInterface * pVB ) ;

	protected:
		static const GLenum	m_glDrawMode[primitiveCount] ;
	public:
		// S3DPrimitiveType -> GLenum 変換
		static GLenum PrimitiveTypeToGL( S3DPrimitiveType typePrimitive ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SGLImageBuffer - OpenGL テクスチャ変換 SGLImageBufferInterface
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLTextureBuffer	: public SGLImageBufferInterface
	{
	public:
		// OpenGL リソース（SGLOpenGLContext 毎）
		class	GLResource
				: public SSystem::SObject, public S3DRenderDevice::Notify
		{
		public:
			SGLOpenGLTextureBuffer *	m_pTextureBuf ;
			GLResource *	m_pNextRsrc ;
			SSystem::SSmartReference<SGLOpenGLContext>
							m_refOpenGL ;
			bool			m_flagOwnTexture ;
			bool			m_flagClear ;
			bool			m_flagUpdate ;
			bool			m_flagMipmapped ;
			bool			m_flagMultisample ;
			bool			m_flagSmoothable ;
			bool			m_flagReqTiling ;
			SGLImageRect	m_rectUpdate ;
			size_t			m_zUpdateFirst ;
			size_t			m_zUpdateCount ;
			SGLPalette		m_colorClear ;
			GLuint			m_glTexture ;
			GLuint			m_glRenderBuffer ;
			GLenum			m_paramTarget ;
			GLint			m_paramDefFilter ;
			GLint			m_paramDefWrap ;
			uint32_t		m_fmtNeedsFormat ;
			SGLImageInfo	m_imginf ;
			size_t			m_nTargetCount ;
			size_t			m_nGLTexBytes ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( GLResource, SObject )
			// 構築関数
			GLResource( SGLOpenGLTextureBuffer * pTextureBuf )
				: m_pTextureBuf(pTextureBuf), m_pNextRsrc(NULL),
					m_flagOwnTexture(false),
					m_flagClear(false), m_flagUpdate(true),
					m_flagMultisample(false),
					m_flagMipmapped(false),
					m_flagSmoothable(true),
					m_flagReqTiling(false),
					m_zUpdateFirst(0), m_zUpdateCount((size_t)-1),
					m_glTexture(0), m_glRenderBuffer(0),
					m_paramTarget(GL_TEXTURE_2D),
					m_paramDefFilter(GL_LINEAR),
					m_paramDefWrap(GL_CLAMP_TO_EDGE),
					m_fmtNeedsFormat(0),
					m_nTargetCount(0), m_nGLTexBytes(0) { }
			// 消滅関数
			~GLResource( void ) ;
			// テクスチャ生成
			void CreateGLTexture( SGLImageBuffer * pImageBuf ) ;
			// テクスチャ関連付け
			SGLError AttachGLTexture
				( SGLImageBuffer * pImageBuf,
						GLuint glTexture, bool fAutoDelete ) ;
			// テクスチャ破棄
			void DeleteGLTexture( void ) ;
			// レンダーバッファ生成
			void CreateGLRenderbuffer( SGLImageBuffer * pImageBuf ) ;
		public:
			// デバイスの削除前に呼び出される
			virtual void OnReleaseDevice( S3DRenderDevice * pDev ) ;
		} ;

	protected:
		SSystem::SCriticalSection	m_csSync ;
		GLResource *				m_pFirstGLRsrc ;

	public:
		// 同期
		void Lock( void ) const ;
		void Unlock( void ) const ;
		// GLResource 取得
		static GLResource * GetResourceAs
			( SGLOpenGLTextureBuffer * pTxtBuf, SGLOpenGLContext * pOpenGL ) ;
		// GLResource 削除
		void ReleaseResource( GLResource * pRsrc ) ;

	public:
		SGLImageBuffer *	m_pImageBuf ;
		GLResource *		m_pRsrc ;
		bool				m_flagMipmap ;

		// OpenGL ピクセルフォーマット
		struct	GL_PIXEL_FORMAT
		{
			GLenum	glTarget ;
			GLint	glInternalFormat ;
			GLenum	glFormat ;
			GLenum	glType ;
			GLint	glLayer ;

			GL_PIXEL_FORMAT( void ) : glLayer(0) {}
			GL_PIXEL_FORMAT( const SGLImageBuffer& imginf )
				{	FromImageInfo( imginf ) ;	}
			SGLError FromImageInfo( const SGLImageBuffer& imginf ) ;
			bool IsTexture2D( void ) const ;
			bool IsTexture2DArray( void ) const ;
			bool IsTexture3D( void ) const ;
			bool IsTextureMultisample( void ) const ;
			bool IsCubemap( void ) const ;
			bool IsFormatRGB( void ) const ;
			bool IsFormatCompressedRGB( void ) const ;
			bool IsFormatCompressedSource( void ) const ;
			bool IsFormatFloatRGB( void ) const ;
			bool IsFormatFloat( void ) const ;
			bool MakeUncompressed( void ) ;
			bool IsFormatGray( void ) const ;
			bool IsFormatDepth( void ) const ;
		} ;

	protected:
		// GL_BGRA_EXT 形式のテクスチャでの glTexImage2D 関数の対応状況
		static bool		m_flagNotSupportedBGRA ;
		static size_t	m_countRetryTextureBGRA ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLTextureBuffer, SGLImageBufferInterface )
		// 構築関数
		SGLOpenGLTextureBuffer( void ) ;
		// 消滅関数
		virtual ~SGLOpenGLTextureBuffer( void ) ;
		// OpenGL テクスチャ関連付け
		SGLError AttachGLTexture
			( SGLOpenGLContext * pOpenGL,
				SGLImageBuffer * pImageBuf,
				GLuint glTexture, bool fAutoDelete ) ;

	public:	// SGLImageBufferInterface
		// クリア通知
		virtual SGLError ClearBuffer
			( SGLImageBuffer * pImageBuf, SGLPalette pxClear ) ;
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 更新確定処理（一度で確定せず少しづつ）
		SGLError CommitBufferProgressively
			( SGLOpenGLContext * pOpenGL,
				SGLImageBuffer * pImageBuf, size_t nProgressivePixelCount ) ;
		// OpenGL テクスチャ生成
		void CreateGLTexture
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf, SGLImageBuffer * pOrgBuf ) ;
		// Mipmap 生成
		SGLError Build2DMipmaps
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf, SGLImageBuffer * pOrgBuf ) ;
		// Cubemap テクスチャを転送
		SGLError UpdateTextureCubemap
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf,
				const SGLSize& sizeFace,
				SGLImageInfo * pRefImage, uint8_t * pbytBuffer ) ;
		// 3D テクスチャを転送
		SGLError UpdateTexture3D
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf,
				SGLImageBuffer * pOrgBuf,
				SGLImageInfo * pRefImage,
				uint8_t * pbytBuffer, size_t nProgressivePixelCount ) ;
		// 2D テクスチャを転送
		SGLError UpdateTexture2D
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf,
				SGLImageBuffer * pOrgBuf,
				SGLImageInfo * pRefImage,
				uint8_t * pbytBuffer, size_t nProgressivePixelCount ) ;
		// 圧縮済み 2D テクスチャを転送
		SGLError UpdateCompressedTexture2D
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf,
				SGLImageBuffer * pOrgBuf,
				SGLImageInfo * pRefImage,
				uint8_t * pbytBuffer, size_t nProgressivePixelCount ) ;
		// GL_RGBA フォーマットで 2D テクスチャの生成を再試行
		void RetryUpdateTexture2DwithRGBA
			( SGLOpenGLContext * pOpenGL,
				GLResource * pRsrc,
				const GL_PIXEL_FORMAT& glpf,
				SGLImageBuffer * pImageBuf,
				SGLImageBuffer * pOrgBuf, uint8_t * pbytBuffer ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;

	public:
		// SGLImageObject から SGLOpenGLTextureBuffer テクスチャ取得
		static SGLOpenGLTextureBuffer * CommitGLTexture
				( SGLOpenGLContext * pOpenGL,
					SGLImageObject * pImage, SGLImageRect& rectRef ) ;
		static SGLOpenGLTextureBuffer::GLResource *
			CommitGLTextureRsrc
				( SGLOpenGLContext * pOpenGL,
					SGLImageObject * pImage, SGLImageRect& rectRef ) ;
		static SGLError CommitGLTextureProgressively
				( SGLOpenGLContext * pOpenGL,
					SGLImageObject * pImage, size_t nProgressivePixelCount ) ;
		// SGLImageObject へ OpenGL テクスチャ関連付け
		static SGLError AttachGLTexture
				( SGLOpenGLContext * pOpenGL,
					SGLImageObject * pImage,
					GLuint glTexture, bool fAutoDelete ) ;

	public:
		// RGBA フォーマット正規化
		static void NormalizePixelFormat
			( SGLImageBuffer * pImageBuf, bool fConvertPixelData ) ;

	protected:
		// テクスチャ破棄
		class	TextureDestroyer	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *	m_pOpenGL ;
			GLuint	m_glTexture ;			// テクスチャ
			GLuint	m_glRenderBuffer ;		// レンダリングバッファ
			size_t	m_bytesUsed ;
		public:
			// 構築関数
			TextureDestroyer
				( SGLOpenGLContext * pOpenGL,
					GLuint glTexture,
					GLuint glRenderBuffer, size_t bytesUsed ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL 標準シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLDefaultShader	: public SGLOpenGLShaderProgram
	{
	public:
		// 定数値
		enum	ConstantValue
		{
			#if	defined(__API_OPEN_GL_ES__)
			MAX_ALL_LIGHT_COUNT		= 5,
			MAX_LIGHT_COUNT			= 4,
			INDEX_LIGHT_FOG			= 4,
			MAX_BONE_PALETTE		= 16,
			MAX_SHADOWMAPPING		= 6,
			#else
			MAX_ALL_LIGHT_COUNT		= 8,
			MAX_LIGHT_COUNT			= 8,
			MAX_BONE_PALETTE		= 16,
			MAX_SHADOWMAPPING		= 12,
			#endif
			BONE_PALETTE_ATTRS		= (MAX_BONE_PALETTE+3)/4,
			ENV_MAPPING_NOTHING		= 0,
			ENV_MAPPING_HEMISPHERE	= 1,
			ENV_MAPPING_SPHERE		= 2,
			ENV_MAPPING_CUBE		= 3,
			ENV_MAPPING_VIEWPORT	= 4,
		} ;
		size_t	m_maxLightCount ;
		size_t	m_maxBonePalette ;
		size_t	m_maxShadowmapping ;
		bool	m_enabledMorphing ;
		bool	m_enabledInstancing ;
		bool	m_enabledVTInstancing ;
		bool	m_preparedDepthBuffer ;

	protected:
		// attribute
		GLint	a_vVertexPosition ;			// vec3 頂点座標
		GLint	a_vVertexNormal ;			// vec3 法線
		GLint	a_vVertexMappingX ;			// vec3 テクスチャマッピングｘ基底
		GLint	a_vVertexMappingY ;			// vec3 テクスチャマッピングｙ基底
		GLint	a_vTextureCoord ;			// vec2 UV座標
		GLint	a_vVertexMulColor ;			// vec4 頂点色
		GLint	a_vVertexAddColor ;			// vec3
		GLint	a_vMorphPosition ;			// vec3 頂点座標（モーフィング）
		GLint	a_vMorphNormal ;			// vec3 法線（モーフィング）
		GLint	a_vMorphMappingX ;			// vec3 テクスチャマッピングｘ基底（モーフィング）
		GLint	a_vMorphMappingY ;			// vec3 テクスチャマッピングｙ基底（モーフィング）
		GLint	a_vMorphTextureCoord ;		// vec2 UV座標（モーフィング）
		GLint	a_vMorphMulColor ;			// vec4 頂点色（モーフィング）
		GLint	a_vMorphAddColor ;			// vec3
		GLint	a_vVertexBoneWeight[BONE_PALETTE_ATTRS] ;
		GLint	a_matInstancingModelView[3] ;	// vec4 インスタンス行列 4x3
		GLint	a_vInstancingMulColor ;		// vec4 インスタンス色
		GLint	a_vInstancingAddColor ;		// vec4

		// uniform
		// 変換行列（※上下反転を行う場合にはカメラ行列で行う）
		GLint	u_mat4PerspectiveView ;		// mat4 透視変換行列
		GLint	u_mat4CameraView ;			// mat4 カメラ変換行列（回転と移動）
		GLint	u_mat3CameraViewForNormal ;	// mat3 カメラ変換行列（回転のみ）
		GLint	u_mat4ICameraView ;			// mat4 カメラ変換行列（回転と移動）
		GLint	u_mat3ICameraViewForNormal ;// mat3 カメラ変換行列（回転のみ）
		GLint	u_mat4ModelView ;			// mat4 変換行列（回転と移動）（カメラを含む）
		GLint	u_mat3ModelViewForNormal ;	// mat3 変換行列（回転のみ）（カメラを含む）
		GLint	u_fpInverseNormal ;			// float 法線反転用
		GLint	u_fpBorderOffset ;			// float 輪郭描画用オフセット
		GLint	u_fpMorphApplication ;		// float モーフィング適用率

		// ボーン
		GLint	u_nBoneCount ;			// int ボーン数
		GLint	u_mat3BoneRotation ;	// mat3[MAX_BONE_PALETTE] ボーン行列
		GLint	u_vBoneTranslate ;		// vec3[MAX_BONE_PALETTE] ボーン移動

		// 頂点テクスチャ
		GLint	u_samplerVertex ;			// sampler2D 浮動小数点テクスチャ
		GLint	u_vVertexTextureScale ;		// vec2 テクスチャUV変換（サイズ逆数：サイズは２の累乗）
		GLint	u_bVertexMorphing ;			// bool モーフィング有効
		GLint	u_bVertexBoneInstance ;		// bool ボーン有効
		GLint	u_yVertexMorphingFirst ;	// float モーフィングバッファのｙ座標
		GLint	u_yNormalMorphingFirst ;	// float （※頂点・法線は差分値）
		GLint	u_yVertexMorphingStride ;	// float １つのモーフィング配列の行数
		GLint	u_yVertexMorphingInstance ;	// float モーフィング適用度配列のｙ座標
		GLint	u_xVertexMorphingInstanceStride ;	// float { 0.0 | u_vVertexTextureScale.y * 2 }
		GLint	u_yVertexBoneWeight ;		// float ボーンウェイト配列（４要素配列）
		GLint	u_yVertexBoneIndex ;		// float ボーンインデックス配列（４要素配列）
		GLint	u_yVertexBoneMatrix ;		// float ボーン行列配列（４×３×ボーン数）
		GLint	u_yVertexBoneMatrixStride ;	// float { 0.0 | u_vVertexTextureScale.y }
		GLint	u_yVertexExAttrElements ;	// float 拡張頂点属性のｙ座標 * u_vVertexTextureScale.y
		GLint	u_xVertexExAttrStride ;		// float 拡張頂点属性の１要素ピクセル数

		// 光源情報
		GLint	u_vLightAmbientColor ;		// vec3 環境光
		GLint	u_vLightAmbientColorMul ;	// vec3 環境光（乗算）
		GLint	u_countLight ;				// int 通常光源数
		GLint	u_typeLighting ;			// int[MAX_LIGHT_COUNT] 光源種別
		GLint	u_vLightColor ;				// vec3[MAX_LIGHT_COUNT] 光源色
		GLint	u_fpLightBrightness ;		// float[MAX_LIGHT_COUNT] 輝度
		GLint	u_fpLightAttenuationPower ;	// float[MAX_LIGHT_COUNT] 距離減衰力
		GLint	u_vLightPosition ;			// vec3[MAX_LIGHT_COUNT] 光源座標（グローバル）
		GLint	u_vLightDirection ;			// vec3[MAX_LIGHT_COUNT] 光源ベクトル（グローバル・正規化済み）
		GLint	u_fpLightAngle ;			// float[MAX_LIGHT_COUNT] 範囲角 cosθ
		GLint	u_fpLightGradation ;		// float[MAX_LIGHT_COUNT] ぼかし範囲 -cosθ

		// 疑似フォッグ
		GLint	u_bEnableFog ;			// bool
		GLint	u_rgbFogColor ;			// vec3
		GLint	u_zFogNear ;			// float
		GLint	u_zFogDistance ;		// float

		// シャドウマッピング
		GLint	u_iEnableShadowmap ;
		GLint	u_mat4PerspectiveShadowmap ;	// mat4[MAX_SHADOWMAPPING] 光源からの透視変換
		GLint	u_mat4ModelViewShadowmap ;		// mat4[MAX_SHADOWMAPPING] 変換行列
		GLint	u_samplerShadowmap ;			// sampler2DArray depth texture
		GLint	u_iShadowmapDepthLayer ;		// int[MAX_SHADOWMAPPING] 深度バッファレイヤー
		GLint	u_vShadowmapUnit ;				// vec2[MAX_SHADOWMAPPING] 1px 相当の UV 値
		GLint	u_fpShadowmapFixErrorGap ;		// float[MAX_SHADOWMAPPING] z 演算誤差固定比率（1/4096 等）
		GLint	u_fpShadowmapVarErrorGap ;		// float[MAX_SHADOWMAPPING] 角度に応じた z 演算誤差比率（1/512 等）

		// 色効果
		GLint	u_vEffectMulColor ;		// vec3 色効果（積）
		GLint	u_vEffectAddColor ;		// vec3 色効果（加）
		GLint	u_fpEffectAlpha ;		// float 不透明度効果

		// 表面属性
		GLint	u_bMaterialShading ;		// bool
		GLint	u_bMaterialToon ;			// bool
		GLint	u_bMaterialDoubleSide ;		// bool
		GLint	u_iMaterialTriming ;		// int
		GLint	u_bMaterialNoFogEffect ;	// bool
		GLint	u_bMaterialVertexAlpha ;	// bool
		GLint	u_vMaterialMulColor ;		// vec3 基本色 - 影色
		GLint	u_vMaterialAddColor ;		// vec3
		GLint	u_vMaterialMulShade ;		// vec3 影色
		GLint	u_vMaterialAddShade ;		// vec3
		GLint	u_vMaterialSpecularColor ;	// vec3 鏡面反射色
		GLint	u_fMaterialAmbient ;		// float 環境光強度
		GLint	u_fMaterialDiffusion ;		// float 拡散反射光強度
		GLint	u_fMaterialBackDiffusion ;	// float 裏拡散反射（疑似AO）
		GLint	u_fMaterialSpecular ;		// float 鏡面反射光強度
		GLint	u_fMaterialSpecularPow ;	// float 鏡面反射光の鋭さ
		GLint	u_fMaterialAlpha ;			// float 不透明度
		GLint	u_fMaterialDeepness ;		// float 透明深度係数
		GLint	u_fMaterialDeepnessPow ;	// float 透明深度指数
		GLint	u_fMaterialReflection ;		// float 反射率
		GLint	u_fMaterialEmission ;		// float 発光度
		GLint	u_vMaterialBackLightMul ;	// vec3 バックライト
		GLint	u_vMaterialBackLightAdd ;	// vec3
		GLint	u_cosShadeCoefficient ;		// float[2] シェード余弦係数
		GLint	u_fpToonShadeThreshold ;	// float トゥーンシェーダー影閾値
		GLint	u_fpToonShadeBrightness ;	// float トゥーンシェーダー影輝度
		GLint	u_fMaterialRimLight ;		// float リムライト輝度
		GLint	u_fMaterialRimDeepness ;	// float リムライト厚み
		GLint	u_vMaterialRimColor ;		// vec3 リムライト色

		// 通常テクスチャ
		GLint	u_bMaterialTexture ;		// bool テクスチャ有効
		GLint	u_samplerMaterialTexture ;	// sampler2D テクスチャ（拡散反射成分）
		GLint	u_vMaterialTextureScale ;	// vec2 テクスチャ座標拡大
		GLint	u_vMaterialTextureBase ;	// vec2 テクスチャ座標平行移動

		// 発光テクスチャ
		GLint	u_fpLuminousTexture ;		// float 発光テクスチャ適用度
		GLint	u_samplerLuminousTexture ;	// sampler2D 発光テクスチャ
		GLint	u_vLuminousTextureScale ;	// vec2 発光テクスチャ座標拡大
		GLint	u_vLuminousTextureBase ;	// vec2 発光テクスチャ座標平行移動

		// 環境マッピング
		GLint	u_typeEnvironmentMapping ;		// int 環境マッピング
		GLint	u_typeEnvironmentRefraction ;	// int 環境マッピング（屈折反映）
		GLint	u_fpRefractionRatio ;			// float 屈折率
		GLint	u_fpRefractionParam ;			// float 屈折適用度
		GLint	u_samplerEnvironmentCube ;		// samplerCube テクスチャ（環境マッピング）
		GLint	u_samplerEnvironmentMapping ;	// sampler2D テクスチャ（環境マッピング）
		GLint	u_samplerViewportMapping ;		// sampler2D テクスチャ（環境マッピング）
		GLint	u_samplerViewportDepth ;		// sampler2D テクスチャ（環境マッピング）
		GLint	u_vViewportUnit ;				// vec2 1px 相当の UV 値（環境マッピング）
		GLint	u_mat3EnvironmentMapping ;		// mat3 変換行列（回転のみ）
		GLint	u_vEnvMapingTextureScale ;		// vec2 テクスチャUV変換行列（拡大）
		GLint	u_vEnvMapingTextureBase ;		// vec2 テクスチャUV変換行列（平行移動）

		// 法線テクスチャ
		GLint	u_fpNormalTexture ;				// float 法線テクスチャ適用度
		GLint	u_samplerNormalTexture ;		// sampler2D テクスチャ（バンプマッピング：r,g に x, y 差分値 +0.5 を格納）
		GLint	u_vNormalTextureScale ;			// vec2 テクスチャUV変換行列（拡大）
		GLint	u_vNormalTextureBase ;			// vec2 テクスチャUV変換行列（平行移動）

		// 標高テクスチャ
		GLint	u_fpBumpHeight ;				// float 標高テクスチャ高さ
		GLint	u_samplerHeightTexture ;		// sampler2D テクスチャ（標高テクスチャ）
		GLint	u_vHeightTextureScale ;			// vec2 テクスチャUV変換行列（拡大）
		GLint	u_vHeightTextureBase ;			// vec2 テクスチャUV変換行列（平行移動）

		// αマッピング
		GLint	u_bMaterialAlphaTexture ;		// bool αテクスチャ有効
		GLint	u_fpAlphaCoefficient ;			// float α係数
		GLint	u_fpAlphaBase ;					// float α基準値
		GLint	u_samplerAlphaMapping ;			// sampler2D テクスチャ（αマッピング）
		GLint	u_vAlphaMapingTextureScale ;	// vec2 テクスチャUV変換行列（拡大）
		GLint	u_vAlphaMapingTextureBase ;		// vec2 テクスチャUV変換行列（平行移動）

		// スペキュラー・粗さ・反射率テクスチャ
		GLint	u_bMaterialSpecularTexture ;	// bool 反射率テクスチャ有効
		GLint	u_samplerSpecularTexture ;		// sampler2D テクスチャ
		GLint	u_vSpecularTextureScale ;		// vec2 テクスチャUV変換行列（拡大）
		GLint	u_vSpecularTextureBase ;		// vec2 テクスチャUV変換行列（平行移動）

		// 大域ライトマップAOテクスチャ
		GLint	u_bMaterialGlobalAOTexture ;	// bool 反射率テクスチャ有効
		GLint	u_samplerGlobalAOTexture ;		// sampler2D テクスチャ

	protected:
		// 最後に設定されたマテリアル情報
		// m_psaLastMaterial が一致する場合には
		// マテリアル情報の Uniform への設定を省略できる
		S3DSurfaceAttribute *	m_psaLastMaterial ;
		uint64_t				m_flagsLastMaterialShading ;

		// 有効化されている属性ポインタ
		bool		m_enabledVertexAttr ;
		bool		m_enabledVertex4Morphing ;
		bool		m_enabledVertexBoneWeight[BONE_PALETTE_ATTRS] ;
		bool		m_enabledInstancingAttr ;

		// インスタンシング対応シェーダー用ダミー
		GLfloat		m_mat4DummyInstancing[4][4] ;	// 単位行列
		S3DColor	m_colorDummyInstancing ;

		// 最後に設定された Uniform 値
		S4DMatrix	m_mat4ModelView ;
		float32_t	m_fpInverseNormal ;
		GLfloat		m_fpMorphApplication ;
		GLint		m_nBoneCount ;

		GLint		m_bMaterialShading ;
		GLint		m_bMaterialToon ;
		GLint		m_iMaterialTriming ;
		GLint		m_bMaterialNoFogEffect ;
		GLint		m_bMaterialVertexAlpha ;
		GLint		m_bMaterialDoubleSide ;
		S3DSurfaceAttribute
					m_saLastMaterial ;

		struct	TextureInfo
		{
			int										iTextureNum ;
			GLenum									glTxTarget ;
			SGLImageObject *						pImage ;
			SGLOpenGLTextureBuffer::GLResource *	pglTexture ;
			SGLImageRect							rectRef ;
			SGLPoint								ptBaseRef ;
		} ;
		TextureInfo	m_txiMaterialTexture ;
		TextureInfo	m_txiMaterialLuminous ;
		TextureInfo	m_txiMaterialEnvironment ;
		TextureInfo	m_txiMaterialRefraction ;
		TextureInfo	m_txiMaterialViewportDepth ;
		TextureInfo	m_txiMaterialNormal ;
		TextureInfo	m_txiMaterialHeight ;
		TextureInfo	m_txiMaterialAlpha ;
		TextureInfo	m_txiMaterialSpecular ;
		TextureInfo	m_txiMaterialLightMapAO ;

	public:
		// 使用テクスチャの割り当て
		enum	OpenGLTextureAllocation
		{
			glTextureDefault	= 0,
			glTextureShadow1	= 1,
			glTextureMaterial0	= 2,
			glTextureMaterial1	= 3,
			glTextureShadow2	= 4,
			glTextureMaterial2	= 5,
			glTextureMaterial3	= 6,
			glTextureMaterial4	= 7,
			glTextureMaterial5,
			glTextureMaterial6,
			glTextureMaterial7,
			glTextureMaterial8,
			glTextureMaterial9,
			glTextureMaterial10,
			glTextureMaterial11,
			glTextureMaterial12,
			glTextureMaterial13,
			glTextureMaterial14,
			glTextureMaterial15,
		} ;
		static const int	m_gl_iMaterialTextures[32] ;

	protected:
		GLint		m_iEnableShadowmap ;
		GLint		m_iEnableShadowmap2 ;

		GLfloat		m_fpLuminousTexture ;
		GLfloat		m_fpNormalTexture ;
		GLfloat		m_fpHeightTexture ;
		S3DMatrix	m_matEnvironmentMapping ;
		GLfloat		m_fpRefractionRatio ;
		GLfloat		m_fpRefractionParam ;
		GLfloat		m_fpAlphaCoefficient ;
		GLfloat		m_fpAlphaBase ;

		S3DColor	m_colorUniformEffect ;
		uint32_t	m_nUniformTransparency ;

		// コンテキスト設定
		bool		m_fEnableTextureSmoothing ;
		bool		m_fDisableWriteDepth ;
		bool		m_fShaderForceToon ;
		bool		m_fShaderForceBorder ;
		bool		m_fShaderNoDrawBorder ;
		bool		m_fShaderSurfaceOffset ;
		bool		m_fShaderEmisiveTarget ;
		SGLPalette	m_rgbBorderColor ;
		float32_t	m_fpBorderCoefficient[2] ;

		S3DRenderBufferInterface::FaceCullingOperation	m_faceCulling ;
		S3DRenderBufferInterface::DepthMaskOperation	m_depthMask ;
		S3DRenderBufferInterface::BlendOperation		m_blendOperation ;

		// 光源情報用バッファ
		struct	LIGHT_BUFFER
		{
			GLfloat	vAmbientColor[3] ;
			GLfloat	vAmbientMul[3] ;
			GLint	countLight ;
			GLint	typeLighting[MAX_ALL_LIGHT_COUNT] ;
			GLfloat	vColor[MAX_ALL_LIGHT_COUNT][3] ;
			GLfloat	fpBrightness[MAX_ALL_LIGHT_COUNT] ;
			GLfloat	fpAttenuationPower[MAX_ALL_LIGHT_COUNT] ;
			GLfloat	vPosition[MAX_ALL_LIGHT_COUNT][3] ;
			GLfloat	vDirection[MAX_ALL_LIGHT_COUNT][3] ;
			GLfloat	fpAngle[MAX_ALL_LIGHT_COUNT] ;
			GLfloat	fpGradation[MAX_ALL_LIGHT_COUNT] ;

			// S3DLightEntry 変換
			void SetAt( size_t i, const S3DLightEntry& light ) ;
		} ;
		LIGHT_BUFFER			m_bufLight ;
		SSystem::SArray<int>	m_tableLightID ;

		// シャドウマップ用バッファ
		struct	SHADOWMAP_BUFFER
		{
			GLint		iLights[MAX_SHADOWMAPPING] ;
			GLint		iSamplers[MAX_SHADOWMAPPING] ;
			GLint		iLayers[MAX_SHADOWMAPPING] ;
			S4DMatrix	mat4Perspectiv[MAX_SHADOWMAPPING] ;
			S4DMatrix	mat4ModelView[MAX_SHADOWMAPPING] ;
			GLfloat		vMapUnit[MAX_SHADOWMAPPING][2] ;
			GLfloat		fpFixErrorGap[MAX_SHADOWMAPPING] ;
			GLfloat		fpVarErrorGap[MAX_SHADOWMAPPING] ;
		} ;
		SHADOWMAP_BUFFER		m_bufShadowmap ;
		bool					m_flagEnabledShadowmap ;

		// 頂点情報用バッファ
		S3DTemporaryNormalBuffer	m_tnbNormals ;
		SSystem::SArray<S3DColor>	m_bufColors ;	// ダミーバッファ
		SSystem::SArray<uint16_t>	m_bufIndex ;	// 指標バッファ

		// 頂点テクスチャ
		SGLImageObject *			m_pVertexTexture ;
		int							m_iVertexTexture ;

		SSystem::SArray<S3DVector4>	m_aVTMorphInstance ;
		SSystem::SArray<S3DMatrix>	m_aVTBoneMatrixBuf ;
		SSystem::SArray<S3DVector>	m_aVTBoneTransBuf ;

		// 環境マッピング
		SGLImageObject *	m_pEnvMapping ;
		int					m_nEnvMappingType ;
		S3DMatrix			m_matEnvMapping ;	// 逆変換行列
		SGLImageObject *	m_pEnvViewportDepth ;
		SGLImageObject *	m_pEnvRefraction ;
		int					m_nEnvRefractionType ;
		float32_t			m_fpEnvRefractionDeepness ;

		// 投影スクリーン設定値
		S4DMatrix			m_mat4Perspective ;
		S3DVector			m_vProjectScreen ;

		// カメラ設定値
		S4DMatrix			m_mat4Camera ;

	public:
		// ボーンパレット使用制限数（シェーダー作成時最大数制限）
		static size_t	m_limit_bone_count ;

		// 光源使用制限数（シェーダー作成時最大数制限）
		static size_t	m_limit_light_count ;

		// シャドウマッピング制限数
		static size_t	m_limit_shadow_map_count ;

		// 環境マッピング使用不可
		static bool		m_disable_environment_mapping ;
		static bool		m_disable_environment_cubemapping ;
		static bool		m_disable_environment_spheremapping ;
		static bool		m_disable_environment_viewport ;
		static bool		m_disable_environment_refraction ;

		// 法線マッピング使用不可
		static bool		m_disable_normal_mapping ;

		// 標高テクスチャ使用不可
		static bool		m_disable_height_mapping ;

		// スペキュラ・粗さ・反射率テクスチャ使用不可
		static bool		m_disable_specular_mapping ;

		// 大域ライトマップAOテクスチャ使用不可
		static bool		m_disable_global_ao_lightmap ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLDefaultShader, SGLOpenGLShaderProgram )
		// 構築関数
		SGLOpenGLDefaultShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLDefaultShader( void ) ;

	public:
		enum	ShadingFlag
		{
			shadingGouraud	= 0x0001,
			shadingPhong	= 0x0002,
		} ;
		// 初期化
		SGLError InitializeProgram
			( int typeShading,
				SGLOpenGLShaderProgram::CompileListener * pListener ) ;
		// ボーン数制限用ヘッダ追加と m_maxBonePalette 設定
		void SetBoneLimitForShaderHeader
			( SSystem::SString& strHeader, size_t nBoneLimit ) ;
		// GLSL attribute 最大数からヘッダを調整する
		void SetAttributeLimitForShaderHeader( SSystem::SString& strHeader ) ;

	public:
		// シェーダープログラム保存
		virtual SGLError SaveProgramBinary( S3DShaderBinary& bin ) ;
		// シェーダープログラム復元
		virtual SGLError LoadProgramBinary
			( const S3DShaderBinary& bin,
				SGLOpenGLShaderProgram::CompileListener * pListener = NULL ) ;
		// シェーダー属性／ユニフォーム位置取得
		void GetShaderVariableLocations( void ) ;
		// プログラムを現在のコンテキストに設定しuniformを初期化
		SGLError InitializeShader( void ) ;

	public:
		// デフォルトのシェーダー設定を取得する
		static void GetDefaultShaderFeatures
			( S3DRenderDevice::ShaderSourceInfo& srcinf ) ;

	public:
		// テクスチャ補完有効化
		void EnableTextureSmoothing( bool fEnable ) ;
		// テクスチャ補完は有効か？
		bool IsEnabledTextureSmoothing( void ) const ;
		// Zバッファへの書き込み有効
		void EnableWriteDepth( bool fEnable ) ;
		// Zバッファへの書き込み有効か？
		bool IsEnabledWriteDepth( void ) const ;
		// トゥーンシェーダーを強制的に有効にする
		void ForceToonShader( bool fToon ) ;
		// トゥーンシェーダーが強制されているか？
		bool IsForcedToonShader( void) const ;
		// 表面オフセット描画を有効にする
		void EnableMeshSurfaceOffset( bool fOffset ) ;
		// 表面オフセット描画が有効に設定されているか？
		bool IsEnabledMeshSurfaceOffset( void ) const ;
		// 輪郭表示（オフセット描画）を強制的に有効にする
		void ForceBorderShader( bool fBorder ) ;
		// 輪郭表示（オフセット描画）が強制されているか？
		bool IsForcedBorderShader( void ) const ;
		// 輪郭表示（オフセット描画）を強制的に無効にする
		void ForceNoBorderShader( bool fNoBorder ) ;
		// 輪郭表示（オフセット描画）が無効にされているか？
		bool IsForcedNoBorderShader( void ) const ;
		// 輪郭描画の色を設定する
		void SetOffsetBorderColor( uint32_t rgb ) ;
		// 輪郭描画の色を取得する
		const SGLPalette& GetOffsetBorderColor( void ) const ;
		// 輪郭描画の太さ係数を設定する
		void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// カリング処理の設定
		void SetFaceCullingOperation
			( S3DRenderBufferInterface::FaceCullingOperation faceCulling ) ;
		// ｚバッファ処理の設定
		void SetDepthMaskOperation
			( S3DRenderBufferInterface::DepthMaskOperation depthMask ) ;
		// 描画処理の設定
		void SetBlendOperation
			( S3DRenderBufferInterface::BlendOperation blendOp ) ;
		// 拡散反射光を発光出力する（マルチレンダーターゲット出力用）
		void SetEmisiveTarget( bool fEmisiveTarget ) ;
		// 拡散反射光を発光出力するか？
		bool IsEmisiveTarget( void ) const ;

	public:
		// シェーダープログラムが設定された（変更された）
		virtual void OnChangedProgram( void ) ;
		// シェーダープログラムが別のプログラムに変更される
		virtual void OnChangingProgram( void ) ;
		// ユニフォーム値更新
		virtual void UpdateCustomUniform( bool flagForceUpdate = false ) ;
		// ユーザーテクスチャ番号 → OpenGL バインドテクスチャ番号
		virtual int GLTextureNumAtUserTexture( size_t iUserTexture ) const ;
		// ユニフォーム指標定義
		virtual void RegisterCustomUniform
			( const wchar_t * pszUniformID,
				S3DCustomShader::UniformType type, size_t nCount ) ;
		// Flush 処理
		virtual void OnFlushContext( void ) ;
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix( const S4DMatrix& mat4 ) ;
		// 投影スクリーン座標設定
		virtual void SetProjectionScreen
			( float32_t xScreen, float32_t yScreen, float32_t zScreen ) ;
		// カメラ変換行列設定
		virtual void SetCameraViewMatrix( const S4DMatrix& mat4 ) ;
		// モデル変換行列設定
		virtual void SetModelViewMatrix
			( const S4DMatrix& mat4, bool fInverseNormal = false ) ;

	public:
		// 光源を設定
		void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		void SetShadowMaps
			( size_t nShadowCount,
				const S4DMatrix * pmat4ICameras,
				const uint32_t * pidLights,
				const S3DShadowMapInfo * pinfShadowMaps,
				SGLImageObject *const* ppShadowMapDepth,
				SGLImageObject *const* ppShadowMapColor = nullptr ) ;
		// 疑似フォッグを設定
		void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		void EnableFog( bool fFog ) ;
		// 環境マッピング設定
		void SetEnvironmentMapping
			( SGLImageObject * pImage,
					uint32_t nFlags, S3DMatrix& matMapping ) ;
		void SetRefractionMapping
			( SGLImageObject * pImage, uint32_t nFlags, float32_t nDeepness ) ;
		void SetEnvironmentMappingViewportDepth( SGLImageObject * pDepth ) ;
		// 頂点テクスチャ設定
		void SetVertexTexture
			( const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
					SGLImageObject * pImage, bool flagMultiShapeInstance ) ;
		void UnsetVertexTexture( void ) ;

	public:
		// 色効果を設定
		void SetColorEffect
			( const S3DColor * pColor = NULL, unsigned int nTransparency = 0 ) ;
		// 表面属性を設定
		void SetMaterial
			( S3DMaterial * pMaterial,
				bool fBackFace = false,
				uint64_t flagRemove = 0, uint64_t flagAdd = 0 ) ;
		// 輪郭描画用パラメータを設定する
		void SetBorderOffset
			( bool fBorder, float32_t zTarget,
					const S3DColor& colorEffect,
					const S3DSurfaceAttribute& saMaterial ) ;
		// テクスチャ（拡散反射光成分）設定
		SGLOpenGLTextureBuffer::GLResource * BindTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect, uint64_t flagsShading ) ;
		// テクスチャ（発光成分）設定
		SGLOpenGLTextureBuffer::GLResource * BindLuminousTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect,
				uint64_t flagsShading, float32_t fpLuminousApply ) ;
		// 環境マッピング・テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindEnvironmentTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect,
				int typeEnvMapping, bool fSmoothing,
				const S3DMatrix * pmat3EnvMap ) ;
		SGLOpenGLTextureBuffer::GLResource * BindRefractionTexture
			( int iTextureNum, SGLImageObject * pRefraction,
				const SGLImageRect * pImageRect,
				int typeRefraction, bool fSmoothing,
				const S3DMatrix * pmat3EnvMap, float32_t fpRefraction ) ;
		SGLOpenGLTextureBuffer::GLResource * BindViewportDepthTexture
			( int iTextureNum, SGLImageObject * pDepth,
				const SGLImageRect * pImageRect, bool fSmoothing ) ;
		// 法線マッピング・テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindNormalTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect,
				uint64_t flagsShading, float32_t fpNormalApply ) ;
		// 標高マッピング・テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindHeightTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect,
				uint64_t flagsShading, float32_t fpOffsetHeight ) ;
		// αマッピング・テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindAlphaTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect,
				uint64_t flagsShading,
				float32_t fpAlphaCoefficient, float32_t fpAlphaBase ) ;
		// スペキュラ・粗さ・反射率テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindSpecularTexture
			( int iTextureNum, SGLImageObject * pTextureImage,
				const SGLImageRect * pImageRect, uint64_t flagsShading ) ;
		// 大域ライトマップAOテクスチャ設定
		SGLOpenGLTextureBuffer::GLResource * BindGlobalAOTexture
			( int iTextureNum, SGLImageObject * pTextureImage, uint64_t flagsShading ) ;
		// テクスチャ割り当て解除
		bool UnbindGLTexture( TextureInfo& txinf, int iAllocated ) const ;
		// テクスチャスムーシング・クリッピング設定
		void SetGLTextureParameter
			( bool fSmoothing, bool fMipmap,
				bool fTiling, GLenum glTxTarget = GL_TEXTURE_2D ) const ;

	public:
		// プリミティブリストをレンダリング
		SGLError AddIndexedPrimitiveList
			( uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		SGLError AddVertexBuffer
			( const S4DDMatrix& matBase,
				const S3DColor& colorBase,
				unsigned int nTransparency,
				uint32_t nFlags, S3DRenderBuffer * pRBuffer,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL,
				S3DVertexVariantBuffer*const* ppInstancingVVB = NULL ) ;
	protected:
		// モーフィング・ボーン適用・頂点セットアップ処理
		struct	VertexSetupContext
		{
			bool				flagVAO ;
			bool				flagVertexTexture ;
			bool				flagMultiShapeInstance ;
			SGLOpenGLVertexBuffer::GLResource *
								pRsrc ;
			SGLOpenGLVertexBuffer::ENTRY_INFO *
								pei ;
			S3DRenderBuffer::RENDER_ENTRY *
								pre ;
			SGLImageObject *	pVTImage ;
			size_t				iMesh ;
			size_t				nInstancing ;
			const S4DMatrix *	pmatInstancing ;
			const S3DColor *	pColorInstancing ;
			S3DVertexVariantBuffer*const*
								ppInstancingVVB ;
		} ;
		void UpdateShapeMatrix
			( VertexSetupContext& vsc,
				SGLOpenGLContext * pOpenGL,
				S3DRenderBuffer * pBuffer,
				SGLOpenGLVertexBuffer * pglBuffer,
				size_t iMesh, size_t nVBOMeshCount,
				SGLOpenGLVertexBuffer::GLResource * pRsrc,
				SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
				S3DRenderBuffer::RENDER_ENTRY * pre,
				size_t nInstancing,
				const S4DMatrix * pmatInstancing,
				const S3DColor * pColorInstancing,
				S3DVertexVariantBuffer*const* ppInstancingVVB ) ;
		// モーフィング・ボーン適用・頂点セットアップ処理後始末
		void FinishShapeMatrix( VertexSetupContext& vsc ) ;
		// モーフィングインスタンスを頂点テクスチャに書き込む
		void WriteMorphInstanceToVertexTexture
			( SGLImageObject * pImage, size_t iMesh,
				const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
				S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount ) ;
		// ボーンインスタンスを頂点テクスチャに書き込む
		void WriteBoneInstanceToVertexTexture
			( SGLImageObject * pImage, size_t iMesh,
				const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
				S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount ) ;
		// 頂点バッファにオリジナル値を書き戻す
		void RestoreDynamicVertexBuffer
			( SGLOpenGLVertexBuffer::GLResource * pRsrc,
				size_t ofsVertex, const S3DRenderBuffer::RENDER_ENTRY * pre ) ;
		// 一時的な頂点バッファを書き込む（CPUでボーンやモーフィング処理時）
		void WriteDynamicVertexBuffer
			( SGLOpenGLVertexBuffer::GLResource * pRsrc,
				size_t ofsVertex, const S3DRenderBuffer::RENDER_ENTRY * pre ) ;
		// サブメッシュを選択
		int SelectSubMesh
			( const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
				const S3DRenderBuffer::RENDER_ENTRY * pre,
				size_t& nDrawIndexCount,
				size_t& ofsElement, const S4DVector& vMeshPos ) const ;
		// 頂点バッファ設定
		void BindVertexBuffer
			( SGLOpenGLVertexBuffer::GLResource * pRsrc,
				const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
				const S3DRenderBuffer::RENDER_ENTRY * pre,
				ssize_t iBaseMesh, ssize_t iMorphMesh, size_t countBone ) ;
		// 頂点バッファ有効化
		void EnableVertexBuffer
			( const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
				const S3DRenderBuffer::RENDER_ENTRY * pre,
				ssize_t iBaseMesh, ssize_t iMorphMesh, size_t countBone ) ;
		// インスタンシングバッファ書き込み
		void WriteInstancingBuffer
			( SGLOpenGLVertexBuffer::GLResource * pRsrc,
				size_t nInstancing,
				const S4DMatrix * pmatInstancing,
				const S3DColor * pColorInstancing ) ;
		// インスタンシングバッファ設定
		void BindInstancingBuffer
			( SGLOpenGLVertexBuffer::GLResource * pRsrc ) ;
		// インスタンシングバッファ有効化
		void EnableInstancingBuffer( void ) ;
		// ボーン回転行列 Uniform 設定
		void SetBoneMatrixUniform
			( const S3DRenderBuffer::RENDER_ENTRY * pre ) ;
		// 描画コマンド
		void DrawVertexBuffer
			( VertexSetupContext& vsc,
				SGLOpenGLContext * pOpenGL,
				GLenum modeDraw, size_t nIndexCount, size_t ofsElement ) ;

	public:
		// 頂点バッファ設定
		void SetVertexPointer( const S3DVector4 * pvVertex ) ;
		// 法線バッファ設定
		void SetNormalPointer( const S3DVector4 * pvNormal ) ;
		// UV 座標バッファ設定
		void SetTexCoordPointer( const S2DVector * pvUVMap ) ;
		// 頂点色バッファ設定
		void SetColorPointer( const S3DColor * pColor ) ;
		// 全頂点ポインタを無効化
		void DisableAllVertexPointer( void ) ;
		// 頂点色用ダミーバッファ（透明）を確保
		S3DColor * AllocateDummyVertexColorBuffer( size_t countVertex ) ;
		// 指標バッファを16ビットへ変換
		uint16_t * ElementIndexToUint16
			( const uint32_t * pIndexedList, size_t countIndexes ) ;

	} ;

	//////////////////////////////////////////////////////////////////////////
	// OpenGL デバイス
	//////////////////////////////////////////////////////////////////////////

	class	S3DOpenGLBufferedRenderer ;
	class	SGLOpenGLContext
				: public S3DRenderDevice, public SSystem::SProcedure
	{
	public:
		enum	DefaultShaderIndex
		{
			indexNonShading,
			indexGouraudShading,
			indexPhongShading,
			countDefaultShader,
		} ;
	protected:
		SGLOpenGLView *				m_pglView ;			// 表示領域
		SGLOpenGLFrameBuffer *		m_pglFrameBuffer ;
		SGLOpenGLShaderProgram *	m_pglCurShader ;
		SGLOpenGLShaderProgram *	m_pDefShader[countDefaultShader] ;	// デフォルトシェーダー

		S3DRenderContextInterface *	m_pCurRenderer ;	// 現在のレンダラ

		SSystem::SThread			m_threadSuitable ;
		SSystem::SProcedureQueue	m_queRenderProc ;
		SSystem::SSignalEvent		m_signalSuitableProc ;
		SSystem::SCriticalSection	m_csSuitableRendering ;
		volatile S3DRenderContextInterface::PROCEDURE_RENDERING
									m_pfnSuitableRendering ;
		void *						m_pSuitableRenderInstance ;
		volatile bool				m_flagReadySuitableThread ;
		volatile bool				m_flagQuitSuitableThread ;
		volatile bool				m_flagInSuitableProcedure ;
		volatile bool				m_flagDoneSuitableProc ;

	public:
		// OpenGL バージョン / 対応機能
		SSystem::SString	m_strVender ;
		SSystem::SString	m_strRenderer ;
		SSystem::SString	m_strVersion ;

		uint32_t	m_versionGL[2] ;			// OpenGL バージョン
		bool		m_flagTextureNonPowerOf2 ;	// テクスチャに非２の累乗サイズが可能
		bool		m_flagElementIndexUint ;	// glDrawElement 指標に UINT が可能
		bool		m_flagDepthTexture ;		// 深度テクスチャを使用可能
		bool		m_flagMultisampling ;		// マルチサンプリングが有効
		bool		m_flagCubemapTexture ;		// Cubemap テクスチャを使用可能
		bool		m_flagCompressionS3TC ;		// EXT_texture_compression_s3tc
		bool		m_flagSupportedStereo3D ;	// Quad Buffer が有効
		bool		m_flagProgramBinary ;		// GLSL バイナリが有効
		bool		m_flagProgramBinaryOES ;
		bool		m_flagAnisotropicExt ;		// 異方性フィルタ有効
		bool		m_flagSupportedMRT ;		// マルチレンダーターゲットが有効
		bool		m_flagSupportedGeometry ;	// ジオメトリシェーダーが有効
		bool		m_flagSupportedVAO ;		// Vertex Array Object が有効
		bool		m_flagTextureFloat ;		// GL_ARB_texture_float
		bool		m_flagColorBufferFloat ;	// GL_ARB_color_buffer_float
		bool		m_flagAvailableMultiSVB ;	// multi shape vertex シェーダー可能
		bool		m_flagSupportedComputeShader ;// Compute Shader 可能
		GLint		m_maxMultiTextureUnits ;	// マルチテクスチャ最大数
		GLint		m_maxTextureImages ;		// テクスチャ対応数
		GLint		m_maxVSTextureImages ;		// 頂点テクスチャフェッチ対応数
		GLint		m_maxCombinedTextureImages ;
		GLint		m_maxTextureSize ;			// テクスチャ最大サイズ
		GLint		m_max3DTextureSize ;
		GLint		m_maxCubemapTextureSize ;
		GLint		m_maxVertexAttributes ;		// GLSL 最大 attribute 数
		GLint		m_maxVertexVaryings ;		// GLSL 最大 varying 数
		GLint		m_maxVertexUniforms ;		// GLSL 最大 uniform 数
		GLint		m_maxFragmentUniforms ;
		GLint		m_maxDrawBuffers ;			// MRT 最大数
		GLint		m_maxColorAttachments ;
		GLfloat		m_maxAnisotropic ;			// 異方性フィルタ最大値
		GLint		m_maxImageUnits ;			// 最大画像ユニット数

		// 使用中リソース
		size_t		m_bytesUsedTexture ;
		size_t		m_bytesMaxUsedTexture ;
		size_t		m_bytesUsedVBO ;
		size_t		m_bytesMaxUsedVBO ;

		// パフォーマンスログ
		struct	PerformanceLog	: public PerformanceFrameLog
		{
		/*
			size_t		countDrawCall ;
			size_t		countDrawInstance ;
			size_t		countTransmitVertex ;
			size_t		countComputeShapeByCPU ;
			size_t		countSwitchRenderer ;
			size_t		countSwitchShader ;
			size_t		countSwitchMaterial ;
			double		msecShapeByCPU ;
		*/
			void Reset( void ) ;
			void Add( const PerformanceLog& pl ) ;
			void Max( const PerformanceLog& pl ) ;
			void ToFrameLog( PerformanceFrameLog& pfl ) const ;
		} ;
		PerformanceLog			m_pflog ;
		PerformanceLog			m_pflogMax ;
		PerformanceLog			m_pflogTotal ;
		size_t					m_nTotalLogFrameCount ;
		SSystem::STimeCounter	m_timerFrame ;
		SSystem::STimeCounter	m_timerLog ;

		// テクスチャ転送用一時バッファ
		SSystem::SArray<uint8_t>	m_bufTempTexture ;

		// 標準シェーダーにグーローシェーディングを使用しない
		static ESL_DLL_EXPORT bool	m_disable_create_std_gouraud_shader ;

		// 標準シェーダーにフォンシェーディングを使用しない
		static ESL_DLL_EXPORT bool	m_disable_create_std_phong_shader ;

	protected:
		// OpenGL 拡張の初期化カウンタ
		static atomic_int_t	m_countGLEXInit ;

		// OpenGL 拡張機能文字列
		SSystem::SObjectArray<SSystem::SString>	m_extension_supported ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SGLOpenGLContext, S3DRenderDevice, SProcedure )
		// 構築関数
		SGLOpenGLContext( void ) ;
		// 消滅関数
		virtual ~SGLOpenGLContext( void ) ;
		// 初期処理
		virtual void OnCreateGLContext( void ) ;
		// 破棄前処理
		virtual void OnDestroyGLContext( void ) ;
		// プログラムバイナリ保存
		SGLError SaveProgramBinary( void ) ;
		// 標準シェーダーのコンパイル
		bool CompileDefaultShader( DefaultShaderIndex dsIndex ) ;
		// カスタムシェーダー・オブジェクト生成
		virtual S3DCustomShader * NewCustomShader( ShaderProgramType type ) ;
		// 定義済み標準シェーダー生成／取得
		virtual S3DCustomShader *
				GetDefaultShaderProgramAs( const wchar_t * pwszID ) ;
		// カスタムシェーダーコンパイル
		virtual SGLError CompileCustomShader
				( S3DCustomShader * pShader,
					const S3DRenderDevice::ShaderSourceInfo& src,
					S3DCustomShader::CompileListener * pListener = NULL ) ;
		// カスタムシェーダー読み込み
		virtual SGLError LoadCustomShader
				( S3DCustomShader * pShader,
					const S3DShaderBinary& bin,
					S3DCustomShader::CompileListener * pListener = NULL ) ;
		// カスタムシェーダー・バイナリの保存
		virtual SGLError SaveCustomShaderBinary
			( S3DShaderBinary& bin, S3DCustomShader * pShader ) ;
		// OpenGL 拡張サポートテスト
		bool IsExtensionSupported( const wchar_t * pwszExtension ) ;
		// OpenGL Quad Buffer サポートテスト
		bool IsSupportedStereo( void ) const ;
		// OpenGL バージョンから使用可能な GLSL バージョン番号取得
		int GetMaxGLSLVersion( void ) const ;

	public:
		// OpenGL 状態変数
		enum	BlendMode
		{
			blendAdd,			// glBlendFunc( GL_ONE, GL_ONE )
			blendCopy,			// glBlendFunc( GL_ONE, GL_ZERO )
			blendProducted,		// glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA )
			blendUnproducted,	// glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA )
			blendMulColor,		// glBlendFunc( GL_ZERO, GL_SRC_COLOR )
			blendDstMasked,		// glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA )
			blendModeCount,
		} ;
		static const GLenum	s_blendModeParam[blendModeCount][2] ;

		BlendMode	m_modeBlend ;
		bool		m_alphaTestZeroClip ;
		bool		m_faceDoubleSide ;
		bool		m_faceCullBack ;
		bool		m_enabledMultiSample ;
		uint32_t	m_funcDepthTest ;

		struct	BindTextureContext
		{
			SGLOpenGLTextureBuffer::GLResource *	pglBind ;
			GLenum									glTarget ;
		} ;
		enum	ContextConstantValue
		{
			countBindTxtureNum	= 32,
		} ;
		BindTextureContext	m_glBinTexture[countBindTxtureNum] ;
		size_t				m_iTempTexture ;

		// バインドされている VBO
		SGLOpenGLVertexBuffer *	m_pBindingVBO ;
		SGLOpenGLVertexBuffer::GLResource *
								m_pRsrcOfVBO ;
		GLenum					m_typeVBOIndex ;
		size_t					m_iMeshOfVBO ;
		ssize_t					m_iMeshMorph0OfVBO ;
		ssize_t					m_iMeshMorph1OfVBO ;

		// OpenGL 描画ステータスを初期設定
		void InitMaterialSetting( void ) ;

	public:
		// SGLOpenGLView の関連付け
		virtual void AttachGLView( SGLOpenGLView * pView ) ;
		// 関連付けられている SGLOpenGLView 取得
		SGLOpenGLView * GetGLView( void ) const
		{
			return	m_pglView ;
		}
		// フレームバッファの関連付け
		virtual void AttachFrameBuffer
				( SGLOpenGLFrameBuffer * pglFrameBuffer ) ;
		// 関連付けられているフレームバッファを取得する
		SGLOpenGLFrameBuffer * GetCurrentFrameBuffer( void ) const
		{
			return	m_pglFrameBuffer ;
		}
		// 現在のシェーダーを関連付ける
		virtual void AttachShaderProgram
						( SGLOpenGLShaderProgram * pglShader ) ;
		// 現在のシェーダーを取得する
		SGLOpenGLShaderProgram * GetCurrentShaderProgram( void ) const
		{
			return	m_pglCurShader ;
		}
		// デフォルトシェーダー取得
		SGLOpenGLShaderProgram *
				GetDefaultShaderProgram( int nShaderType ) const ;
		SGLOpenGLShaderProgram *
				GetStandardShaderProgram( int nShaderType ) const ;

	public:
		// 現在のレンダラを関連付ける
		virtual void AttachRenderContext( S3DRenderContextInterface * pRender ) ;
		// 関連付けられているレンダラを取得する
		S3DRenderContextInterface * GetCurrentRenderContext( void ) const
		{
			return	m_pCurRenderer ;
		}
		// 現在のスレッドで且つ関連付けられたレンダラか？
		bool IsCurrentRenderContext( const S3DRenderContextInterface * pRender )
		{
			return	(m_pCurRenderer == pRender) && IsOnRenderThread() ;
		}

	public:
		// バインドテクスチャ情報を設定する
		void SetBindTextureInfo
			( int iTexture,
				SGLOpenGLTextureBuffer::GLResource * pglTexture,
				GLenum glTarget = GL_TEXTURE_2D ) ;
		// バインドテクスチャ情報を取得する
		bool IsBindingTexture( int iTexture, GLenum& glTarget ) const ;
		// ブレンドモード設定
		void SetBlendMode( BlendMode modeBlend ) ;
		// ブレンドモード変換
		static BlendMode BlendModeFromBlendOp
			( S3DRenderBufferInterface::BlendOperation blendOp, BlendMode modeDefault ) ;

	public:
		// レンダリングスレッドか判定（※要オーバーライド）
		virtual bool IsOnRenderThread( void ) ;
		virtual bool IsOnAsyncNoRenderThread( void ) const = 0 ;
		// OpenGL スレッドで実行する（※要オーバーライド）
		virtual SGLError Procedure
			( SSystem::SProcedure* pProc, ProcedurePriority priority ) ;
		// レンダラ生成
		virtual S3DRenderContextInterface * NewRenderer( void ) const ;
		// レンダリングデバイス用の画像インスタンスを生成
		virtual SGLError CommitDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の VBO インスタンスを生成／更新
		virtual SGLError CommitDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の画像インスタンスを解放
		virtual SGLError ReleaseDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の VBO インスタンスを解放
		virtual SGLError ReleaseDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) ;
		// デバイス機能取得
		virtual SGLError GetDeviceFeatures( Features& features ) ;

	protected:
		class	CommitImageProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *							m_pOpenGL ;
			SSystem::SSmartReference<SGLImageObject>	m_refImage ;
			size_t										m_nUpdatePixels ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CommitImageProcedure, SProcedure )
			// 構築関数
			CommitImageProcedure
				( SGLOpenGLContext * pOpenGL,
					SGLImageObject * pImage, size_t nUpdatePixels = 0 ) ;
			// 実行
			virtual void Run( void ) ;
		} ;
		class	CommitVertexBufferProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *								m_pOpenGL ;
			SSystem::SSmartReference<SGLOpenGLVertexBuffer>	m_refVBO ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CommitVertexBufferProcedure, SProcedure )
			// 構築関数
			CommitVertexBufferProcedure
				( SGLOpenGLContext * pOpenGL, SGLOpenGLVertexBuffer * pVBO ) ;
			// 実行
			virtual void Run( void ) ;
		} ;
		class	CompileDefaultShaderProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *						m_pOpenGL ;
			SGLOpenGLContext::DefaultShaderIndex	m_dsIndex ;
			SSystem::SSignalEvent					m_eventDone ;
		public:
			bool									m_fResult ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CompileDefaultShaderProcedure, SProcedure )
			// 構築関数
			CompileDefaultShaderProcedure
				( SGLOpenGLContext * pOpenGL,
					SGLOpenGLContext::DefaultShaderIndex dsIndex ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
			// 同期
			SSystem::SError Wait( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		} ;
		class	CompileCustomShaderProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *					m_pOpenGL ;
			S3DCustomShader *					m_pShader ;
			S3DCustomShader::CompileListener *	m_pListener ;
			S3DRenderDevice::ShaderSourceInfo	m_source ;
			SSystem::SSignalEvent				m_eventDone ;
		public:
			SGLError							m_errResult ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CompileCustomShaderProcedure, SProcedure )
			// 構築関数
			CompileCustomShaderProcedure
				( SGLOpenGLContext * pOpenGL,
					S3DCustomShader * pCustomShader,
					S3DCustomShader::CompileListener * pListener,
					const S3DRenderDevice::ShaderSourceInfo& source ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
			// 同期
			SSystem::SError Wait( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		} ;
		class	LoadCustomShaderProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLContext *					m_pOpenGL ;
			S3DCustomShader *					m_pShader ;
			S3DCustomShader::CompileListener *	m_pListener ;
			const S3DShaderBinary *				m_pBinary ;
		public:
			SGLError							m_errResult ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( LoadCustomShaderProcedure, SProcedure )
			// 構築関数
			LoadCustomShaderProcedure
				( SGLOpenGLContext * pOpenGL,
					S3DCustomShader * pCustomShader,
					S3DCustomShader::CompileListener * pListener,
					const S3DShaderBinary * pBinary ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;

	protected:
		// SGLOpenGLContext チェーン
		SGLOpenGLContext *			m_pChainNext ;
		static ESL_DLL_EXPORT SGLOpenGLContext *	m_pChainFirst ;

		void AddToChain( void ) ;
		void DetachFromChain( void ) ;

	public:
		// デフォルトの SGLOpenGLContext を取得する
		static SGLOpenGLContext * GetDefault( void ) ;
		// デフォルトの SGLOpenGLContext を変更する
		static void SwitchDefault( SGLOpenGLContext * pOpenGL ) ;
		// 現在のスレッドの SGLOpenGLContext を取得する
		static SGLOpenGLContext * GetCurrentGLContext( void ) ;
		// 現在のスレッドの SGLOpenGLContext に関連付けられている SGLOpenGLView を取得
		static SGLOpenGLView * GetCurrentGLView( void ) ;
		// OpenGL スレッドで実行する
		static SGLError ProcedureOnGLThread
			( SSystem::SProcedure* pProc, ProcedurePriority priority ) ;
		// OpenGL のエラーをチェックし、エラーの場合はログに出力する
		static bool VerifyError( const char * pszFunction ) ;

	public:
		// SuitableRender 実行
		void SuitableProcedure
			( S3DRenderContextInterface::PROCEDURE_RENDERING pfnRendering, void * pInstance ) ;
		// SuitableRender 実行中にOpenGL スレッドで実行する
		SGLError ProcedureInSuitable( SSystem::SProcedure* pProc, bool fSync ) ;
	protected:
		// SuitableRender 用スレッド開始
		SGLError BeginSuitableThread( void ) ;
		// SuitableRender 用スレッド終了
		SGLError EndSuitableThread( void ) ;

		// スレッド関数
		virtual void Run( void ) ;

	public:
		// テクスチャ転送用一時バッファを取得する
		void GetTemporaryImageBuffer
			( SGLImageBuffer& imgTemp,
				uint32_t format, uint32_t depth,
				uint32_t width, uint32_t height ) ;
		// テクスチャ使用容量を加算する
		void AddUsedTextureBytes( size_t bytesTeture ) ;

	public:
		// パフォーマンスログ・フレーム開始
		virtual void BeginFramePerformanceLog( void ) ;
		// パフォーマンスログ・フレーム終了
		virtual void EndFramePerformanceLog( void ) ;
		// パフォーマンスログ・デバッグ出力
		virtual void DebugTracePerformanceLog( PerformanceLogInfo * pli ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// OpenGL ビュー・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLView
	{
	public:
		SGLSize			m_sizePhysical ;	// 物理ビューサイズ
		SGLSize			m_sizeVirtual ;		// 論理ビューサイズ
		SGLImageRect	m_rectViewPort ;	// 論理ビューポート
		SGLImageRect	m_rectViewClip ;
		S2DVector		m_vViewOffset ;		// ビュー座標オフセット
		float32_t		m_xViewLeft ;		// glOrtho 用パラメータ
		float32_t		m_xViewRight ;
		float32_t		m_yViewTop ;
		float32_t		m_yViewBottom ;
		bool			m_flagRotatable ;
		bool			m_flagRotation ;
		bool			m_flagViewport ;
		bool			m_flagZBounds ;		// ｚ範囲
		bool			m_flagOrthogonal ;	// 2D 描画座標系
		float32_t		m_zNear ;
		float32_t		m_zFar ;
		bool			m_flagMatrixPers ;
		S4DMatrix		m_matPerspective ;
		float32_t		m_matPerspectM22 ;
		float32_t		m_matPerspectM23 ;
		SGLAffine		m_affineView ;		// 論理座標→物理ビューポート座標変換
		SGLAffine		m_affineIView ;		// 物理ビューポート座標→論理座標変換
		SGLAffine		m_affineRotation ;	// 論理座標回転
		SGLAffine		m_affineViewport ;	// ビューポート表示変換

	public:
		// 構築関数
		SGLOpenGLView( void ) ;
		// 物理ビューサイズ設定
		void SetPhysicalViewSize( const SGLSize& size ) ;
		// 論理ビューサイズ設定
		void SetVirtualViewSize( const SGLSize& size ) ;
		// 論理ビューの回転（縦横比の自動的な最適化）を有効／無効化
		void EnableViewRotation( bool fRotatable = true ) ;
		// ｚ範囲を設定
		void SetZBounds
			( float32_t zNear, float32_t zFar, bool fZBounds = true ) ;
		// ビューポート更新
		void UpdateViewPort( void ) ;

	public:
		// OpenGL ビューポート設定
		void SetOpenGLViewPort
			( const SGLImageRect * pView,
					const S2DVector * pvOffset = NULL ) ;
		void RestoreOpenGLViewPort( void ) ;
		// 2D 描画用座標系設定
		void PutOpenGLOrthogonalProjection
			( SGLOpenGLShaderProgram * pShader, bool yReverse ) ;
		void PutOpenGLOrthogonalProjection
			( SGLOpenGLShaderProgram * pShader,
				double xScreen, double yScreen,
				double zScale, bool yReverse ) ;
		void PutOpenGLOrthogonalProjection
			( SGLOpenGLShaderProgram * pShader,
				const S4DMatrix& matProjection ) ;
		// 3D 描画用座標系設定
		void PutOpenGLPerspectiveProjection
			( SGLOpenGLShaderProgram * pShader,
				double xScreen, double yScreen,
					double zScreen, double fpAspectRatio ) ;
		// 透視変換行列設定
		void PutOpenGLPerspectiveMatrix
			( SGLOpenGLShaderProgram * pShader,
				const S4DMatrix& matPers, const S3DVector& vScreenPos ) ;
		// モデル座標系を初期設定
		void InitializeOpenGLModelView
			( SGLOpenGLShaderProgram * pShader,
				bool xReverse = false, bool yReverse = false,
				bool zReverse = false, bool fReverseFace = false ) ;
		// 2D 描画要座標系か？
		bool IsOrthogonalProjection( void ) const
		{
			return	m_flagOrthogonal ;
		}

	public:
		// OpenGL depth 値 -> z 値変換
		void ZValueFromDepth
			( float32_t* pzDst, const float32_t* pzSrc, size_t nCount ) ;
		SGLError ZBufferFromDepth( SGLImageBuffer& imgbuf ) ;
		// z 値 -> OpenGL depth 値変換
		void DepthFromZValue
			( float32_t* pzDst, const float32_t* pzSrc, size_t nCount ) ;
		SGLError DepthFromZBuffer( SGLImageBuffer& imgbuf ) ;

	public:
		// 変換行列取得（論理座標⇔物理座標）
		void GetAffineVirtualToPhysics( SGLAffine& affine ) ;
		// 座標変換（論理座標⇔物理座標）
		void VirtualPointFromPhysics( SGL2DVector<double,double>& v ) ;
		void VirtualPointToPhysics( SGL2DVector<double,double>& v ) ;
		// 座標変換（論理座標⇔物理ビューポート座標）
		void VertexFromPhysics( SGL2DVector<double,double>& v ) ;
		void VertexToPhysics( SGL2DVector<double,double>& v ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// フレームバッファ
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLFrameBuffer	: public S3DRenderDevice::Notify
	{
	public:
		SSystem::SSmartReference<SGLOpenGLContext>
						m_refOpenGL ;
		GLuint			m_glFrameBuffer ;		// フレームバッファ
		GLenum			m_glBufTarget ;			// GL_FRAMEBUFFER | GL_READ_FRAMEBUFFER
		bool			m_flagMultiSample ;

		SSystem::SArray<GLenum>	m_aMultiTargets ;

	public:
		// 構築関数
		SGLOpenGLFrameBuffer( void ) ;
		// 消滅関数
		virtual ~SGLOpenGLFrameBuffer( void ) ;
		// フレームバッファターゲット設定
		void SetFrameBufferTarget( GLenum glTarget ) ;
		// フレームバッファ生成
		SGLError CreateFrameBuffer( void ) ;
		// フレームバッファ破棄
		void ReleaseFrameBuffer( void ) ;
		// フレームバッファを設定
		void AttachFrameBuffer
			( SGLOpenGLTextureBuffer * pglColor = NULL,
				SGLOpenGLTextureBuffer * pglDepth = NULL ) ;
		// マルチレンダーターゲット（2つめ以降）設定
		void AddRenderTarget
			( SGLOpenGLTextureBuffer ** ppglColor, size_t nCount ) ;
		// マルチレンダーターゲット（2つめ以降）解除
		void DetachMultiTarget( void ) ;
		// フレームバッファを解除
		void DetachFrameBuffer( void ) ;

	public:
		// フレームバッファへ設定
		void AttachToFrameBuffer
			( SGLOpenGLTextureBuffer::GLResource * pglBuffer,
				GLenum attachment, GLenum txtarget, GLint iLayer ) ;
		// フレームバッファ間転送
		void BlitFramebufferTo
			( GLuint glDstFBO,
				GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
				GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1 ) ;
		// OpenGL FBO 取得
		GLuint GetFrameBufferOGL( void ) const
		{
			return	m_glFrameBuffer ;
		} ;

	public:
		// デバイスの削除前に呼び出される
		virtual void OnReleaseDevice( S3DRenderDevice * pDev ) ;

	protected:
		// フレームバッファ破棄
		class	FrameBufferDestroyer	: public SSystem::SProcedure
		{
		protected:
			GLuint	m_glFrameBuffer ;
		public:
			// 構築関数
			FrameBufferDestroyer( GLuint glFrameBuffer ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// OpenGL レンダリング・コンテキスト (OpenGL 1.x 対応用)
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLRenderingContext
	{
	protected:
		// 最後に設定された表面属性
		S3DMaterial *			m_pLastMaterial ;
		bool					m_flagBackFace ;
		bool					m_flagNeedsMulAlpha ;
		bool					m_flagWithoutAlpha ;
		S3DSurfaceAttribute *	m_pLastSufAttr ;
		SGLImageObject *		m_pLastTextureImage ;
		SGLOpenGLTextureBuffer::GLResource *
								m_pglLastTexture ;
		SGLAffine				m_affineTextureUV ;

		// 機能フラグ
		bool			m_flagLightingGL ;
		bool			m_flagTextureSmoothing ;

		// 効果
		bool			m_flagMulColor ;
		bool			m_flagAddColor ;
		GLfloat			m_colorMulEffect[3] ;
		GLfloat			m_colorAddEffect[3] ;
		GLfloat			m_alphaEnvEffect ;
		GLfloat			m_alphaEffect ;

		// バッファ
		SSystem::SArray<S2DVector>	m_bufUVMap ;
		SSystem::SArray<GLfloat>	m_bufAddColor ;
		SSystem::SArray<GLfloat>	m_bufMulColor ;
		#if	defined(__API_OPEN_GL_ES__)
		SSystem::SArray<uint16_t>	m_bufIndexed16 ;
		#endif

	public:
		// 構築関数
		SGLOpenGLRenderingContext( void ) ;
		// 機能フラグ設定
		void EnableLightingByGL( bool flagLighting ) ;
		void EnableTextureSmoothing( bool flagSmoothing ) ;
		// 機能フラグ取得
		bool IsEnabledLightingByGL( void ) const
		{
			return	m_flagLightingGL ;
		}
		bool IsEnabledTextureSmoothing( void ) const
		{
			return	m_flagTextureSmoothing ;
		}
		// 効果を設定
		void SetColorEffect
			( const S3DColor * colorEffect, unsigned int nTransparency ) ;

	public:
		// 光源を設定する
		void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// 疑似フォッグを設定
		void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		void EnableFog( bool fFog ) ;

	public:
/*
		// ポリゴンリストをレンダリングバッファに追加
		SGLError AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		SGLError AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
*/
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;

	public:
		// 頂点の設定をクリアする
		void FlushVertexPointers( void ) ;
		// 頂点を設定する
		void PutVertexPointer
			( const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, size_t nCount ) ;
		// 頂点色を設定する
		bool PutVertexColors( const S3DColor * pColor, size_t nCount ) ;
		// 頂点色を設定する（加算成分）
		void PutVertexAddColors( void ) ;
		// 頂点色を設定する（乗算成分）
		void PutVertexMulColors( void ) ;
		// 頂点色変換
		void ConvertColorToFloat
			( GLfloat * pFloat, const S3DColor * pColor, size_t nCount ) ;
		void ConvertMulColorToFloat
			( GLfloat * pFloat, const S3DColor * pColor, size_t nCount ) ;
		bool ConvertAddColorToFloat
			( GLfloat * pFloat, const S3DColor * pColor, size_t nCount ) ;

	public:
		// マテリアル設定をクリアする
		void FlushGLMaterial( void ) ;
		// マテリアルを設定する
		SGLError PutGLMaterial
			( SGLOpenGLContext * pOpenGL, S3DMaterial * pMaterial, bool fBack ) ;
		// テクスチャ設定
		SGLOpenGLTextureBuffer::GLResource *
			BindGLTexture( SGLImageObject * pTextureImage ) ;
		// テクスチャ補完設定
		static void PutGLTextureSmoothing( bool fSmoothing ) ;
		// テクスチャ繰り返し設定
		static void PutGLTextureTiling( bool fTiling ) ;
		// 入力画像はα非積算済みか？
		bool IsNeedsMulAlpha( void ) const
		{
			return	m_flagNeedsMulAlpha ;
		}
		// 入力画像はα無しか？
		bool IsTextureWithoutAlpha( void ) const
		{
			return	m_flagWithoutAlpha ;
		}
		// テクスチャのUV座標変換行列取得
		const SGLAffine& GetAffineForTextureUV( void ) const
		{
			return	m_affineTextureUV ;
		}

	protected:
		// ベース色レンダリング用マテリアル補正
		void ReviseGLMaterialForBaseColor( SGLOpenGLContext * pOpenGL ) ;
		// 加算色レンダリング用マテリアル設定
		void PutGLMaterialForAddColor( SGLOpenGLContext * pOpenGL ) ;
		// 加算色レンダリング用後マテリアル再設定
		void PutGLMaterialAfterAddColor( SGLOpenGLContext * pOpenGL ) ;

	} ;

}

#endif
