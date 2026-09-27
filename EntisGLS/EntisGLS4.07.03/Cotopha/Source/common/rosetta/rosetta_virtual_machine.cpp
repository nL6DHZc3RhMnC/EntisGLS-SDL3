
#include <rosetta/rosetta.h>
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <glscs/glscs_sakura2_module_maker.h>
#include <sakuraglx/sglx_std_app.h>
#include <rosetta/rosetta_compiler.h>
#include <rosetta/rosetta_number.h>
#include <rosetta/rosetta_string.h>
#include <rosetta/rosetta_array.h>
#include <rosetta/rosetta_date.h>
#include <rosetta/rosetta_file.h>
#include <rosetta/rosetta_thread.h>
#include <rosetta/rosetta_crypt.h>
#include <rosetta/rosetta_dialog.h>
#include <rosetta/rosetta_image.h>
#include <rosetta/rosetta_sprite.h>
#include <rosetta/rosetta_media.h>
#include <rosetta/rosetta_model.h>
#include <rosetta/rosetta_scene.h>

using namespace	SSystem ;
using namespace SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 仮想マシン
//////////////////////////////////////////////////////////////////////////////

// クラス・ドキュメント用
//////////////////////////////////////////////////////////////////////////////
static const wchar_t * s_pwszClassDoc[][2] =
{
	{ L"Object",
		L"　すべてのクラスの基底クラスです。" },
	{ L"Integer",
		L"　Integer クラスは64ビット整数オブジェクトを実装します。" },
	{ L"Number",
		L"　Number クラスは数値オブジェクト（32ビット整数、64ビット浮動小数点）を実装します。\n"
		L"　尚、Number は32ビット整数も表現できますが、通常のモードでは Integer で32ビット以下の整数も表現されます。" },
	{ L"String",
		L"　String クラスは文字列オブジェクトを実装します。\n"
		L"　setString メソッド以外はすべて const メソッドです。\n"
		L"　文字コード配列は toCharArray()、又は encodeTo(String.encodingUTF16) で取得できます。" },
	{ L"Array",
		L"　Array クラスは配列オブジェクトを実装します。\n"
		L"　Array オブジェクトに対し [ ] 括弧演算子で配列要素を取得できます。\n"
		L"　指標は 0 から始まる整数で、最大値は Array 構築時に渡された配列最大長 - 1 までとなります。" },
	{ L"HashMap",
		L"　HashMap クラスは文字列索引型配列オブジェクトを実装します。\n"
		L"　HashMap オブジェクトに対し、[ ] 括弧演算子でメンバを参照する場合、"
		L"要素指標として String を指定した場合には対応する要素が取得されます。\n"
		L"　0 から始まる Integer を要素指標として指定した場合には要素を列挙できます。"
		L"Integer 指標の範囲は size 関数で取得できます。" },
	{ L"ArrayBuffer",
		L"　ArrayBuffer クラスはバッファを提供します。" },
	{ L"Uint8Pointer",
		L"　Uint8Pointer はバッファへのポインタを提供します。" },
	{ L"Structure",
		L"　Structure クラスはすべての構造体の基底クラスとなります。" },
	{ L"Exception",
		L"　Exception は標準的な例外の基底クラスです。\n"
		L"　構築関数に渡した文字列は、(String）キャストか、toString 関数で取り出すことが出来ます。" },
	{ L"Runnable",
		L"　Runnable は run 関数のためのインターフェースです。" },
	{ L"Thread",
		L"　Thread はスレッドを提供します。" },
	{ L"System",
		L"　System はシステム情報や機能を提供します。" },
	{ L"Console",
		L"　Console はコンソールへの入出力インターフェースを提供します。" },
	{ L"Date",
		L"　Date はカレンダー機能を提供します。\n"
		L"　デフォルトのコンストラクタは現時刻を格納しています。\n"
		L"　getTime() で取得した long をコンストラクタに渡すと年月日・時刻を再フォーマットできます。" },
	{ L"Math",
		L"　Math は各種算術関数を提供します。" },
	{ L"File",
		L"　File はファイルやディレクトリの情報、操作機能を提供します。" },
	{ L"InputStream",
		L"　InputStream は入力ストリームインターフェースを提供します。" },
	{ L"OutputStream",
		L"　OutputStream は出力ストリームインターフェースを提供します。" },
	{ L"RandomAccessFile",
		L"　RandomAccessFile はファイルへの入出力を提供します。\n"
		L"　getInputStream, getOutputStream 関数で InputStream, OutputStream インターフェースを"
		L"取得できます。getOutputStream で取得した OutputStream への出力はバッファリング"
		L"されているので、出力を確定するには flush 関数を明示的に呼び出す必要があります。" },
	{ L"SmartBufferFile",
		L"　SmartBufferFile はメモリ上バッファの可変長ファイルインターフェースを提供します。" },
	{ L"HttpInputStream",
		L"　HttpInputStream は HTTP、HTTPS 上のファイルへの接続と入力ストリームを提供します。"
		L"EntisGLS4 の SHttpFile クラスとほぼ同等です。" },
	{ L"NoaFileArchiver",
		L"　NoaFileArchiver は NOA 形式書庫ファイルへのインターフェースを提供します。" },
	{ L"StringParser",
		L"　StringParser は文字列の構文解析を行います。\n"
		L"　EntisGLS4 の SStringParser と同等です。\n"
		L"　構築関数や attachString 関数で設定された、又は loadTextFile, readTextFile で"
		L"読み込まれた文字列が解析対象となり、指標を移動させながら構文を解釈します。\n"
		L"　解析対象の文字列は、(String) キャストか toString 関数によって取得できます。" },
	{ L"UsageMatcher",
		L"<summary>　EntisGLS4 の SUsageMatcher とほぼ同等です。<br/>\n"
		L"<div class=\"code_quote\">\n"
		L"String[]	param = new String[] ;<br/>"
		L"UsageMatcher usage = new UsageMatcher( \"(%t) is (%t)\" ) ;<br/>\n"
		L"if ( usage.parse( \"apple is red\", param ) == null )<br/>\n"
		L"{<br/>\n"
		L"&nbsp; &nbsp; System.console().printf( \"%s, %s\n\", param[0], param[1] ) ; // apple, red<br/>\n"
		L"}<br/>\n"
		L"</div></summary>" },
	{ L"CRC32Context",
		L"　CRC32Context は CRC32 を計算するクラスです。" },
	{ L"MD5DigestContext",
		L"　MD5DigestContext は MD5 ダイジェストを計算するクラスです。" },
	{ L"Randomizer",
		L"　Randomizer は擬似乱数を発生するクラスです。\n"
		L"　quickInt、quickFloat 関数は、initSeed で設定された同一の初期値に対し、"
		L"同じ数列を出力します。randomize 関数は同じ数列を出力するとは限りません。\n"
		L"　Randomizer は initSeed 関数を呼び出さない場合、ランダムな初期値で"
		L"初期化されています。" },
	{ L"Window",
		L"　Window は抽象ウィンドウへのインターフェースです。" },
	{ L"Dialog",
		L"　Dialog はダイアログウィンドウを提供するクラスです。" },
	{ L"NativeObject",
		L"　ネイティブなオブジェクトコンテナの基底クラスです。\n"
		L"　同一オブジェクトは === 演算子では必ずしも判定できません。\n"
		L"　equals 関数で実際のネイティブオブジェクトが同一か判定できます。"
		L"あるいは getNativePointer の値を比較します。\n"
		L"　NativeObject はネイティブにはウィークポインタのように振舞います（EntisGLS4 の参照型）。\n"
		L"　ネイティブオブジェクトの実体が破棄された場合 equals(null) が true を評価します。" },
	{ L"Point",
		L"　Point は座標情報を格納する構造体です。\n"
		L"　EntisGLS4 の SGLPoint と同等です。" },
	{ L"Size",
		L"　Size はサイズ情報を格納する構造体です。\n"
		L"　EntisGLS4 の SGLSize と同等です。" },
	{ L"Rect",
		L"　Rect は矩形情報を格納する構造体です。\n"
		L"　EntisGLS4 の SGLImageRect と同等です。" },
	{ L"RGBColor",
		L"　RGBColor は RGBA を格納する構造体です（αを含んでいます）。\n"
		L"　EntisGLS4 の SGLPalette と似ています。" },
	{ L"Color3D",
		L"　Color3D は3Dグラフィックス処理で扱う色を表現します。\n"
		L"　EntisGLS4 の S3DColor と同等で、乗算成分と加算成分を保持しています。\n"
		L"　α成分は利用しませんが、頂点バッファでは乗算成分のαチャネルは"
		L"頂点のα成分として利用されます。" },
	{ L"Vector2D",
		L"<summary>　Vector2D は座標情報を格納する構造体です。<br/>\n"
		L"　EntisGLS4 の S2DVector と同等です。<br/>\n"
		L"　add, sub, mul, div, normalize は const 関数ではないことに注意してください。<br/>"
		L"　副作用なく、(a + b - c) の結果を d に受け取る場合には"
		L" clone 関数を使い以下のように記述します。<br/>\n"
		L"<div class=\"code_quote\">\n"
		L"Vector2D d = a.clone().add(b).sub(c) ;\n"
		L"</div></summary>" },
	{ L"Vector3D",
		L"<summary>　Vector3D は座標情報を格納する構造体です。<br/>\n"
		L"　EntisGLS4 の S3DVector と同等です。<br/>\n"
		L"　add, sub, mul, div, normalize は const 関数ではないことに注意してください。<br/>"
		L"　副作用なく、(a + b - c) の結果を d に受け取る場合には"
		L" clone 関数を使い以下のように記述します。<br/>\n"
		L"<div class=\"code_quote\">\n"
		L"Vector3D d = a.clone().add(b).sub(c) ;\n"
		L"</div></summary>" },
	{ L"Vector3D4",
		L"<summary>　Vector3D4 は座標情報を格納する構造体です。<br/>\n"
		L"　EntisGLS4 の S3DVector4 や S4DVector と同等です"
		L"（EntisGLS4 では S3DVector4 と S4DVector は使い分けられていますが、Rosetta では共用されています）。<br/>\n"
		L"　Vector3D から派生しており、w 要素が必ず使用されるわけではありません。"
		L"absolute や normalize は3次元ベクトルの関数として機能することに注意してください。<br/></summary>" },
	{ L"Quaternion",
		L"<summary>　Quaternion は四元数を格納する構造体です。<br/>\n"
		L"　EntisGLS4 の S3DQuaternion と同等です。<br/>\n"
		L"　add, sub, mul, div, normalize 等の関数は const 関数ではないことに注意してください。<br/>"
		L"　副作用なく、normalize(a + b - c) の結果を d に受け取る場合には"
		L" clone 関数を使い以下のように記述します。<br/>\n"
		L"<div class=\"code_quote\">\n"
		L"Quaternion d = a.clone().add(b).sub(c).normalize() ;\n"
		L"</div>\n"
		L"　Matrix3D からは fromMatrix、Matrix3D への変換は toMatrix 関数で行えます。</summary>" },
	{ L"Affine",
		L"　Affine はアフィン行列を格納する構造体です。\n"
		L"　EntisGLS4 の SGLAffine と同等です。" },
	{ L"Matrix3D",
		L"　Matrix3D は3次元行列を格納する構造体です。\n"
		L"　EntisGLS4 の S3DMatrix とほぼ同等です。" },
	{ L"Matrix4D",
		L"　Matrix4D は4次元行列を格納する構造体です。\n"
		L"　EntisGLS4 の S4DMatrix とほぼ同等です。" },
	{ L"FontStyle",
		L"　FontStyle はフォント情報を格納するクラスです。\n"
		L"　EntisGLS4 の SGLFontStyle と同等です。" },
	{ L"LetteringContext",
		L"　LetteringContext はレタリング情報を格納するクラスです。\n"
		L"　EntisGLS4 の SGLLetteringContext と同等です。" },
	{ L"LetteringDecoration",
		L"　LetteringDecoration は文字装飾情報を格納するクラスです。\n"
		L"　EntisGLS4 の SGLLetterer::Decoration と同等です。" },
	{ L"Image",
		L"　Image クラスは画像データを保持します。\n"
		L"　EntisGLS4 の SGLImage と同等です。\n"
		L"　Image.BufferInfo 構造体は SGLImageInfo 構造体に相当します。" },
	{ L"ImageComposition",
		L"　ImageComposition クラスは複数の画像データから構成されたデータを保持します。\n"
		L"　EntisGLS4 の SGLImageComposition と同等です。\n"
		L"　主に PSD ファイルを読み込んで処理するために利用されます。" },
	{ L"PaintContext",
		L"　PaintContext クラスは描画オブジェクトを保持します。\n"
		L"　EntisGLS4 の SGLPaintContext と同等です。\n"
		L"　PaintContext.PaintParam クラスは SGLPaintParam 構造体に相当します。" },
	{ L"SoundPlayer",
		L"　SoundPlayer は音声出力機能を供給します。\n"
		L"　EntisGLS4 の SGLSoundPlayer コンテナです。\n"
		L"　SoundPlayer.Format クラスは SGLSoundFormat 構造体に、"
		L"SoundPlayer.Listener クラスは SGLSoundPlayerListener クラスに相当します。" },
	{ L"AudioPlayer",
		L"　AudioPlayer はオーディオファイル再生機能を供給します。\n"
		L"　EntisGLS4 の SGLAudioPlayer コンテナです。" },
	{ L"MediaOptionalInfo",
		L"　MediaOptionalInfo はメディアファイルの付属情報を保持します。" },
	{ L"AudioInputStream",
		L"　AudioInputStream はオーディオファイルのデコーダーインターフェースを供給します。\n"
		L"　EntisGLS4 の SGLAudioInputStream コンテナです。" },
	{ L"AudioOutputStream",
		L"　AudioOutputStream はオーディオファイルのエンコーダーインターフェースを供給します。\n"
		L"　EntisGLS4 の SGLAudioOutputStream コンテナです。" },
	{ L"VideoInputStream",
		L"　VideoInputStream はビデオファイルのデコーダーインターフェースを供給します。\n"
		L"　EntisGLS4 の SGLVideoInputStream コンテナです。" },
	{ L"VideoOutputStream",
		L"　VideoOutputStream はビデオファイルのエンコーダーインターフェースを供給します。\n"
		L"　EntisGLS4 の SGLVideoOutputStream コンテナです。" },
	{ L"SkinManager",
		L"　SkinManager クラスは EntisGLS3 互換のスキンデータ"
		L"（UI外観定義及び配置データ）を保持します。\n"
		L"　EntisGLS4 の SGLSkinManager と同等です。" },
	{ L"BasicFormParser",
		L"　BasicFormParser クラスは EntisGLS4 の簡易 UI 外観定義データを"
		L"読み込み／保持します。\n"
		L"　EntisGLS4 の SGLBasicFormParser と同等です。\n"
		L"　EntisGLS3 互換の UI データを扱う SkinManager より GPU 描画の最適化に適しています。" },
	{ L"Sprite",
		L"　Sprite は画面表示インターフェースの基底クラスです。\n"
		L"　EntisGLS4 の SGLSprite と同等です。" },
	{ L"SpriteAction",
		L"　SpriteAction は Sprite パラメータアニメーションを Sprite に供給します。\n"
		L"　EntisGLS4 の SGLSpriteAction と同等です。" },
	{ L"SpriteKeyListener",
		L"　SpriteKeyListener は Sprite へのキー入力インターフェースです。\n"
		L"　EntisGLS4 の SGLSpriteKeyListener と同等です。\n"
		L"　また、SpriteKeyListener.StartComposition クラスは "
		L"SGLInputStartComposition 構造体に、SpriteKeyListener.CompositionString "
		L"クラスは SGLInputCompositionString 構造体に相当します。" },
	{ L"SpriteMouseListener",
		L"　SpriteMouseListener は Sprite へのマウス入力インターフェースです。\n"
		L"　EntisGLS4 の SGLSpriteMouseStateListener と同等です。" },
	{ L"SpriteTimer",
		L"　SpriteTimer は Sprite へのタイマーインターフェースです。\n"
		L"　EntisGLS4 の SGLSpriteTimer と同等です。" },
	{ L"WindowSprite",
		L"　WindowSprite はウィンドウ表示インターフェースを Sprite に供給します。\n"
		L"　EntisGLS4 の SGLWindowSprite コンテナです。" },
	{ L"VirtualInput",
		L"　VirtualInput はウィンドウへの入力を仮想化するインターフェースです。\n"
		L"　EntisGLS4 の SGLVirtualInput コンテナです。" },
	{ L"RectangleSprite",
		L"　RectangleSprite は矩形を表示します。\n"
		L"　EntisGLS4 の SGLSpriteRectangle コンテナです。" },
	{ L"TextSprite",
		L"　TextSprite は文字列を表示します。\n"
		L"　EntisGLS4 の SGLSpriteText コンテナです。" },
	{ L"ButtonSprite",
		L"　ButtonSprite はボタンUIを供給します。\n"
		L"　EntisGLS4 の SGLSpriteButton コンテナです。" },
	{ L"MessageSprite",
		L"　MessageSprite は文字列を順次表示します。\n"
		L"　EntisGLS4 の SGLSpriteMessage コンテナです。" },
	{ L"MovieSprite",
		L"　MovieSprite は動画を再生表示できます。\n"
		L"　EntisGLS4 の SGLSpriteMovie コンテナです。" },
	{ L"RenderableSprite",
		L"　RenderableSprite は Rosetta スクリプト上で描画関数を"
		L"オーバーライド可能な Sprite です。\n"
		L"　RenderableSprite コンストラクタはオーバーライド可能な "
		L"Sprite の実体を生成しますので、派生クラスでコンストラクタを"
		L"記述する際には、super() の呼び出しを忘れないように注意が必要です。" },
	{ L"TextureLibrary",
		L"　TextureLibrary は3Dグラフィックス処理で扱うテクスチャ画像を集積します。\n"
		L"　EntisGLS4 の S3DTextureLibrary コンテナです。" },
	{ L"SurfaceAttribute",
		L"　SurfaceAttribute は3Dグラフィックス処理で扱うマテリアルの表面属性を表現します。\n"
		L"　EntisGLS4 の S3DSurfaceAttribute と同等です。" },
	{ L"Material",
		L"　Material は3Dグラフィックス処理で扱うマテリアルを表現します。\n"
		L"　EntisGLS4 の S3DMaterial コンテナです。" },
	{ L"MaterialLibrary",
		L"　MaterialLibrary は3Dグラフィックス処理で扱うマテリアルを集積します。\n"
		L"　EntisGLS4 の S3DMaterialLibrary コンテナです。" },
	{ L"RenderBuffer",
		L"　RenderBuffer は3Dグラフィックス処理のレンダリングの抽象対象物を表現します。\n"
		L"　EntisGLS4 の S3DRenderBufferInterface コンテナです。" },
	{ L"VertexVariantBuffer",
		L"　VertexVariantBuffer は3Dグラフィックス処理で扱う変形モデルパラメータの抽象物です。\n"
		L"　EntisGLS4 の S3DVertexVariantBuffer コンテナです。" },
	{ L"VertexBuffer",
		L"　VertexBuffer は3Dグラフィックス処理で扱う頂点バッファ抽象物です。\n"
		L"　EntisGLS4 の S3DVertexBufferInterface コンテナです。" },
	{ L"ModelBuffer",
		L"　ModelBuffer は3Dグラフィックス処理で扱うモデルファイルを扱います。\n"
		L"　EntisGLS4 の S3DModelBuffer コンテナです。" },
	{ L"CustomShader",
		L"　CustomShader はシェーダー抽象物です。\n"
		L"　EntisGLS4 の S3DCustomShader コンテナです。" },
	{ L"RenderDevice",
		L"　RenderDevice は3Dレンダリングデバイスの抽象物です。\n"
		L"　EntisGLS4 の S3DRenderDevice コンテナです。" },
	{ L"RenderContext",
		L"　RenderContext は3Dレンダリングコンテキスト抽象物です。\n"
		L"　EntisGLS4 の S3DRenderContextInterface コンテナです。\n"
		L"　PaintContext, RenderBuffer を継承しており、描画メソッドはそれらを利用できます。\n"
		L"　RenderContext では主にレンダリング設定（カメラ・光源等）を"
		L"行うメソッドが追加されています。" },
	{ NULL, NULL },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVirtualMachine, RSNamespace )

