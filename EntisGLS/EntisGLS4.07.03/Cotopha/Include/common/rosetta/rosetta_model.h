
#if	!defined(__ROSETTA_MODEL_H__)
#define	__ROSETTA_MODEL_H__	1

#include <sakuraglx/sgl_object.h>
#include <sakuraglx/render/sglx_model_buffer.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Vector3D 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSVector3DClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVector3DClass, RSStructuredPointerClass )
		// 構築関数
		RSVector3DClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Vector3D" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S3DVector *
			GetThisVector( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( double x, double y, double z )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean equals( Vector3D v )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D copy( Vector3D v )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D add( Vector3D v )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D sub( Vector3D v )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D mul( double s )
		static RSObject * method_mul
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D div( double s )
		static RSObject * method_div
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double absolute()
		static RSObject * method_absolute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalize()
		static RSObject * method_normalize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double innerProduct( Vector3D v )
		static RSObject * method_innerProduct
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D exteriorProduct( Vector3D v )
		static RSObject * method_exteriorProduct
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Vector3D4 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSVector3D4Class	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVector3D4Class, RSStructuredPointerClass )
		// 構築関数
		RSVector3D4Class
			( RSClass * pClass, const wchar_t * pwszClassName = L"Vector3D4" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S4DVector *
			GetThisVector( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( double x, double y, double z, double w )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D4 clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D4 copy( Vector3D4 v )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D add( Vector3D v )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D sub( Vector3D v )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D mul( double s )
		static RSObject * method_mul
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D div( double s )
		static RSObject * method_div
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Color3D 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSColor3DClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSColor3DClass, RSStructuredPointerClass )
		// 構築関数
		RSColor3DClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Color3D" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S3DColor *
			GetThisColor( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( int rgbMul, int rgbAdd )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Color3D set( int rgbMul, int rgbAdd )
		static RSObject * method_set
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Color3D copy( Color3D color )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Color3D blend( Color3D color )
		static RSObject * method_blend1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int blend( int color )
		static RSObject * method_blend2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Quaternion 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSQuaternionClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSQuaternionClass, RSStructuredPointerClass )
		// 構築関数
		RSQuaternionClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Quaternion" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S3DQuaternion *
			GetThisQuaternion( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( double q0, double q1, double a2, double q3 )
		static RSObject * method_init0
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Quaternion q )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Matrix3D mat3 )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean equals( Quaternion q )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Quaternion clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion copy( Quaternion q )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion add( Quaternion q )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion sub( Quaternion q )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion mul( Quaternion q )
		static RSObject * method_mul1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion mul( double s )
		static RSObject * method_mul2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion div( double s )
		static RSObject * method_div
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double norm()
		static RSObject * method_norm
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion normalize()
		static RSObject * method_normalize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double innerProduct( Quaternion q )
		static RSObject * method_innerProduct
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Quaternion inverse()
		static RSObject * method_inverse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix3D toMatrix( Matrix3D mat3 )
		static RSObject * method_toMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion fromMatrix( Matrix3D mat3 )
		static RSObject * method_fromMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double getRodriguesRotaion( Vector3D v )
		static RSObject * method_getRodriguesRotaion
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion setRodriguesRotaion( Vector3D v, double rad )
		static RSObject * method_setRodriguesRotaion
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion lerp( Quaternion q0, Quaternion q1, double t )
		static RSObject * method_lerp
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Quaternion slerp( Quaternion q0, Quaternion q1, double t )
		static RSObject * method_slerp
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Matrix3D 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSMatrix3DClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMatrix3DClass, RSStructuredPointerClass )
		// 構築関数
		RSMatrix3DClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Matrix3D" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::SGL3DMatrix<float32_t,3> *
			GetThisMatrix( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( double m11, double m12, double m13,
		//				double m21, double m22, double m23,
		//				double m31, double m32, double m33 )
		static RSObject * method_init0
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Vector3D v0, Vector3D v1, Vector3D v2 )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Vector3D v0 )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Quaternion q )
		static RSObject * method_init3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean equals( Matrix3D m )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix3D clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D copy( Matrix3D m )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D add( Matrix3D m )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D sub( Matrix3D m )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D mul( Matrix3D m )
		static RSObject * method_mul1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D mul( Vector3D v )
		static RSObject * method_mul2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D mul( double s )
		static RSObject * method_mul3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D div( double s )
		static RSObject * method_div
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D transpose()
		static RSObject * method_transpose
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D inverse()
		static RSObject * method_inverse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double determinant()
		static RSObject * method_determinant
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D magnifyByVector( Vector3D v )
		static RSObject * method_magnifyByVector
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D magnifyOnVectorOf( Vector3D v, double s )
		static RSObject * method_magnifyOnVectorOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateOnX( double sin, double cos )
		static RSObject * method_rotateOnX
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateOnY( double sin, double cos )
		static RSObject * method_rotateOnY
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateOnZ( double sin, double cos )
		static RSObject * method_rotateOnZ
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateOnVectorOf( Vector3D v, double sin, double cos )
		static RSObject * method_rotateOnVectorOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D vectorRotationOf( Vector3D v0, Vector3D v1 )
		static RSObject * method_vectorRotationOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateByAngleOn( Vector3D v )
		static RSObject * method_rotateByAngleOn
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix3D rotateForAngle( Vector3D v )
		static RSObject * method_rotateForAngle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector3D cameraAngleOf( Vector3D vTarget, Vector3D vView, Vector3D vTop )
		static RSObject * method_cameraAngleOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void transformVectors( Vector3D4 vDst, Vector3D4 vSrc, int count, Vector3D vOffset = null )
		static RSObject * method_transformVectors
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Matrix4D 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSMatrix4DClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMatrix4DClass, RSStructuredPointerClass )
		// 構築関数
		RSMatrix4DClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Matrix4D" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S4DMatrix *
			GetThisMatrix( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( double m11, double m12, double m13, double m14,
		//				double m21, double m22, double m23, double m24,
		//				double m31, double m32, double m33, double m34,
		//				double m41, double m42, double m43, double m44 )
		static RSObject * method_init0
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Matrix3D m, Vector3D v )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Vector3D4 v0, Vector3D4 v1, Vector3D4 v2, Vector3D4 v3 )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Vector3D4 v )
		static RSObject * method_init3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean equals( Matrix4D m )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix4D clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D copy( Matrix4D m )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix3D getMatrix3D( Matrix3D m = null )
		static RSObject * method_getMatrix3D
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D setMatrix3D( Matrix3D m )
		static RSObject * method_setMatrix3D
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D getTranslation( Vector3D v = null )
		static RSObject * method_getTranslation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D setTranslation( Vector3D v )
		static RSObject * method_setTranslation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D add( Matrix4D m )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D sub( Matrix4D m )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D mul( Matrix4D m )
		static RSObject * method_mul1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D4 mul( Vector3D4 v )
		static RSObject * method_mul2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D mul( double s )
		static RSObject * method_mul3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D scale( double x, double y, double z )
		static RSObject * method_scale
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D translate( double x, double y, double z )
		static RSObject * method_translate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D transpose()
		static RSObject * method_transpose
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Matrix4D inverse()
		static RSObject * method_inverse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double determinant()
		static RSObject * method_determinant
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// TextureLibrary クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSTextureLibraryClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSTextureLibraryClass, RGenericNativeObjectClass )
		// 構築関数
		RSTextureLibraryClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"TextureLibrary" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DTextureLibrary *
			GetThisTextureLibrary( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Image getTextureAs( String id )
		static RSObject * method_getTextureAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addTextureAs( String id, Image texture )
		static RSObject * method_addTextureAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean removeTextureAs( String id )
		static RSObject * method_removeTextureAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean removeAllTexture()
		static RSObject * method_removeAllTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SurfaceAttribute 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSSurfaceAttributeClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( RSSurfaceAttributeClass, RSStructuredPointerClass )
		// 構築関数
		RSSurfaceAttributeClass
			( RSClass * pClass,
				const wchar_t * pwszClassName = L"SurfaceAttribute" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::S3DSurfaceAttribute *
			GetThisSurfaceAttribute( RSContext& context, RSObject* pThis ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Material クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMaterialClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMaterialClass, RGenericNativeObjectClass )
		// 構築関数
		RSMaterialClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Material" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DMaterial *
			GetThisMaterial( RSContext& context, RSObject* pThis ) ;
		static SakuraGL::S3DMaterial *
			GetMaterialOf( RSContext& context, RSObject* pObj ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void getSurfaceAttribute( SurfaceAttribute attr )
		static RSObject * method_getSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void getBackSurfaceAttribute( SurfaceAttribute attr )
		static RSObject * method_getBackSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isEnabledBackSurfaceAttribute()
		static RSObject * method_isEnabledBackSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSurfaceAttribute( SurfaceAttribute attr )
		static RSObject * method_setSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setBackSurfaceAttribute( SurfaceAttribute attr )
		static RSObject * method_setBackSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void enableBackSurfaceAttribute( boolean flagBack )
		static RSObject * method_enableBackSurfaceAttribute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getTexture( int iTexture = 0 )
		static RSObject * method_getTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getBackTexture( int iTexture = 0 )
		static RSObject * method_getBackTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getTextureFlags( int iTexture = 0 )
		static RSObject * method_getTextureFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getTextureType( int iTexture = 0 )
		static RSObject * method_getTextureType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getBackTextureFlags( int iTexture = 0 )
		static RSObject * method_getBackTextureFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getBackTextureType( int iTexture = 0 )
		static RSObject * method_getBackTextureType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float getTextureApplication( int iTexture = 0 )
		static RSObject * method_getTextureApplication
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float getBackTextureApplication( int iTexture = 0 )
		static RSObject * method_getBackTextureApplication
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float getTextureParameter( int iTexture = 0 )
		static RSObject * method_getTextureParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float getBackTextureParameter( int iTexture = 0 )
		static RSObject * method_getBackTextureParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int findTextureTypeOf( int type )
		static RSObject * method_findTextureTypeOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int findBackTextureTypeOf( int type )
		static RSObject * method_findBackTextureTypeOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTexture
		//	( Image image, int iTexture = 0,
		//		int nFlags = Material.textureDiffusion,
		//		float nApply = 1.0, float nParam1 = 0.0 )
		static RSObject * method_setTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setBackTexture
		//	( Image image, int iTexture = 0,
		//		int nFlags = Material.textureDiffusion,
		//		float nApply = 1.0, float nParam1 = 0.0 )
		static RSObject * method_setBackTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ModelPose クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSModelPoseClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSModelPoseClass, RGenericNativeObjectClass )
		// 構築関数
		RSModelPoseClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"ModelPose" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DModelPose *
			GetThisModelPose( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getAnimationTime()
		static RSObject * method_getAnimationTime
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double getFramePerSec()
		static RSObject * method_getFramePerSec
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void applyPoseTo( ModelBuffer model, double w = 1.0, double sec = 0.0 )
		static RSObject * method_applyPoseTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void productPoseTo( ModelBuffer model, double w = 1.0, double sec = 0.0 )
		static RSObject * method_productPoseTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// MaterialLibrary クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMaterialLibraryClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMaterialLibraryClass, RGenericNativeObjectClass )
		// 構築関数
		RSMaterialLibraryClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"MaterialLibrary" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DMaterialLibrary *
			GetThisMaterialLibrary( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Material getMaterialAs( String id )
		static RSObject * method_getMaterialAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addMaterialAs( String id, Material material )
		static RSObject * method_addMaterialAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean removeMaterialAs( String id )
		static RSObject * method_removeMaterialAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean removeAllMaterial()
		static RSObject * method_removeAllMaterial
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ModelPoseLibrary クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSModelPoseLibraryClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSModelPoseLibraryClass, RGenericNativeObjectClass )
		// 構築関数
		RSModelPoseLibraryClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"ModelPoseLibrary" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DModelPoseLibrary *
			GetThisPoseLibrary( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadLibrary( String file )
		static RSObject * method_loadLibrary
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readLibrary( InputStream is )
		static RSObject * method_readLibrary
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ModelPose getPoseAs( String id )
		static RSObject * method_getPoseAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// CustomShader クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSCustomShaderClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCustomShaderClass, RGenericNativeObjectClass )
		// 構築関数
		RSCustomShaderClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"CustomShader" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DCustomShader *
			GetThisCustomShader( RSContext& context, RSObject* pThis ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderDevice クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRenderDeviceClass	: public RGenericNativeObjectClass
	{
	public:
		class	ShaderDescClass	: public RGenericNativeObjectClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ShaderDescClass, RGenericNativeObjectClass )
			// 構築関数
			ShaderDescClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"ShaderDesc" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;

		public:
			// void <init>()
			static RSObject * method_init
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// boolean loadDescriptor( String strDescFile )
			static RSObject * method_loadDescriptor
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// boolean parseDescriptor( Uint8Pointer ptrDescXml )
			static RSObject * method_parseDescriptor
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;
		class	DefaultShaderIdClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( DefaultShaderIdClass, RSClass )
			// 構築関数
			DefaultShaderIdClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"DefaultShaderId" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRenderDeviceClass, RGenericNativeObjectClass )
		// 構築関数
		RSRenderDeviceClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RenderDevice" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DRenderDevice *
			GetThisRenderDevice( RSContext& context, RSObject* pThis ) ;

	public:
		// const boolean isOnRenderThread()
		static RSObject * method_isOnRenderThread
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean procedure( Runnable proc, int priority )
		static RSObject * method_procedure
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean waitUntilAsyncAllProcedures( long timeout )
		static RSObject * method_waitUntilAsyncAllProcedures
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RenderContext newRenderer()
		static RSObject * method_newRenderer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean commitDeviceImage( Image img, long timeout = 0 )
		static RSObject * method_commitDeviceImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean commitDeviceVertexBuffer( VertexBuffer vb, long timeout = 0 )
		static RSObject * method_commitDeviceVertexBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean releaseDeviceImage( Image img, long timeout = 0 )
		static RSObject * method_releaseDeviceImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean releaseDeviceVertexBuffer( VertexBuffer vb, long timeout = 0 )
		static RSObject * method_releaseDeviceVertexBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// CustomShader getCustomShaderAs( String id )
		static RSObject * method_getCustomShaderAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// CustomShader buildCustomShader( String id, RenderDevice.ShaderDesc shddsc )
		static RSObject * method_buildCustomShader
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeCustomShaderAs( String id )
		static RSObject * method_removeCustomShaderAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderBuffer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRenderBufferClass	: public RGenericNativeObjectClass
	{
	public:
		// RenderBuffer.EnvMappingParam クラス
		class	EnvMappingParamClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( EnvMappingParamClass, RSStructuredPointerClass )
			// 構築関数
			EnvMappingParamClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"EnvMappingParam" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;

			// Object -> RenderContext::EnvMappingParam 変換
			static void FromObject
				( RSContext& context,
					SakuraGL::S3DRenderContextInterface::EnvMappingParam& param, RSObject * pObj ) ;
			// Object <- RenderContext::EnvMappingParam 変換
			static void ToObject
				( RSContext& context, RSObject * pObj,
					const SakuraGL::S3DRenderContextInterface::EnvMappingParam& param ) ;
		} ;

		// RenderBuffer.OffsetBorderParam クラス
		class	OffsetBorderParamClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( OffsetBorderParamClass, RSStructuredPointerClass )
			// 構築関数
			OffsetBorderParamClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"OffsetBorderParam" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

		// RenderBuffer.OptionalContextSet クラス
		class	OptionalContextSetClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( OptionalContextSetClass, RSStructuredPointerClass )
			// 構築関数
			OptionalContextSetClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"OptionalContextSet" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRenderBufferClass, RGenericNativeObjectClass )
		// 構築関数
		RSRenderBufferClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RenderBuffer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DRenderBufferInterface *
			GetThisRenderBuffer( RSContext& context, RSObject* pThis ) ;
		// Object -> S3DRenderBufferInterface 変換
		static SakuraGL::S3DRenderBufferInterface *
			RenderBufferFromObject( RSContext& context, RSObject* pObject ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setShadingFlag( long nShadingMethod )
		static RSObject * method_setShadingFlag
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getShadingFlag()
		static RSObject * method_getShadingFlag
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void appendMatrixTransformation
		// ( Matrix3D mat, Vector3D pos,
		//		Color3D color = null, int transparency = 0 )
		static RSObject * method_appendMatrixTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMatrixTransformation
		// ( Matrix3D mat, Vector3D pos,
		//		Color3D color = null, int transparency = 0 )
		static RSObject * method_setMatrixTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const void getMatrixTransformation
		// ( Matrix3D mat, Vector3D pos,
		//		Color3D color, Uint32Pointer transparency )
		static RSObject * method_getMatrixTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void pushTransformation()
		static RSObject * method_pushTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void popTransformation()
		static RSObject * method_popTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void resetTransformation()
		static RSObject * method_resetTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean attachCustomShader( CustomShader pShader )
		static RSObject * method_attachCustomShader
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const CustomShader getCustomShader()
		static RSObject * method_getCustomShader
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setCustomShaderUniform
		//	( String idUniform, int type, Uint8Pointer pData, int count )
		static RSObject * method_setCustomShaderUniform
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean resetCustomShaderUniform()
		static RSObject * method_resetCustomShaderUniform
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setOptionalFeature
		// ( int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2 )
		static RSObject * method_setOptionalFeature
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getOptionalFeature
		// ( int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2 )
		static RSObject * method_getOptionalFeature
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addIndexedPrimitiveList
		// ( Material material, int nFlags, int typePrimitive,
		//	int countIndex, int countVertex,
		//	Vector3D4 vVertex, Vector3D4 vNormal,
		//	Vector2D vUV, Color3D color, Uint32Pointer pIndex )
		static RSObject * method_addPrimitiveList
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addVertexBuffer
		// ( int nFlags, VertexBuffer vb,
		//	int iFirst = 0, int iEnd = -1,
		//	int nInstancing = 0,
		//	Matrix4D pMatrixInstance = null,
		//	Color3D pColorInstance = null )
		static RSObject * method_addVertexBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// vod flush()
		static RSObject * method_flush
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// VertexVariantBuffer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVertexVariantBufferClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVertexVariantBufferClass, RGenericNativeObjectClass )
		// 構築関数
		RSVertexVariantBufferClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"VertexVariantBuffer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DVertexVariantBuffer *
			GetThisVertexVariantBuffer( RSContext& context, RSObject* pThis ) ;
		// Object -> S3DVertexVariantBuffer 変換
		static SakuraGL::S3DVertexVariantBuffer *
			VertexVariantBufferFromObject( RSContext& context, RSObject* pObject ) ;

	public:
		// boolean setBoneMatrix( int iMesh, int nCount, Matrix4D pMatrix )
		static RSObject * method_setBoneMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getBoneMatrix( int iMesh, int nCount, Matrix4D pMatrix )
		static RSObject * method_getBoneMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setMorphingApplication
		//	( int iMesh, Int32Pointer pTarget,
		//	Float32Pointer pApplication, int nTargetCount )
		static RSObject * method_setMorphingApplication
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getMorphingApplication
		//	( int iMesh, Int32Pointer pTarget,
		//	Float32Pointer pApplication, int iTargetIndex )
		static RSObject * method_getMorphingApplication
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean enableToRenderMesh
		//	( int iFirst = 0, int iEnd = -1, boolean fEnable = true )
		static RSObject * method_enableToRenderMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isEnabledToRenderMesh( int iMesh )
		static RSObject * method_isEnabledToRenderMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setMaterialToRenderMesh( int iMesh, Material material )
		static RSObject * method_setMaterialToRenderMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Material getMaterialToRenderMesh( int iMesh )
		static RSObject * method_getMaterialToRenderMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVertexBufferClass	: public RSClass
	{
	public:
		// VertexBuffer.MeshInfo クラス
		class	MeshInfoClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MeshInfoClass, RSClass )
			// 構築関数
			MeshInfoClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"MeshInfo" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

		// VertexBuffer.PrimitiveBuffer クラス
		class	PrimitiveBufferClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PrimitiveBufferClass, RSClass )
			// 構築関数
			PrimitiveBufferClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"PrimitiveBuffer" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVertexBufferClass, RSClass )
		// 構築関数
		RSVertexBufferClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"VertexBuffer" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DVertexBufferInterface *
			GetThisVertexBuffer( RSContext& context, RSObject* pThis ) ;
		// Object -> S3DVertexBufferInterface 変換
		static SakuraGL::S3DVertexBufferInterface *
			VertexBufferFromObject( RSContext& context, RSObject* pObject ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getBufferControlFlags()
		static RSObject * method_getBufferControlFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setBufferControlFlags( int nFlags )
		static RSObject * method_setBufferControlFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Material getDefaultMaterial()
		static RSObject * method_getDefaultMaterial
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachDefaultMaterial( Material material )
		static RSObject * method_attachDefaultMaterial
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean allocatePrimitiveBuffer
		//	( PrimitiveBuffer prmbuf, int typePrimitive,
		//		int countIndex, int countVertex )
		static RSObject * method_allocatePrimitiveBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addPrimitiveBuffer
		//	( Material material, int nFlags, int typePrimitive,
		//		PrimitiveBuffer prmbuf, int countIndex, int countVertex )
		static RSObject * method_addPrimitiveBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean freePrimitiveBuffer( PrimitiveBuffer prmbuf )
		static RSObject * method_freePrimitiveBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getMeshCount()
		static RSObject * method_getMeshCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getMeshInfoAt
		//	( MeshInfo info, int iMesh, int nCopyVerteics )
		static RSObject * method_getMeshInfoAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean updatePrimitiveList
		//	( int iMesh, int nFlags, int countIndex, int countVertex,
		//		Vector3D4 vVertex, Vector3D4 vNormal,
		//		Vector2D vUVMap, Color3D pColor Uint32Pointer pIndex )
		static RSObject * method_updatePrimitiveList
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setBoneWeightMap
		//	( int iMesh, int nCount, Float32Pointer pWeightMaps )
		static RSObject * method_setBoneWeightMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setBoneJointMap
		//	( int iMesh, int nBoneCount, int nJointCount, Uint32Pointer pJointMaps )
		static RSObject * method_setBoneJointMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean allocateMorphing( int iMesh, int nTargetCount )
		static RSObject * method_allocateMorphing
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setMorphingTargetMesh
		//	( int iMesh, int iMorphTarget, int countVertex,
		//		Vector3D4 vVertex, Vector3D4 vNormal,
		//		Vector2D vUVMap, Color3D pColor )
		static RSObject * method_setMorphingTargetMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setMorphingTargetWeight
		//	( int iMesh, int iMorphTarget,
		//		int countVertex, Float32Pointer pWeight )
		static RSObject * method_setMorphingTargetWeight
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// VertexVariantBuffer createVariantBuffer()
		static RSObject * method_createVariantBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// VertexVariantBuffer newReferenceVariantBuffer()
		static RSObject * method_newReferenceVariantBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean updateVertexVariant
		//	( VertexVariantBuffer vvb, int iFirst = 0, int iEnd = -1 )
		static RSObject * method_updateVertexVariant
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void clearBuffer()
		static RSObject * method_clearBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getCircumscribedParallelepiped( Vector3D vMin, Vector3D vMax )
		static RSObject * method_getCircumscribedParallelepiped
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int countOfTotalPolygons()
		static RSObject * method_countOfTotalPolygons
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int countOfTotalVertices()
		static RSObject * method_countOfTotalVertices
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean enableMultiInstancingMode( boolean flagEnable )
		static RSObject * method_enableMultiInstancingMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isMultiInstancingMode()
		static RSObject * method_isMultiInstancingMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean clearAllInstance()
		static RSObject * method_clearAllInstance
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addInstanceVariant
		//	( VertexVariantBuffer vvb, Matrix4D matrix, Color3D color )
		static RSObject * method_addInstanceVariant
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ModelBuffer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSModelBufferClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSModelBufferClass, RSClass )
		// 構築関数
		RSModelBufferClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"ModelBuffer" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DModelBuffer *
			GetThisModelBuffer( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadModel( String file, String mime = null )
		static RSObject * method_loadModel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readModel( InputStream is, String mime = null )
		// boolean readModel( RandomAccessFile file, String mime = null )
		static RSObject * method_readModel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean saveModel
		//	( String file, String mime = null, String mimeImage = null )
		static RSObject * method_saveModel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean writeModel
		//	( RandomAccessFile file,
		//		String mime = null, String mimeImage = null )
		static RSObject * method_writeModel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// TextureLibrary getTextureLibrary()
		static RSObject * method_getTextureLibrary
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// MaterialLibrary getMaterialLibrary()
		static RSObject * method_getMaterialLibrary
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ModelPoseLibrary getPoseLibrary()
		static RSObject * method_getPoseLibrary
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderContext クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRenderContextClass	: public RSClass
	{
	public:
		// RenderContext.Light
		class	RSLightClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO
				( RSLightClass, RSStructuredPointerClass )
			// 構築関数
			RSLightClass
				( RSClass * pClass,
					const wchar_t * pwszClassName = L"Light" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

		// RenderContext.ShadowMapInfo
		class	RSShadowMapInfoClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO
				( RSShadowMapInfoClass, RSStructuredPointerClass )
			// 構築関数
			RSShadowMapInfoClass
				( RSClass * pClass,
					const wchar_t * pwszClassName = L"ShadowMapInfo" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRenderContextClass, RSClass )
		// 構築関数
		RSRenderContextClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RenderContext" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DRenderContextInterface *
			GetThisRenderContext( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean attachMultiTargetImages( Image[] targets )
		static RSObject * method_attachMultiTargetImages
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Image[] getMultiTargetImages()
		static RSObject * method_getMultiTargetImages
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setProjectionScreen
		//	( Vector3D vScreen, double zScale = 1.0, double pixelAspect = 1.0 )
		static RSObject * method_setProjectionScreen
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getProjectionScreen
		//	( Vector3D vScreen, Float64Pointer zScale, Float64Pointer pixelAspect )
		static RSObject * method_getProjectionScreen
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getPerspectiveMatrix( Matrix4D matPers )
		static RSObject * method_getPerspectiveMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPerspectiveMatrix
		//	( int stereoViewIndex, Matrix4D matPers, boolean flagPersMatrix = true )
		static RSObject * method_setPerspectiveMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void enablePerspectiveMatrix( boolean flagPersMatrix )
		static RSObject * method_enablePerspectiveMatrix
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCamera( Matrix3D matCamera, Vector3D vCamera )
		static RSObject * method_setCamera
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void getCamera( Matrix3D matCamera, Vector3D vCamera )
		static RSObject * method_getCamera
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setInverseCameraTransformation
		//		( Color3D color = null, int transparency = 0 )
		static RSObject * method_setInverseCameraTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isSphereIntoView( Vector3D vLocalPos, double radius )
		static RSObject * method_isSphereIntoView
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getProjectedPosition
		//	( Vector2D vProjPos, Vector3D vLocalPos, double fpLimit = 100000.0 )
		static RSObject * method_getProjectedPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setParallax
		//	( double xParallax, double zFocusRate, double xScreenDelta )
		static RSObject * method_setParallax
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double getParallax()
		static RSObject * method_getParallax
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setZClipRange( double zMin, double zMax )
		static RSObject * method_setZClipRange
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setLightEntries( RenderContext.Light pLights, int countLights )
		static RSObject * method_setLightEntries
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setShadowMap
		//	( int idLight, Image pShadowDepth,
		//		RenderContext.ShadowMpaInfo infShadowMap )
		static RSObject * method_setShadowMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFog( int rgbFog, double zFogNear, double zFogFar )
		static RSObject * method_setFog
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void enableFog( boolean flagEnable )
		static RSObject * method_enableFog
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int currentParallaxView()
		static RSObject * method_currentParallaxView
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean selectParallaxView( int stereoViewIndex )
		static RSObject * method_selectParallaxView
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean begin3DRenderer( long flags = 0 )
		static RSObject * method_begin3DRenderer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean end3DRenderer( long flags = 0 )
		static RSObject * method_end3DRenderer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RenderDevice getRenderDevice( long flags = 0 )
		static RSObject * method_getRenderDevice
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addTemporaryObject( NativeObject obj )
		static RSObject * method_addTemporaryObject
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


}

#endif