const RSVirtualMachine::NativeFuncDescriptor *	RSVirtualMachine::s_pnfdFirstDesc = NULL ;


// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVirtualMachine::RSVirtualMachine( void )
{
	m_typeObj = typeOther ;
	//
	m_pSakura2VM = NULL ;
	m_pContext = NULL ;
	m_pMacroCtx = NULL ;
	m_pSysThread = NULL ;
	//
	m_pRunningThreads = NULL ;
	m_countRunningThreads = 0 ;
	//
	m_pMetaClass = NULL ;
	m_pVarClass = NULL ;
	m_pFuncClass = NULL ;
	m_pBooleanClass = NULL ;
	m_pIntegerClass = NULL ;
	m_pNumberClass = NULL ;
	m_pStringClass = NULL ;
	m_pArrayClass = NULL ;
	m_pExceptionClass = NULL ;
	m_pJObjectClass = NULL ;
	m_pNObjectClass = NULL ;
	m_pJSObjectClass = NULL ;
	m_pStructureClass = NULL ;
	//
	for ( size_t i = 0; i < RSCodeControl::wiBasicTypeCount; i ++ )
	{
		m_pBasicTypeClass[i] = NULL ;
	}
	for ( size_t i = 0; i < RSReferenceNumber::typeCountOfNumber; i ++ )
	{
		m_pPtrTypeClass[i] = NULL ;
	}
	//
	m_countRunning = 0 ;
	//
	m_pStdInput = &cin ;
	m_pStdOutput = &cout ;
	//
	m_flagRefVM = false ;
	m_flagDebug = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSVirtualMachine::~RSVirtualMachine( void )
{
	RSVirtualMachine::Release() ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::Initialize( uint32_t nInitClasses )
{
	m_flagRefVM = false ;
	//
	// 基本クラス設定
	//
	m_pMetaClass = new RSClassClass( NULL, L"Class" ) ;
	m_pVarClass = new RSAbstractPointerClass( m_pMetaClass, L"var" ) ;
	m_pBooleanClass = new RSBooleanClass( m_pMetaClass ) ;
	m_pIntegerClass =
				new RSIntegerClass
					( m_pMetaClass, L"Integer", RSInteger::typeInt64 ) ;
	m_pNumberClass = new RSNumberClass( m_pMetaClass ) ;
	m_pStringClass = new RSStringClass( m_pMetaClass ) ;
	m_pArrayClass = new RSArrayClass( m_pMetaClass, L"Array" ) ;
	m_pArrayBufferClass = new RSArrayBufferClass( m_pMetaClass, L"ArrayBuffer" ) ;
	m_pExceptionClass = new RSExceptionClass( m_pMetaClass, L"Exception" ) ;
	m_pJObjectClass = new RSGenricObjectClass( m_pMetaClass, L"Object" ) ;
	m_pNObjectClass = new RNativeObjectClass( m_pMetaClass, L"NativeObject" ) ;
	m_pJSObjectClass = new RSDynamicObjectClass( m_pMetaClass, L"HashMap" ) ;
	m_pStructureClass = new RSStructureClass( m_pMetaClass, L"Structure" ) ;
	//
	m_pBasicTypeClass[RSCodeControl::wiBoolean - RSCodeControl::wiFirstBasicType]
		= m_pBooleanClass ;
	m_pBasicTypeClass[RSCodeControl::wiByte - RSCodeControl::wiFirstBasicType]
		= new RSIntegerClass
				( m_pMetaClass, L"byte", RSInteger::typeInt8 ) ;
	m_pBasicTypeClass[RSCodeControl::wiShort - RSCodeControl::wiFirstBasicType]
		= new RSIntegerClass
				( m_pMetaClass, L"short", RSInteger::typeInt16 ) ;
	m_pBasicTypeClass[RSCodeControl::wiChar - RSCodeControl::wiFirstBasicType]
		= new RSIntegerClass
				( m_pMetaClass, L"char", RSInteger::typeUint16 ) ;
	m_pBasicTypeClass[RSCodeControl::wiInt - RSCodeControl::wiFirstBasicType]
		= new RSIntegerClass
				( m_pMetaClass, L"int", RSInteger::typeInt32 ) ;
	m_pBasicTypeClass[RSCodeControl::wiLong - RSCodeControl::wiFirstBasicType]
		= new RSIntegerClass
				( m_pMetaClass, L"long", RSInteger::typeInt64 ) ;
	m_pBasicTypeClass[RSCodeControl::wiFloat - RSCodeControl::wiFirstBasicType]
		= new RSNumberClass( m_pMetaClass, L"float" ) ;
	m_pBasicTypeClass[RSCodeControl::wiDouble - RSCodeControl::wiFirstBasicType]
		= new RSNumberClass( m_pMetaClass, L"double" ) ;
	//
	m_pPtrTypeClass[RSReferenceNumber::typeUint8]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Uint8Pointer", RSReferenceNumber::typeUint8 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeInt8]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Int8Pointer", RSReferenceNumber::typeInt8 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeUint16]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Uint16Pointer", RSReferenceNumber::typeUint16 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeInt16]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Int16Pointer", RSReferenceNumber::typeInt16 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeUint32]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Uint32Pointer", RSReferenceNumber::typeUint32 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeInt32]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Int32Pointer", RSReferenceNumber::typeInt32 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeInt64]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Int64Pointer", RSReferenceNumber::typeInt64 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeFloat32]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Float32Pointer", RSReferenceNumber::typeFloat32 ) ;
	m_pPtrTypeClass[RSReferenceNumber::typeFloat64]
		= new RSTypedArrayPointerClass
				( m_pMetaClass, L"Float64Pointer", RSReferenceNumber::typeFloat64 ) ;
	//
	RSContext	context( this ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Class", m_pMetaClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"var", m_pVarClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"boolean", m_pBooleanClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Integer", m_pIntegerClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Number", m_pNumberClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"String", m_pStringClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Array", m_pArrayClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"ArrayBuffer", m_pArrayBufferClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Exception", m_pExceptionClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Object", m_pJObjectClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"NativeObject", m_pNObjectClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"HashMap", m_pJSObjectClass ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs( context, L"Structure", m_pStructureClass ) ) ;
	//
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"byte",
				m_pBasicTypeClass
				[RSCodeControl::wiByte - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"short",
				m_pBasicTypeClass
				[RSCodeControl::wiShort - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"char",
				m_pBasicTypeClass
				[RSCodeControl::wiChar - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"int",
				m_pBasicTypeClass
				[RSCodeControl::wiInt - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"long",
				m_pBasicTypeClass
				[RSCodeControl::wiLong - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"float",
				m_pBasicTypeClass
				[RSCodeControl::wiFloat - RSCodeControl::wiFirstBasicType] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"double",
				m_pBasicTypeClass
				[RSCodeControl::wiDouble - RSCodeControl::wiFirstBasicType] ) ) ;
	//
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Uint8Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeUint8] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Int8Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeInt8] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Uint16Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeUint16] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Int16Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeInt16] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Uint32Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeUint32] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Int32Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeInt32] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Int64Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeInt64] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Float32Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeFloat32] ) ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, L"Float64Pointer",
				m_pPtrTypeClass[RSReferenceNumber::typeFloat64] ) ) ;
	//
	m_pFuncClass = new RSFunctionClass( m_pMetaClass, L"Function" ) ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( *m_pContext, L"Function", m_pFuncClass ) ) ;
	//
	// システムコンテキスト設定
	//
	ESLAssert( m_pContext == NULL ) ;
	ESLAssert( m_pMacroCtx == NULL ) ;
	m_pContext = new RSContext( this ) ;
	m_pMacroCtx = new RSContext( this ) ;
	m_pMacroCtx->PushNamespace
			( NULL, new RSNamespace, RSObject::modifierPublic, true ) ;
	//
	// クラスの初期設定
	//
	m_pJObjectClass->Initialize( *m_pContext ) ;
	m_pJObjectClass->FinishClass( *m_pContext ) ;
	//
	m_pMetaClass->AddSuperClass( *m_pContext, m_pJObjectClass ) ;
	m_pMetaClass->Initialize( *m_pContext ) ;
	m_pMetaClass->FinishClass( *m_pContext ) ;
	//
	m_pPtrTypeClass[RSReferenceNumber::typeUint8]->Initialize( *m_pContext ) ;
	//
	for ( size_t i = 0; i < m_gcmGenClasses.GetMemberCount(); i ++ )
	{
		RSObject *	pObj = m_gcmGenClasses.GetMemberAt( i ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
		if ( (pClass != NULL)
			&& (pClass != m_pMetaClass)
			&& (pClass != m_pJObjectClass)
			&& (pClass != m_pFuncClass)
			&& (pClass != m_pPtrTypeClass[RSReferenceNumber::typeUint8]) )
		{
			pClass->Initialize( *m_pContext ) ;
			pClass->FinishClass( *m_pContext ) ;
		}
		RSObject::ReleaseRef( pObj ) ;
	}
	m_pFuncClass->Initialize( *m_pContext ) ;
	m_pFuncClass->FinishClass( *m_pContext ) ;
	//
	// 拡張クラス設定
	//
	RSRenderDeviceClass *	pRenderDeviceClass = new RSRenderDeviceClass( m_pMetaClass ) ;
	if ( nInitClasses & classStdAppend )
	{
		RegisterNewClass( new RSStringParserClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSUsageMatcherClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSWindowClass( m_pMetaClass ) ) ;
		RegisterNewClass( pRenderDeviceClass, true ) ;
	}
	if ( nInitClasses & classStandard )
	{
		RegisterNewClass( new RSSystemClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSConsoleClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMathClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSDateClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSFileClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSInputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSOutputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRandomAccessFileClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRunnableClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSThreadClass( m_pMetaClass ) ) ;
	}
	if ( nInitClasses & classCrypt )
	{
		RegisterNewClass( new RSCRC32ContextClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMD5DigestContextClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRandomizerClass( m_pMetaClass ) ) ;
	}
	ImplementNewClasses() ;
	//
	if ( nInitClasses & classStdAppend )
	{
		RegisterNewClass( new RSSmartBufferFileClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSNoaFileArchiverClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSHttpInputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSDialogClass( m_pMetaClass ) ) ;
	}
	if ( nInitClasses & classCrypt )
	{
		RegisterNewClass( new RSEncrypt32OutputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSDecrypt32InputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSEncryptRSA96OutputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSDecryptRSA96InputStreamClass( m_pMetaClass ) ) ;
	}
	if ( nInitClasses & classPaint2D )
	{
		RegisterNewClass( new RSPointClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSizeClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRectClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRGBColorClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVector2DClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSAffineClass( m_pMetaClass ) ) ;
		//
		RegisterNewClass( new RSVector3DClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSColor3DClass( m_pMetaClass ) ) ;
		//
		RegisterNewClass( new RSFontStyleClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSLetteringContextClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSLetteringDecorationClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSImageClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSPaintContextClass( m_pMetaClass ) ) ;
		//
		if ( nInitClasses & classSprite )
		{
			RegisterNewClass( new RSSpriteClass( m_pMetaClass ) ) ;
			RegisterNewClass( new RSSpriteTimerClass( m_pMetaClass ) ) ;
			RegisterNewClass( new RSSpriteMouseListenerClass( m_pMetaClass ) ) ;
			RegisterNewClass( new RSSpriteKeyListenerClass( m_pMetaClass ) ) ;
			RegisterNewClass( new RSSpriteActionClass( m_pMetaClass ) ) ;
		}
	}
	if ( nInitClasses & classMedia )
	{
		RegisterNewClass( new RSSoundPlayerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSAudioPlayerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMediaOptionalInfoClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSAudioInputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSAudioOutputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVideoInputStreamClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVideoOutputStreamClass( m_pMetaClass ) ) ;
	}
	ImplementNewClasses() ;
	//
	if ( (nInitClasses & (classPaint2D | classRender3D))
							== (classPaint2D | classRender3D) )
	{
		RegisterNewClass( new RSVector3D4Class( m_pMetaClass ) ) ;
		RegisterNewClass( new RSQuaternionClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMatrix3DClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMatrix4DClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSTextureLibraryClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSurfaceAttributeClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMaterialClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMaterialLibraryClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSModelPoseClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSModelPoseLibraryClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSCustomShaderClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRenderBufferClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVertexVariantBufferClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVertexBufferClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSModelBufferClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRenderContextClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRenderableSpriteClass( m_pMetaClass ) ) ;
	}
	if ( (nInitClasses & classStdAppend)
		|| ((nInitClasses & (classPaint2D | classRender3D))
							== (classPaint2D | classRender3D)) )
	{
		AddImplementClass( pRenderDeviceClass ) ;
	}
	if ( (nInitClasses & (classPaint2D | classSprite))
							== (classPaint2D | classSprite) )
	{
		RegisterNewClass( new RSImageCompositionClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSWindowSpriteClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSRectangleSpriteClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSTextSpriteClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSButtonSpriteClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSVirtualInputClass( m_pMetaClass ) ) ;
	}
	ImplementNewClasses() ;
	//
	if ( (nInitClasses & (classPaint2D | classSprite))
							== (classPaint2D | classSprite) )
	{
		RegisterNewClass( new RSSkinManagerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSBasicFormParserClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMessageSpriteClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSMovieSpriteClass( m_pMetaClass ) ) ;
	}
	ImplementNewClasses() ;
	//
	if ( nInitClasses & classScene3D )
	{
		RegisterNewClass( new RSSceneClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneManagerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneComposerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneParameterClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneSequencerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSScenePropertyClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneControllerClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneItemClass( m_pMetaClass ) ) ;
		RegisterNewClass( new RSSceneCompositionClass( m_pMetaClass ) ) ;
	}
	ImplementNewClasses() ;
	//
	GetExceptioinClassAs( L"ArrayIndexOutOfBoundsException" ) ;
	GetExceptioinClassAs( L"IndexOutOfBoundsException" ) ;
	GetExceptioinClassAs( L"NullPointerException" ) ;
	GetExceptioinClassAs( L"FileNotFoundException" ) ;
	GetExceptioinClassAs( L"IOException" ) ;
	GetExceptioinClassAs( L"IllegalArgumentException" ) ;
	GetExceptioinClassAs( L"IllegalMonitorStateException" ) ;
	//
	// クラスのドキュメント用コメント
	//
	for ( int i = 0; s_pwszClassDoc[i][0] != NULL; i ++ )
	{
		RSClass *	pClass = GetClassAs( s_pwszClassDoc[i][0] ) ;
		if ( pClass != NULL )
		{
			pClass->SetDefinitionComment
				( pClass->ImmediateComment( s_pwszClassDoc[i][1] ) ) ;
		}
	}
	//
	// ネイティブ関数
	//
	RegisterNativeMethodAllDescriptors() ;
	//
	// クラス以外の初期化
	//
	InitializeVMContext() ;
}

// システム定義クラスをインポートして初期化する
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::InitializeRefVM( RSVirtualMachine& vmProto )
{
	m_flagRefVM = true ;
	//
	// 基本クラス設定
	//
	m_pMetaClass = vmProto.m_pMetaClass ;
	m_pVarClass = vmProto.m_pMetaClass ;
	m_pFuncClass = vmProto.m_pFuncClass ;
	m_pBooleanClass = vmProto.m_pBooleanClass ;
	m_pIntegerClass = vmProto.m_pIntegerClass ;
	m_pNumberClass = vmProto.m_pNumberClass ;
	m_pStringClass = vmProto.m_pStringClass ;
	m_pArrayClass = vmProto.m_pArrayClass ;
	m_pArrayBufferClass = vmProto.m_pArrayBufferClass ;
	m_pExceptionClass = vmProto.m_pExceptionClass ;
	m_pJObjectClass = vmProto.m_pJObjectClass ;
	m_pNObjectClass = vmProto.m_pNObjectClass ;
	m_pJSObjectClass = vmProto.m_pJSObjectClass ;
	m_pStructureClass = vmProto.m_pStructureClass ;
	//
	for ( int i = 0; i < RSCodeControl::wiBasicTypeCount; i ++ )
	{
		m_pBasicTypeClass[i] = vmProto.m_pBasicTypeClass[i] ;
	}
	for ( int i = 0; i < RSReferenceNumber::typeCountOfNumber; i ++ )
	{
		m_pPtrTypeClass[i] = vmProto.m_pPtrTypeClass[i] ;
	}
	//
	// クラス参照
	//
	RSContext	context( this ) ;
	size_t	nCount = vmProto.m_gcmGenClasses.GetMemberCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const wchar_t *
			pwszClassName = vmProto.m_gcmGenClasses.GetMemberNameAt( i ) ;
		RSObject *	pObjClass = vmProto.m_gcmGenClasses.GetMemberAt( i ) ;
		ESLAssert( pwszClassName != NULL ) ;
		ESLAssert( pObjClass != NULL ) ;
		if ( pwszClassName && pObjClass )
		{
			RSObject::ReleaseRef
				( m_gcmGenClasses.SetMemberAs
					( context, pwszClassName, pObjClass ) ) ;
		}
	}
	//
	// クラス以外の初期化
	//
	ESLAssert( m_pContext == NULL ) ;
	ESLAssert( m_pMacroCtx == NULL ) ;
	m_pContext = new RSContext( this ) ;
	m_pMacroCtx = new RSContext( this ) ;
	//
	InitializeVMContext() ;
	//
	// ネイティブ関数の継承
	//
	m_ssaNativeFuncs = vmProto.m_ssaNativeFuncs ;
}

// クラス以外の状態を初期化する
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::InitializeVMContext( void )
{
	//
	// 同期オブジェクト
	//
	m_countRunning = 0 ;
	m_mutexAssertLock.Initialize() ;
	m_sigAllLeaved.Initialize( true ) ;
	//
	// システムスレッド設定
	//
	m_pSysThread = new RSThread( this, NULL, GetClassAs( L"Thread" ) ) ;
	m_pContext->AttachThreadObject( m_pSysThread ) ;
	m_pMacroCtx->AttachThreadObject( m_pSysThread ) ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::Release( void )
{
	AbortAllThreads() ;

	if ( m_pContext != NULL )
	{
		SPointerArray<RSClass>	lstClasses ;
		for ( size_t i = 0; i < GetElementCount(); i ++ )
		{
			RSObject *	pObj = GetElementAt( *m_pContext, (int) i ) ;
			RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
			if ( pClass != NULL )
			{
				lstClasses.Add( pClass ) ;
			}
			else
			{
				RSObject::ReleaseRef( pObj ) ;
			}
		}
		if ( !m_flagRefVM )
		{
			for ( size_t i = 0; i < m_gcmGenClasses.GetMemberCount(); i ++ )
			{
				RSObject *	pObj = m_gcmGenClasses.GetMemberAt( i ) ;
				RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
				if ( pClass != NULL )
				{
					lstClasses.Add( pClass ) ;
				}
				else
				{
					RSObject::ReleaseRef( pObj ) ;
				}
			}
		}
		DisposeObject( *m_pContext ) ;
		//
		if ( !m_flagRefVM )
		{
			m_gcmGenClasses.DisposeAllMembers( *m_pContext ) ;
		}
		else
		{
			m_gcmGenClasses.RemoveAllMembers() ;
		}
		//
		delete	m_pContext ;
		delete	m_pMacroCtx ;
		m_pContext = NULL ;
		m_pMacroCtx = NULL ;
		//
		for ( size_t i = 0; i < lstClasses.GetLength(); i ++ )
		{
			RSClass *	pClass = lstClasses.GetAt(i) ;
			pClass->ReleaseReferenceChain() ;
			pClass->ReleaseRef() ;
		}
		//
		m_pMetaClass = NULL ;
		m_pVarClass = NULL ;
		m_pBooleanClass = NULL ;
		m_pIntegerClass = NULL ;
		m_pNumberClass = NULL ;
		m_pStringClass = NULL ;
		m_pArrayClass = NULL ;
		m_pExceptionClass = NULL ;
		m_pJObjectClass = NULL ;
		m_pNObjectClass = NULL ;
		m_pJSObjectClass = NULL ;
		//
		for ( size_t i = 0; i < RSCodeControl::wiBasicTypeCount; i ++ )
		{
			m_pBasicTypeClass[i] = NULL ;
		}
		//
		m_pSysThread->ReleaseRef() ;
		m_pSysThread = NULL ;
		//
		m_countRunning = 0 ;
		m_mutexAssertLock.Delete() ;
		m_sigAllLeaved.Delete() ;
		//
		if ( m_pSakura2VM != NULL )
		{
			m_pSakura2VM->AttachExceptionHandler( NULL ) ;
			//
			for ( size_t i = 0; i < m_aModules.GetLength(); i ++ )
			{
				ECSSakura2::ExecutableModule *
							pxmModule = m_aModules.GetAt( i ) ;
				if ( pxmModule != NULL )
				{
					m_pSakura2VM->FreeModuleAllocation( pxmModule ) ;
				}
			}
		}
		m_aModules.RemoveAll() ;
		m_pSakura2VM = NULL ;
	}
}

// Sakura2 仮想マシン関連付け
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AttachSakura2VM( ECSSakura2::StandardVM * pVM )
{
	if ( m_pSakura2VM != NULL )
	{
		m_pSakura2VM->AttachExceptionHandler( NULL ) ;
	}
	m_pSakura2VM = pVM ;
	pVM->AttachExceptionHandler( this ) ;
}

// スクリプトパスの追加
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AddScriptPath( const wchar_t * pwszPath )
{
	m_arrScriptPath.Add( new SString( pwszPath ) ) ;
}

// 環境変数からスクリプトパスを追加
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AddScriptEnvironmentPath( const wchar_t * pwszEnvName )
{
	SString	strEnvIncludePath ;
	if ( SGLStdApplication::GetEnvironmentVariable
					( pwszEnvName, strEnvIncludePath ) != NULL )
	{
		SStringParser	sparsPath ;
		sparsPath.AttachString( strEnvIncludePath ) ;
		while ( !sparsPath.IsIndexOverflow() )
		{
			SString	strPath ;
			sparsPath.NextEnclosedString( strPath, L';' ) ;
			if ( !strPath.IsEmpty() )
			{
				AddScriptPath( strPath ) ;
			}
		}
	}
}

// スクリプトファイル・オープナーを追加
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AddScriptFileOpener( SSystem::SFileOpener * pOpener )
{
	if ( m_arrFileOpener.FindPtr( pOpener ) < 0 )
	{
		m_arrFileOpener.Add( pOpener ) ;
	}
}

// スクリプトファイル・オープナーを削除
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::DetachScriptFileOpener( SSystem::SFileOpener * pOpener )
{
	ssize_t	i = m_arrFileOpener.FindPtr( pOpener ) ;
	if ( i >= 0 )
	{
		m_arrFileOpener.RemoveAt( (size_t) i ) ;
	}
}

// 標準入出力関連付け
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AttachStandardInput( SSystem::SBufferedFile * pStdIn )
{
	ESLAssert( pStdIn != NULL ) ;
	m_pStdInput = pStdIn ;
}

void RSVirtualMachine::AttachStandardOutput( SSystem::SBufferedFile * pStdOut )
{
	ESLAssert( pStdOut != NULL ) ;
	m_pStdOutput = pStdOut ;
}

// 標準入出力取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SBufferedFile & RSVirtualMachine::GetStandardInput( void ) const
{
	ESLAssert( m_pStdInput != NULL ) ;
	return	*m_pStdInput ;
}

SSystem::SBufferedFile & RSVirtualMachine::GetStandardOutput( void ) const
{
	ESLAssert( m_pStdOutput != NULL ) ;
	return	*m_pStdOutput ;
}

// デバッグフラグ
//////////////////////////////////////////////////////////////////////////////
bool RSVirtualMachine::IsDebug( void ) const
{
	return	m_flagDebug ;
}

void RSVirtualMachine::SetDebug( bool flagDebug )
{
	m_flagDebug = flagDebug ;
}

// スクリプトファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	RSVirtualMachine::OpenScriptFile( const wchar_t * pwszFilePath ) const
{
	for ( size_t i = 0; i < m_arrFileOpener.GetLength(); i ++ )
	{
		SFileOpener *	pOpener = m_arrFileOpener.GetAt( i ) ;
		if ( pOpener != NULL )
		{
			SFileInterface *	pFile =
				pOpener->NewOpenFile
					( pwszFilePath, SFileOpener::shareRead ) ;
			if ( pFile != NULL )
			{
				return	pFile ;
			}
		}
	}
	SFileInterface *
		pFile = SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		for ( size_t i = 0; i < m_arrScriptPath.GetLength(); i ++ )
		{
			SString *	pstrPath = m_arrScriptPath.GetAt( i ) ;
			ESLAssert( pstrPath != NULL ) ;
			pFile = SFileOpener::DefaultNewOpenFile
						( pstrPath->OffsetFilePath( pwszFilePath ),
												SFileOpener::shareRead ) ;
			if ( pFile != NULL )
			{
				break ;
			}
		}
	}
	return	pFile ;
}

// スクリプト読み込み
//////////////////////////////////////////////////////////////////////////////
RSScript * RSVirtualMachine::LoadScript
	( const wchar_t * pwszFilePath,
		SSystem::SParserErrorInterface& perr )
{
	RSContext	context( this ) ;
	RSScript *	pScript = LoadScript( context, pwszFilePath ) ;
	if ( pScript == NULL )
	{
		return	NULL ;
	}
	context.OutputExceptionError( perr ) ;
	return	pScript ;
}

RSScript * RSVirtualMachine::LoadScript
	( RSContext& context, const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>	pFile = OpenScriptFile( pwszFilePath ) ;
	if ( pFile == NULL )
	{
		SString	strSrcPath ;
		SString	strSrcLine ;
		size_t	iSrcIndex, iSrcLine ;
		if ( !context.GetCurrentPositionInfo
				( strSrcPath, strSrcLine, iSrcIndex, iSrcLine ) )
		{
			OutputError
				( SString(L"\'") + SString(pwszFilePath)
							+ L"\' を開けませんでした",
					strSrcPath, strSrcLine, iSrcLine, 0 ) ;
		}
		else
		{
			OutputError
				( SString(L"\'") + SString(pwszFilePath)
					+ L"\' を開けませんでした", NULL, NULL, 0, 0 ) ;
		}
		return	NULL ;
	}
	ESLTrace( "load Rosetta script: \'%s\'\n",
				SString(pwszFilePath).ToCharArray().GetConstArray() ) ;
	RSSourceParser	sparsSource ;
	sparsSource.ReadTextFile( *pFile ) ;
	return	AddScriptSource( context, sparsSource, pwszFilePath ) ;
}

RSScript * RSVirtualMachine::AddScriptSource
	( const wchar_t * pwszSource,
		const wchar_t * pwszFilePath,
		SSystem::SParserErrorInterface& perr )
{
	RSContext	context( this ) ;
	RSScript *	pScript =
		AddScriptSource( context, pwszSource, pwszFilePath ) ;
	if ( pScript == NULL )
	{
		return	NULL ;
	}
	context.OutputExceptionError( perr ) ;
	return	pScript ;
}

RSScript * RSVirtualMachine::AddScriptSource
	( RSContext& context,
		const wchar_t * pwszSource, const wchar_t * pwszFilePath )
{
	ESLAssert( m_pMacroCtx != NULL ) ;
	SParserErrorLogger	pelog ;
	RSScript *	pScript = new RSScript ;
	pScript->SetDebugFlag( m_flagDebug ) ;
	pScript->SetSourcePath( pwszFilePath ) ;
	m_csMacroCtx.Lock() ;
	if ( pScript->ParseSource( *m_pMacroCtx, pwszSource, pelog ) )
	{
		m_csMacroCtx.Unlock() ;
		OutputErrorLog( pelog, pwszFilePath ) ;
		delete	pScript ;
		return	NULL ;
	}
	m_csMacroCtx.Unlock() ;
	OutputErrorLog( pelog, pwszFilePath ) ;
	//
	QuickLock() ;
	m_ssoaScripts.SetAs( NormalizeScriptPath( pwszFilePath ), pScript ) ;
	QuickUnlock() ;
	//
	RSCodeStream	cs( *pScript ) ;
	AddRef() ;
	context.PushNamespace( NULL, this, RSObject::modifierPublic, true ) ;
	context.ExecuteAllStatements( cs ) ;
	context.PopNamespace() ;
	//
	return	pScript ;
}

// 単一の数式を評価（主に無名 function の生成の為）
//////////////////////////////////////////////////////////////////////////////
RSExpressionScript * RSVirtualMachine::CreateExpressionScript
	( const wchar_t * pwszExpr, SSystem::SParserErrorInterface& perr )
{
	RSExpressionScript *	pxs = new RSExpressionScript ;
	if ( pxs->Execute( this, pwszExpr, perr ) )
	{
		delete	pxs ;
		return	NULL ;
	}
	return	pxs ;
}

// スクリプト取得
//////////////////////////////////////////////////////////////////////////////
RSScript * RSVirtualMachine::GetLoadedScriptAs( const wchar_t * pwszFilePath ) const
{
	RSScript *	pScript ;
	QuickLock() ;
	pScript = m_ssoaScripts.GetAs( NormalizeScriptPath( pwszFilePath ) ) ;
	QuickUnlock() ;
	return	pScript ;
}

// スクリプトパスの正規化
//////////////////////////////////////////////////////////////////////////////
SString RSVirtualMachine::NormalizeScriptPath( const wchar_t * pwszFilePath )
{
	SString	strPath = pwszFilePath ;
	SString	strFileName = strPath.GetFileNamePart() ;
	strFileName.MakeLower() ;
	strFileName.Replace( L'\\', L'/' ) ;
	return	strFileName ;
}

// Sakura2 コードへコンパイル
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSVirtualMachine::CompileToSakura2
		( bool flagFlatPointer,
			bool flagCompileToNative, SSystem::SParserErrorLogger& perr )
{
	if ( m_pSakura2VM == NULL )
	{
		return	errFailed ;
	}
	ECSSakura2::ExecutableModuleMaker *
			pxmModule = new ECSSakura2::ExecutableModuleMaker ;
	pxmModule->InitializeMake() ;
	//
	m_ulsoaCompiledInfo.RemoveAll() ;
	//
	RSCompiler	compiler( m_pContext, pxmModule ) ;
	if ( flagFlatPointer )
	{
		compiler.SetBehaviorFlags
			( compiler.GetBehaviorFlags() | RSCompiler::behaviorFlatPointer ) ;
	}
	else
	{
		compiler.SetBehaviorFlags
			( compiler.GetBehaviorFlags() & ~RSCompiler::behaviorFlatPointer ) ;
	}
	SError		err = CompileNamespace( compiler, this, perr ) ;
	if ( !err && (m_ulsoaCompiledInfo.GetLength() > 0) )
	{
		pxmModule->FinishMake() ;
		m_pSakura2VM->AllocateModule( pxmModule ) ;
		m_aModules.Add( pxmModule ) ;
		//
		if ( flagCompileToNative )
		{
			pxmModule->CompileToNativeCode( false ) ;
		}
		//
		for ( size_t i = 0; i < m_ulsoaCompiledInfo.GetLength(); i ++ )
		{
			CompiledInfo *	pci = m_ulsoaCompiledInfo.GetAt( i ) ;
			ESLAssert( pci != NULL ) ;
			uint64_t	addrFunc =
				m_pSakura2VM->GetFunctionAddress( pci->m_strFullName, NULL ) ;
			if ( (addrFunc != 0)
				&& pci->m_pProto
				&& (pci->m_pProto->m_pfnFuncAddr == -1) )
			{
				pci->m_pProto->m_pfnFuncAddr = (int64_t) addrFunc ;
			}
		}
	}
	else
	{
		delete	pxmModule ;
	}
	return	err ;
}

SSystem::SError RSVirtualMachine::CompileNamespace
	( RSCompiler & compiler, RSObject * pObj,
		SSystem::SParserErrorLogger& perr )
{
	SError		err = errSuccess ;
	RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
	size_t		nCount = pObj->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pElement = pObj->GetElementAt( *m_pContext, (int) i ) ;
		RSFunctionObject *
					pFunc = ESLTypeCast<RSFunctionObject>( pElement ) ;
		if ( pFunc != NULL )
		{
			if ( CompileFunction( compiler, *pFunc, perr ) )
			{
				err = errFailed ;
			}
		}
		else
		{
			RSClass *	pSubClass = ESLTypeCast<RSClass>( pElement ) ;
			if ( pSubClass != NULL )
			{
				if ( CompileNamespace( compiler, pSubClass, perr ) )
				{
					err = errFailed ;
				}
			}
		}
		RSObject::ReleaseRef( pElement ) ;
	}
	if ( (pClass != NULL) && !(pClass->IsCompiledImplement()) )
	{
		nCount = pClass->m_gcmVirtuals.GetMemberCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSObject *	pElement = pClass->m_gcmVirtuals.GetMemberAt( i ) ;
			RSFunctionObject *
						pFunc = ESLTypeCast<RSFunctionObject>( pElement ) ;
			if ( pFunc != NULL )
			{
				if ( CompileFunction( compiler, *pFunc, perr ) )
				{
					err = errFailed ;
				}
			}
			RSObject::ReleaseRef( pElement ) ;
		}
		pClass->SetCompiledFlag() ;
	}
	return	err ;
}

SSystem::SError RSVirtualMachine::CompileFunction
	( RSCompiler & compiler,
		RSFunctionObject & func, SSystem::SParserErrorLogger& perr )
{
	SError	err = errSuccess ;
	for ( size_t i = 0; i < func.m_arrPrototypes.GetLength(); i ++ )
	{
		RSFunctionPrototype *	pProto = func.m_arrPrototypes.GetAt( i ) ;
		if ( (pProto == NULL)
			|| pProto->IsCompiledImplement()
			|| (pProto->m_pfnFuncAddr != -1) /* コンパイル済み */
			|| (pProto->m_methodNative.pfnMethod != NULL) /* ネイティブ関数 */ )
		{
			continue ;
		}
		CompiledInfo *	pci =
			m_ulsoaCompiledInfo.GetAs( (ulong_ptr_t) pProto ) ;
		if ( pci != NULL )
		{
			continue ;
		}
		SString		strFuncName ;
		RSClass *	pClass = pProto->m_pNamespaceClass ;
		if ( pClass != NULL )
		{
			strFuncName = pClass->GetFullClassName() ;
			strFuncName += L"." ;
		}
		strFuncName += func.m_strFuncName ;
		if ( func.m_strFuncName.IsEmpty() )
		{
			strFuncName += L"function" ;
		}
		if ( compiler.GetModule()->GetFunctionEntry( strFuncName ) != NULL )
		{
			SString	strBaseName = strFuncName ;
			for ( int j = 0; j < 0x7FFFFFFF; j ++ )
			{
				strFuncName = strBaseName ;
				strFuncName += SString( j ) ;
				if ( compiler.GetModule()->
						GetFunctionEntry( strFuncName ) == NULL )
				{
					break ;
				}
			}
		}
		SParserErrorLogger	peLog ;
		peLog.AttachErrorLogContext( perr.GetErrorLogContext() ) ;
		//
		compiler.GetModule()->BeginFunction( strFuncName ) ;
		if ( compiler.CompileFunction( pClass, *pProto, peLog ) )
		{
			for ( size_t j = 0; j < peLog.GetErrorCount(); j ++ )
			{
				SParserErrorLogger::ErrorLog *
							pErrLog = peLog.GetErrorLogAt( j ) ;
				ESLAssert( pErrLog != NULL ) ;
				perr.AddErrorLog
					( new SParserErrorLogger::ErrorLog( *pErrLog ) ) ;
				err = errFailed ;
			}
			for ( size_t j = 0; j < peLog.GetWarningCount(); j ++ )
			{
				SParserErrorLogger::ErrorLog *
							pErrLog = peLog.GetWarningLogAt( j ) ;
				ESLAssert( pErrLog != NULL ) ;
				perr.AddWarningLog
					( new SParserErrorLogger::ErrorLog( *pErrLog ) ) ;
			}
			pci = new CompiledInfo ;
			pci->m_pProto = pProto ;
			pci->m_strFullName = strFuncName ;
			m_ulsoaCompiledInfo.Add( (ulong_ptr_t) pProto, pci ) ;
			//
			compiler.GetModule()->EndFunction() ;
		}
		else
		{
			for ( size_t j = 0; j < peLog.GetErrorCount(); j ++ )
			{
				SParserErrorLogger::ErrorLog *
							pErrLog = peLog.GetErrorLogAt( j ) ;
				ESLAssert( pErrLog != NULL ) ;
				perr.AddWarningLog
					( new SParserErrorLogger::ErrorLog( *pErrLog ) ) ;
			}
			for ( size_t j = 0; j < peLog.GetWarningCount(); j ++ )
			{
				SParserErrorLogger::ErrorLog *
							pErrLog = peLog.GetWarningLogAt( j ) ;
				ESLAssert( pErrLog != NULL ) ;
				perr.AddWarningLog
					( new SParserErrorLogger::ErrorLog( *pErrLog ) ) ;
			}
			compiler.GetModule()->DeleteFunction() ;
		}
		pProto->SetCompiledFlag() ;
	}
	return	err ;
}

// Sakura2 コード位置からスクリプト位置を検索
//////////////////////////////////////////////////////////////////////////////
bool RSVirtualMachine::SearchCodePosition
		( RSVirtualMachine::ScriptPosition& sp, int64_t ip ) const
{
	if ( m_pSakura2VM == NULL )
	{
		return	false ;
	}
	size_t	nModules = m_pSakura2VM->GetModuleCount() ;
	for ( size_t i = 0; i < nModules; i ++ )
	{
		ECSSakura2::ExecutableModuleMaker *
			pModule = ESLTypeCast<ECSSakura2::ExecutableModuleMaker>
									( m_pSakura2VM->GetModuleAt( (int) i ) ) ;
		if ( (pModule != NULL)
			&& ((DWORD)(ip >> 32) == pModule->m_bufCode.m_dwHighAddr) )
		{
			const ECSSakura2::ExecutableModuleMaker::DebugCodeInfo *
				pdci = pModule->SearchDebugCodeInfo( (size_t) ((DWORD)ip) ) ;
			if ( pdci != NULL )
			{
				sp.pParenthesis = pdci->pParenthesis ;
				sp.iSource = pdci->indexSrc ;
				return	true ;
			}
		}
	}
	return	false ;
}

// 全クラス定義ダンプ
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::DumpAllClassDeclaration( SSystem::SFileInterface& file )
{
	size_t	nCount = m_gcmGenClasses.GetMemberCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObj = m_gcmGenClasses.GetMemberAt( i ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
		if ( pClass != NULL )
		{
			SString	strDecl ;
			pClass->DumpClassDeclaration( *m_pContext, strDecl ) ;
			strDecl += L" ;\r\n\r\n" ;
			file.WriteEncodedString( strDecl ) ;
		}
		m_pContext->ReleaseObjectRef( pObj ) ;
	}
	nCount = GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObj = GetElementAt( *m_pContext, (int) i ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
		if ( pClass != NULL )
		{
			SString	strDecl ;
			pClass->DumpClassDeclaration( *m_pContext, strDecl ) ;
			strDecl += L" ;\r\n\r\n" ;
			file.WriteEncodedString( strDecl ) ;
		}
		m_pContext->ReleaseObjectRef( pObj ) ;
	}
}

// 実行中スレッド登録
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AddRunningThread( RSThread * pThread )
{
	ESLAssert( pThread->m_pChainPrev == NULL ) ;
	ESLAssert( pThread->m_pChainNext == NULL ) ;
	QuickLock() ;
	pThread->m_pChainNext = m_pRunningThreads ;
	if ( m_pRunningThreads != NULL )
	{
		ESLAssert( m_pRunningThreads->m_pChainPrev == NULL ) ;
		m_pRunningThreads->m_pChainPrev = pThread ;
	}
	m_pRunningThreads = pThread ;
	m_countRunning ++ ;
	QuickUnlock() ;
}

// 実行中スレッド解除
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::DetachRunningThread( RSThread * pThread )
{
	QuickLock() ;
	RSThread *	pPrev = pThread->m_pChainPrev ;
	RSThread *	pNext = pThread->m_pChainNext ;
	if ( pPrev != NULL )
	{
		pPrev->m_pChainNext = pNext ;
	}
	else
	{
		m_pRunningThreads = pNext ;
	}
	if ( pNext != NULL )
	{
		pNext->m_pChainPrev = pPrev ;
	}
	pThread->m_pChainPrev = NULL ;
	pThread->m_pChainNext = NULL ;
	ESLVerify( -- m_countRunning >= 0 ) ;
	QuickUnlock() ;
}

// 実行中スレッド数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSVirtualMachine::GetRunningThreadCount( void ) const
{
	return	m_countRunningThreads ;
}

// 全スレッドを強制終了
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::AbortAllThreads( void )
{
	for ( ; ; )
	{
		SPointerArray<RSThread>	aThreads ;
		RSThread *	pThread ;
		QuickLock() ;
		pThread = m_pRunningThreads ;
		while ( pThread != NULL )
		{
			aThreads.Add( pThread ) ;
			pThread->AddRef() ;
			pThread = pThread->m_pChainNext ;
		}
		QuickUnlock() ;
		//
		size_t	nCount = aThreads.GetLength() ;
		if ( nCount == 0 )
		{
			break ;
		}
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSThread *	pThread = aThreads.GetAt( i ) ;
			pThread->AbortThread() ;
			pThread->ReleaseRef() ;
		}
	}
}

// マクロコンテキストの排他同期（スクリプト解釈同期用
//////////////////////////////////////////////////////////////////////////////
RSContext * RSVirtualMachine::LockMacroContext( void ) const
{
	m_csMacroCtx.Lock() ;
	return	m_pMacroCtx ;
}

void RSVirtualMachine::UnlockMacroContext( void ) const
{
	m_csMacroCtx.Unlock() ;
}

// スクリプト実行開始同期
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	RSVirtualMachine::EnterRunning( RSContext& context, int64_t msecTimeout )
{
	SError	err = m_mutexAssertLock.Lock( msecTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	m_countRunning ++ ;
	m_sigAllLeaved.ResetSignal() ;
	m_mutexAssertLock.Unlock() ;
	return	err ;
}

SSystem::SError RSVirtualMachine::LeaveRunning( void )
{
	SError	err = m_mutexAssertLock.Lock() ;
	if ( err )
	{
		return	err ;
	}
	if ( (-- m_countRunning) <= 0 )
	{
		ESLAssert( m_countRunning == 0 ) ;
		m_sigAllLeaved.SetSignal() ;
	}
	m_mutexAssertLock.Unlock() ;
	return	err ;
}

// スクリプト実行排他同期
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSVirtualMachine::LockAssert( int64_t msecTimeout )
{
	for ( ; ; )
	{
		SError	err = m_sigAllLeaved.Wait( msecTimeout ) ;
		if ( err )
		{
			return	err ;
		}
		err = m_mutexAssertLock.Lock() ;
		if ( err )
		{
			return	err ;
		}
		if ( m_countRunning == 0 )
		{
			break ;
		}
		m_mutexAssertLock.Unlock() ;
	}
	return	errSuccess ;
}

SSystem::SError RSVirtualMachine::UnlockAssert( void )
{
	m_mutexAssertLock.Unlock() ;
	return	errSuccess ;
}

// クラス取得
//////////////////////////////////////////////////////////////////////////////
RSClass * RSVirtualMachine::GetClassAs( const wchar_t * pwszClassName ) const
{
	RSObject *	pObj = m_gcmMembers.GetMemberAs( pwszClassName ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetBasicType() == RSObject::typeClass )
		{
			ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
			pObj->ReleaseRef() ;
			return	(RSClass*) pObj ;
		}
		pObj->ReleaseRef() ;
	}
	QuickLock() ;
	pObj = m_gcmGenClasses.GetMemberAs( pwszClassName ) ;
	QuickUnlock() ;
	if ( pObj != NULL )
	{
		if ( pObj->GetBasicType() == RSObject::typeClass )
		{
			ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
			pObj->ReleaseRef() ;
			return	(RSClass*) pObj ;
		}
		pObj->ReleaseRef() ;
	}
	return	NULL ;
}

RSClass * RSVirtualMachine::GetArrayClassAs
			( RSClass * pClass, int nArrayDimension )
{
	ESLAssert( pClass != NULL ) ;
	QuickLock() ;
	while ( nArrayDimension > 0 )
	{
		if ( pClass->m_pArrayClass == NULL )
		{
			SString	strClassName = pClass->GetFullClassName() ;
			strClassName += L"[]" ;
			//
			RSObject *	pArrayClassObj =
							m_gcmGenClasses.GetMemberAs( strClassName );
			if ( (pArrayClassObj != NULL)
				&& (pArrayClassObj->GetBasicType() == RSObject::typeClass) )
			{
				pArrayClassObj->ReleaseRef() ;
				pClass->m_pArrayClass = (RSClass*) pArrayClassObj ;
			}
			else
			{
				RSObject::ReleaseRef( pArrayClassObj ) ;
				pClass->m_pArrayClass = 
					new RSGenericArrayClass( m_pMetaClass, strClassName, pClass ) ;
				//
				ESLAssert( m_pContext != NULL ) ;
				RSObject::ReleaseRef
					( m_gcmGenClasses.SetMemberAs
						( *m_pContext, strClassName, pClass->m_pArrayClass ) ) ;
				QuickUnlock() ;
				//
				RSContext	context( this ) ;
				pClass->m_pArrayClass->AddSuperClass( context, m_pArrayClass ) ;
				pClass->m_pArrayClass->Initialize( context ) ;
				pClass->m_pArrayClass->AddRef() ;
				//
				QuickLock() ;
			}
		}
		pClass = pClass->m_pArrayClass ;
		nArrayDimension -- ;
	}
	QuickUnlock() ;
	return	pClass ;
}

RSClass * RSVirtualMachine::GetHashMapClassAs( RSClass * pClass )
{
	ESLAssert( pClass != NULL ) ;
	QuickLock() ;
	if ( pClass->m_pHashMapClass == NULL )
	{
		SString	strClassName = L"HashMap<" ;
		strClassName += pClass->GetFullClassName() ;
		strClassName += L">" ;
		//
		RSObject *	pHashClassObj =
						m_gcmGenClasses.GetMemberAs( strClassName );
		if ( (pHashClassObj != NULL)
			&& (pHashClassObj->GetBasicType() == RSObject::typeClass) )
		{
			pClass->m_pHashMapClass = (RSClass*) pHashClassObj ;
		}
		else
		{
			RSObject::ReleaseRef( pHashClassObj ) ;
			pClass->m_pHashMapClass = 
				new RSGenericHashMapClass
						( m_pMetaClass, strClassName, pClass ) ;
			//
			ESLAssert( m_pContext != NULL ) ;
			RSObject::ReleaseRef
				( m_gcmGenClasses.SetMemberAs
					( *m_pContext, strClassName, pClass->m_pHashMapClass ) ) ;
			QuickUnlock() ;
			//
			RSContext	context( this ) ;
			pClass->m_pHashMapClass->AddSuperClass( context, m_pJSObjectClass ) ;
			pClass->m_pHashMapClass->Initialize( context ) ;
			pClass->m_pHashMapClass->AddRef() ;
			//
			QuickLock() ;
		}
	}
	pClass = pClass->m_pHashMapClass ;
	QuickUnlock() ;
	return	pClass ;
}

RSClass * RSVirtualMachine::GetFunctionClassAs( const RSFunctionPrototype & proto )
{
	//
	// ジェネリッククラス名
	//
	SString	strClassName = L"Function<" ;
	if ( proto.m_pReturnType != NULL )
	{
		strClassName += proto.m_pReturnType->GetFullClassName() ;
	}
	else
	{
		strClassName += L"void" ;
	}
	strClassName += L"," ;
	//
	if ( proto.m_pNamespaceClass != NULL )
	{
		strClassName += proto.m_pNamespaceClass->GetFullClassName() ;
	}
	else
	{
		strClassName += L"Object" ;
	}
	for ( size_t i = 0; i < proto.m_aArgTypes.GetLength(); i ++ )
	{
		RSClass *	pArgType = proto.m_aArgTypes.GetAt( i ) ;
		strClassName += L"," ;
		if ( pArgType != NULL )
		{
			strClassName += pArgType->GetFullClassName() ;
		}
		else
		{
			strClassName += L"Object" ;
		}
	}
	strClassName += L">" ;
	//
	// 定義済みクラス検索
	//
	RSClass *	pGenClass = NULL ;
	QuickLock() ;
	RSObject *	pClassObj = m_gcmGenClasses.GetMemberAs( strClassName ) ;
	if ( (pClassObj != NULL)
		&& (pClassObj->GetBasicType() == RSObject::typeClass) )
	{
		pGenClass = (RSClass*) pClassObj ;
		pClassObj->ReleaseRef() ;
		QuickUnlock() ;
	}
	else
	{
		//
		// クラス生成
		//
		RSObject::ReleaseRef( pClassObj ) ;
		//
		pGenClass =
			new RSGenericFunctionClass
				( m_pMetaClass, strClassName, proto ) ;
		//
		ESLAssert( m_pContext != NULL ) ;
		RSObject::ReleaseRef
			( m_gcmGenClasses.SetMemberAs
				( *m_pContext, strClassName, pGenClass ) ) ;
		QuickUnlock() ;
		//
		RSContext	context( this ) ;
		pGenClass->Initialize( context ) ;
	}
	return	pGenClass ;
}

RSClass * RSVirtualMachine::GetExceptioinClassAs( const wchar_t * pwszTypeClass )
{
	RSClass *	pClass = NULL ;
	QuickLock() ;
	RSObject *	pClassObj = m_gcmGenClasses.GetMemberAs( pwszTypeClass ) ;
	if ( (pClassObj != NULL)
		&& (pClassObj->GetBasicType() == RSObject::typeClass) )
	{
		pClass = (RSClass*) pClassObj ;
		pClassObj->ReleaseRef() ;
		QuickUnlock() ;
	}
	else
	{
		RSObject::ReleaseRef( pClassObj ) ;
		pClass = new RSExceptionClass( m_pMetaClass, pwszTypeClass ) ;
		//
		ESLAssert( m_pContext != NULL ) ;
		RSObject::ReleaseRef
			( m_gcmGenClasses.SetMemberAs
				( *m_pContext, pwszTypeClass, pClass ) ) ;
		QuickUnlock() ;
		//
		RSContext	context( this ) ;
		pClass->AddSuperClass( context, m_pExceptionClass ) ;
		pClass->Initialize( context ) ;
	}
	return	pClass ;
}

// 拡張クラス定義
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::RegisterNewClass( RSClass * pClass, bool flagNoImpl )
{
	RSContext	context( this ) ;
	QuickLock() ;
	RSObject::ReleaseRef
		( m_gcmGenClasses.SetMemberAs
			( context, pClass->GetRSClassName(), pClass ) ) ;
	if ( !flagNoImpl )
	{
		m_aRegClasses.Add( pClass ) ;
	}
	QuickUnlock() ;
}

void RSVirtualMachine::AddImplementClass( RSClass * pClass )
{
	QuickLock() ;
	m_aRegClasses.Add( pClass ) ;
	QuickUnlock() ;
}

void RSVirtualMachine::ImplementNewClasses( void )
{
	RSContext	context( this ) ;
	for ( size_t i = 0; i < m_aRegClasses.GetLength(); i ++ )
	{
		RSClass *	pClass = m_aRegClasses.GetAt( i ) ;
		pClass->Initialize( context ) ;
		pClass->FinishClass( context ) ;
	}
	m_aRegClasses.RemoveAll() ;
}

// ジェネリッククラス取得
//////////////////////////////////////////////////////////////////////////////
size_t RSVirtualMachine::GetGenericClassCount( void ) const
{
	return	m_gcmGenClasses.GetMemberCount() ;
}

RSClass * RSVirtualMachine::GetGenericClassAt( size_t nIndex ) const
{
	RSClass *	pClass = NULL ;
	QuickLock() ;
	RSObject *	pObj = m_gcmGenClasses.GetMemberAt( nIndex );
	if ( pObj != NULL )
	{
		pClass = ESLTypeCast<RSClass>( pObj ) ;
		pObj->ReleaseRef() ;
	}
	QuickUnlock() ;
	return	pClass ;
}

// 基本型クラス判定
//////////////////////////////////////////////////////////////////////////////
RSCodeControl::WordIndex RSVirtualMachine::IsBasicTypeClass( RSClass * pClass ) const
{
	for ( size_t i = 0; i < RSCodeControl::wiBasicTypeCount; i ++ )
	{
		if ( m_pBasicTypeClass[i] == pClass )
		{
			return	(RSCodeControl::WordIndex) (RSCodeControl::wiFirstBasicType + i) ;
		}
	}
	return	RSCodeControl::wiInvalid ;
}

// ポインタ型クラス判定
//////////////////////////////////////////////////////////////////////////////
RSReferenceNumber::NumberType RSVirtualMachine::IsTypedPointerClass( RSClass * pClass ) const
{
	for ( size_t i = 0; i < RSReferenceNumber::typeCountOfNumber; i ++ )
	{
		if ( m_pPtrTypeClass[i] == pClass )
		{
			return	(RSReferenceNumber::NumberType) i ;
		}
	}
	return	RSReferenceNumber::typeObject ;
}

// ジェネリック型クラス判定
bool RSVirtualMachine::IsGenericTypeClass( RSClass * pClass ) const
{
	RSGenericArrayClass *	pArray = ESLTypeCast<RSGenericArrayClass>( pClass ) ;
	if ( pArray != NULL )
	{
		return	(pArray->m_pElementClass != NULL) ;
	}
	return	(ESLTypeCast<RSGenericHashMapClass>( pClass ) != NULL)
			|| (ESLTypeCast<RSGenericFunctionClass>( pClass ) != NULL) ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualMachine::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	QuickLock() ;
	RSObject *	pObj = RSNamespace::GetMemberAs( context, pwszName ) ;
	QuickUnlock() ;
	return	pObj ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualMachine::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	QuickLock() ;
	RSObject *	pMember = m_gcmMembers.DetachMemberAs( pwszName ) ;
	pObj = m_gcmMembers.SetMemberAs( context, pwszName, pObj ) ;
	QuickUnlock() ;
	context.ReleaseObjectRef( pMember ) ;
	return	pObj ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualMachine::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	QuickLock() ;
	RSObject *	pMember = m_gcmMembers.GetMemberAs( pwszName ) ;
	if ( pMember != NULL )
	{
		QuickUnlock() ;
		context.ThrowExceptionError
			( SString(pwszName) + L" は二重定義です" ) ;
		context.ReleaseObjectRef( pMember ) ;
	}
	else
	{
		pMember = m_gcmMembers.SetMemberAs( context, pwszName, pObj ) ;
		QuickUnlock() ;
		return	pMember ;
	}
	return	NULL ;
}

// ネイティブ関数の設定
//////////////////////////////////////////////////////////////////////////////
SError RSVirtualMachine::SetNativeMethod
	( RSContext& context,
		const wchar_t * pwszMethodPath,
		RSObject::METHOD_PROC pfnMethod, void * pInstance )
{
	RSSourceParser	sparsMethod = pwszMethodPath ;
	SString			strName, strMemberOp ;
	RSSmartPtr		ptrNamespace( NULL, &context ) ;
	RSObject *		pMember = NULL ;
	RSObject *		pNamespace = this ;
	for ( ; ; )
	{
		if ( sparsMethod.NextToken( strName )
						!= SStringParser::tokenNormal )
		{
			ESLTrace( "syntax error for method: %s\n",
							sparsMethod.ToCharArray().GetConstArray() ) ;
			return	errFailed ;
		}
		pMember = pNamespace->GetMemberAs( context, strName ) ;
		if ( pMember == NULL )
		{
			RSClass *	pClass = ESLTypeCast<RSClass>( pNamespace ) ;
			if ( pClass != NULL )
			{
				pMember = pClass->GetVirtualMemberAs( context, strName ) ;
			}
			if ( pMember == NULL )
			{
				ESLTrace( "not found member \'%s\' of \'%s\'\n",
							strName.ToCharArray().GetConstArray(),
							sparsMethod.ToCharArray().GetConstArray() ) ;
				return	errFailed ;
			}
		}
		pNamespace = pMember ;
		ptrNamespace = pMember ;
		//
		sparsMethod.NextToken( strMemberOp ) ;
		if ( (strMemberOp != L".") && (strMemberOp != L"::") )
		{
			if ( strMemberOp.IsEmpty() )
			{
				break ;
			}
			ESLTrace( "syntax error \'%s\' of \'%s\'\n",
						strMemberOp.ToCharArray().GetConstArray(),
						sparsMethod.ToCharArray().GetConstArray() ) ;
			return	errFailed ;
		}
	}
	RSFunctionObject *	pFuncObj = ESLTypeCast<RSFunctionObject>( pMember ) ;
	if ( (pFuncObj == NULL) || (pFuncObj->m_pPrototype == NULL) )
	{
		ESLTrace( "\'%s\' is not function.\n",
						sparsMethod.ToCharArray().GetConstArray() ) ;
		return	errFailed ;
	}
	RSFunctionPrototype *	pProto = pFuncObj->m_pPrototype ;
	if ( pProto->m_pParenthesis != NULL )
	{
		ESLTrace( "warning: \'%s\' is implemented.\n",
						sparsMethod.ToCharArray().GetConstArray() ) ;
	}
	pProto->m_methodNative.pfnMethod = pfnMethod ;
	pProto->m_methodNative.pInstance = pInstance ;
	return	errSuccess ;
}

// ネイティブ関数の設定（native 関数宣言前・事前登録）
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::RegisterNativeMethod
	( const wchar_t * pwszMethodPath, const RSObject::METHOD_ENTRY & method )
{
	m_ssaNativeFuncs.SetAs( pwszMethodPath, method ) ;
}

// ネイティブ簡易実装関数を登録
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::RegisterNativeMethodAllDescriptors( void )
{
	const NativeFuncDescriptor *	pnfdNext = s_pnfdFirstDesc ;
	while ( pnfdNext != nullptr )
	{
		const NativeFuncDescriptor *	pnfdDesc = pnfdNext ;
		pnfdNext = pnfdNext->pnfdNext ;
		//
		SString		strFuncName = pnfdDesc->pwszFuncName ;
		if ( strFuncName.Find( L'.' ) < 0 )
		{
			if ( strFuncName.GetAt(0) == L'_' )
			{
				size_t		nLen = strFuncName.GetLength() ;
				uint16_t *	pwStr = strFuncName.LockBuffer( nLen ) ;
				size_t		iDst = 0, iSrc = 1 ;
				while ( iSrc < nLen )
				{
					if ( (pwStr[iSrc] == L'_')
						&& (iSrc + 1 < nLen)
						&& (pwStr[iSrc + 1] == L'_') )
					{
						pwStr[iDst ++] = (uint16_t) L'.' ;
						iSrc += 2 ;
					}
					else
					{
						pwStr[iDst ++] = pwStr[iSrc ++] ;
					}
				}
				strFuncName.UnlockBuffer( (ssize_t) iDst ) ;
			}
			else
			{
				strFuncName.Replace( L'_', L'.' ) ;
			}
		}
		//
		RSObject::METHOD_ENTRY	method ;
		method.pfnMethod = pnfdDesc->pfnNativeProc ;
		method.pInstance = nullptr ;
		//
		RegisterNativeMethod( strFuncName, method ) ;
	}
}

// ネイティブ関数取得（native 関数定義時）
// （デフォルトは RegisterNativeMethod で登録された関数を取得）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSVirtualMachine::GetNativeMethod
	( RSObject::METHOD_ENTRY& method,
		RSContext& context, const wchar_t * pwszMethodPath )
{
	RSObject::METHOD_ENTRY *	pme = m_ssaNativeFuncs.GetAs( pwszMethodPath ) ;
	if ( pme != NULL )
	{
		method = *pme ;
		return	errSuccess ;
	}
	return	errFailed ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void RSVirtualMachine::OutputError
	( const wchar_t * pwszErrorMsg, const wchar_t * pwszFile,
		const wchar_t * pwszLineText, size_t nLineNum, size_t nColNum )
{
	ErrorLog *	pel = new ErrorLog ;
	pel->m_strErrMsg = pwszErrorMsg ;
	pel->m_strFile = pwszFile ;
	pel->m_strLineText = pwszLineText ;
	pel->m_nLineNum = nLineNum ;
	pel->m_nColNum = nColNum ;
	m_arrErrorLog.Add( pel ) ;
}

// エラーログ取得
//////////////////////////////////////////////////////////////////////////////
size_t RSVirtualMachine::GetErrorLogCount( void ) const
{
	return	m_arrErrorLog.GetLength() ;
}

RSVirtualMachine::ErrorLog *
	RSVirtualMachine::GetErrorLogAt( size_t i ) const
{
	return	m_arrErrorLog.GetAt( i ) ;
}

void RSVirtualMachine::OutputErrorLog
	( const SParserErrorLogger& pelog, const wchar_t * pwszFile )
{
	size_t	nCount = pelog.GetErrorCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SParserErrorLogger::ErrorLog *	pLog = pelog.GetErrorLogAt( i ) ;
		if ( pLog != NULL )
		{
			OutputError
				( pLog->m_strError, pwszFile,
					pLog->m_strLine, pLog->m_nLineNum, pLog->m_nColNum ) ;
		}
	}
}

// ExceptionHandler 実装
//////////////////////////////////////////////////////////////////////////////
bool RSVirtualMachine::HandleExceptionError
	( ECSSakura2::StandardVM * pVM,
		ECSSakura2Processor::Context * context, const wchar_t * pwszErr )
{
	return	true ;
}

// ネイティブ関数簡易実装用
//////////////////////////////////////////////////////////////////////////////
const RSVirtualMachine::NativeFuncDescriptor *
	RSVirtualMachine::AddNativeFuncDescriptor( const NativeFuncDescriptor * pnfdDesc )
{
	const NativeFuncDescriptor *	pnfdNext = s_pnfdFirstDesc ;
	s_pnfdFirstDesc = pnfdDesc ;
	return	pnfdNext ;
}

