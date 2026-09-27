
#if	!defined(__ROSETTA_COMPILER_H__)
#define	__ROSETTA_COMPILER_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Rosetta -> Sakura2 コンパイラ
	//////////////////////////////////////////////////////////////////////////

	class	RSCompiler	: public ESLObject
	{
	protected:
		RSContext *							m_context ;
		ECSSakura2::ExecutableModuleMaker *	m_xmm ;

	public:
		// 動作フラグ
		enum	BehaviorFlag
		{
			behaviorTruthSymbol	= 0x0001,	// 自明シンボルでない場合エラー出力
			behaviorFlatPointer	= 0x0002,	// 構造体のポインタ変換で境界チェックを省略
		} ;
	protected:
		uint32_t	m_flagsBehaivor ;

		// 変数数値型
		enum	NumberType
		{
			typeObject	= -1,
			typeInt8,
			typeUint8,
			typeInt16,
			typeUint16,
			typeInt32,
			typeUint32,
			typeInt64,
			typeFloat32,
			typeFloat64,
		} ;
		// ローカル変数アロケーション情報
		//	0000H: number/object/pointer
		//	0008H: 以前の YP チェーン          (flagObject == true)
		//	0010H: 破棄関数のID(0:ptr, 1:obj)  (flagObject == true)
		//	0018H: object                      (flagObject == true)
		struct	LocalVariable
		{
			ssize_t		bpNumOffset ;	// 数値データオフセット値
			ssize_t		bpObjOffset ;	// 解放が必要なオブジェクトへのオフセット値
			size_t		nBytes ;		// 占有バイト数
			bool		flagObject ;	// Rosetta 所有オブジェクト
			bool		flagPointer ;	// ポインタ型
			NumberType	typeNumber ;	// 数値型
			RSClass *	pClass ;		// 型
			uint32_t	accMod ;		// 型修飾

			// 構築関数
			LocalVariable( void )
				: bpObjOffset(0), bpNumOffset(0), nBytes(0),
					flagObject(false), flagPointer(false),
					typeNumber(typeObject), pClass(NULL), accMod(0) {}
			LocalVariable( const LocalVariable& lv )
				: bpObjOffset(lv.bpObjOffset),
					bpNumOffset(lv.bpNumOffset),
					nBytes(lv.nBytes),
					flagObject(lv.flagObject),
					flagPointer(lv.flagPointer),
					typeNumber(lv.typeNumber),
					pClass(lv.pClass), accMod(lv.accMod) {}
		} ;
		// コンパイル時型情報
		// m_pClass に型を格納
		// m_typeNum == typeObject の時には m_regLoaded レジスタに RSObject* を保持
		// m_typeNum != typeObject の時には実際の数値やポインタを保持
		// m_typeNum 型の数値かポインタかは m_flagPointer で判定
		class	TypeInfo
		{
		public:
			RSCompiler *	m_compiler ;
			TypeInfo *		m_ptiNext ;
			NumberType		m_typeNum ;			// 数値型
			uint32_t		m_accMod ;			// 型修飾
			bool			m_flagPointer ;		// ポインタ型
			bool			m_flagReference ;	// 参照型
			bool			m_flagLocal ;		// ローカル変数への参照
			bool			m_flagVirtual ;		// m_pImmediate は仮想関数
			bool			m_flagLoaded ;		// レジスタにロード済み
			bool			m_flagLockAddr ;	// m_regLoaded はローカル変数キャッシュ
			bool			m_flagRefRosetta ;	// m_regObject は Rosetta オブジェクト
												// そうでない場合はポインタ
			int				m_regLoaded ;		// ロードされたレジスタ番号
			int				m_regObject ;		// 解放が必要な Rosetta オブジェクトを保持しているレジスタ番号
			int				m_addrOffset ;		// オフセットアドレス
			const LocalVariable *
							m_pLocalVar ;		// ローカル変数
			RSClass *		m_pClass ;			// 型
			RSObject *		m_pImmediate ;		// 即値
		public:
			// 構築関数
			TypeInfo( RSCompiler * compiler ) ;
			TypeInfo( const TypeInfo & ti ) ;
			// 消滅関数
			~TypeInfo( void ) ;
			// 代入
			TypeInfo & operator = ( const TypeInfo & ti ) ;
			// Rosetta オブジェクトか？
			bool IsObject( void ) const ;
			// 整数型か？
			bool IsInteger( void ) const ;
			// 浮動小数点型か？
			bool IsFloatingPoint( void ) const ;
			// 型設定
			void SetType( RSContext& context, RSClass * pClass ) ;
			void ChangeType( RSContext& context, RSClass * pClass ) ;
			void CopyType( const TypeInfo & ti ) ;
			// ローカル変数への参照を設定
			void SetVarReference
				( RSContext& context, const LocalVariable * plv ) ;
			// 構造体メンバへの参照を設定
			void SetMemberReference
				( RSContext& context,
					const RSStructuredPointerClass::ElementInfo * pei ) ;
			// 即値設定
			void SetImmediate
				( RSContext& context, RSObject * pObj, RSClass * pClass ) ;
			// 即値取得
			RSObject * GetImmediate( void ) const ;
			// オブジェクトを設定
			void SetLoadedObject
				( RSContext& context, RSClass * pClass, int regLoaded ) ;
			// ポインタを設定
			void SetLoadedPointer
				( RSContext& context,
					RSTypedArrayPointerClass * pClass,
					int regLoaded, int regOwner = -1 ) ;
			// 数値を設定
			void SetLoadedNumber
				( RSContext& context, RSClass * pClass, int regLoaded ) ;
		} ;
		// ローカル空間
		class	LocalSpace
		{
		public:
			SSystem::SStrSortArray<LocalVariable>	m_ssaVar ;
			ssize_t									m_bpOffset ;
			size_t									m_nBytes ;
			bool									m_flagLoop ;
			SSystem::SArray<size_t>					m_arrBreakRefAddr ;
			SSystem::SArray<size_t>					m_arrContinueRefAddr ;
		public:
			// 構築関数
			LocalSpace( void )
				: m_bpOffset(0), m_nBytes(0), m_flagLoop(false) {}
			LocalSpace( const LocalSpace& ls )
				: m_ssaVar( ls.m_ssaVar ),
					m_bpOffset( ls.m_bpOffset ),
					m_nBytes( ls.m_nBytes ),
					m_flagLoop( ls.m_flagLoop ) {}
		} ;
		SSystem::SObjectArray<LocalSpace>	m_stackLocals ;
		LocalVariable *						m_plvThis ;
		size_t								m_nMaxLocalBytes ;
		bool								m_flagInvalidFunc ;
		SSystem::SParserErrorInterface *	m_pperr ;
		size_t								m_addrAddSP ;
		TypeInfo *							m_ptiFirstTemp ;
		TypeInfo *							m_ptiExprParentOf ;

		// 変数レジスタ割り当て r1～r7
		struct	RegisterAssignSlot
		{
			const LocalVariable *	m_plv ;			// 変数
			int						m_lastAccess ;	// 最後にアクセスしたカウンタ
			int						m_locked ;		// ロックカウンタ
			bool					m_modified ;	// 変更された
		} ;
		struct	RegisterAssign
		{
			RegisterAssignSlot	m_slot[7] ;
			int					m_access ;
		} ;
		RegisterAssign		m_raRegs ;

		// テンポラリレジスタ
		struct	ExprRegContext
		{
			int		nAlloc ;
			uint8_t	maskAlloc[128/8] ;

			// リセット
			void Reset( void ) ;
			// コピー
			const ExprRegContext& operator = ( const ExprRegContext& xrc ) ;
			// レジスタ確保
			int Allocate( void ) ;
			// レジスタ解放
			void Free( int reg ) ;
		} ;
		ExprRegContext			m_xrcRegs ;

		RSFunctionPrototype *	m_pPrototype ;
		const RSParenthesis *	m_pCurParenthesis ;
		size_t					m_iSrcStatement ;

		friend class TypeInfo ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCompiler, ESLObject )
		// 構築関数
		RSCompiler
			( RSContext * context,
				ECSSakura2::ExecutableModuleMaker * xmm ) ;
		// 消滅関数
		virtual ~RSCompiler( void ) ;
		// 出力先モジュール
		ECSSakura2::ExecutableModuleMaker * GetModule( void ) const
		{
			return	m_xmm ;
		}
		// 動作フラグ取得
		uint32_t GetBehaviorFlags( void ) const
		{
			return	m_flagsBehaivor ;
		}
		// 動作フラグ設定
		void SetBehaviorFlags( uint32_t nFlags )
		{
			m_flagsBehaivor = nFlags ;
		}

	public:
		// 関数コンパイル
		bool CompileFunction
			( RSClass * pThisClass,
				RSFunctionPrototype& proto,
				SSystem::SParserErrorInterface& perr ) ;
	protected:
		// 関数コンパイル初期化
		void PrepareCompileFunction
			( RSClass * pThisClass,
				RSFunctionPrototype& proto,
				SSystem::SParserErrorInterface& perr ) ;
		// 関数コンパイル終了
		void FinishCompileFunction( void ) ;
		// コンパイルエラー出力
		void OutputError( const wchar_t * pwszErrMsg ) ;

		// 関数復帰
		void CompileReturn( int regRet = ECSSakura2Processor::regAcc ) ;
		// Rosetta 例外判定し、例外発生の場合関数復帰
		void CompileCheckException( bool fNoExceptPos = false ) ;
		// アライメントチェックし、例外発生
		void CompileCheckAlignment( int reg, int nAlign ) ;

		// ローカル空間追加
		LocalSpace * PushLocalSpace( void ) ;
		// ローカル空間削除
		LocalSpace * PopLocalSpace( void ) ;
		// ローカル変数追加
		LocalVariable * AllocateLocalVariable
				( const wchar_t * pwszName, RSClass * pType ) ;
		// ローカル変数検索
		LocalVariable * SearchLocalVariable( const wchar_t * pwszName ) ;
		// ローカル空間の解放
		void FreeLocalSpace( LocalSpace * pls ) ;
		// 全ローカル空間の解放（LocalSpace は保持）
		void FreeAllLocalSpace( void ) ;

		// ローカル変数初期化（オブジェクトを数値やポインタへ実体化）
		void InitializeArgument( LocalVariable * plv, int regInit ) ;
		// ローカル変数初期化
		void InitializeVariable( LocalVariable * plv ) ;
		// ローカル変数の解放
		void FreeLocalVariable( const LocalVariable * plv ) ;
		// ローカル変数の解放（Rosetta オブジェクトの参照のみ）
		void ReleaseRefLocalVariable( const LocalVariable * plv ) ;
		// ローカル変数の解放（Sakura2 オブジェクトの解放のみ）
		void FreePointerLocalVariable( const LocalVariable * plv ) ;

		// 全一時オブジェクト参照を解放する（情報は現在のまま保持）
		void ReleaseAllTemporaryOwnerObjects( void ) ;
		// TypeInfo をチェーンに追加
		void AddChainTyepInfo( TypeInfo * ptiTemp ) ;
		// TypeInfo をチェーンから削除
		void DetachChainTyepInfo( TypeInfo * ptiTemp ) ;

		// コンパイル時型情報とオブジェクト参照を移転
		void MoveTemporaryTypeInfo( TypeInfo& tiDst, TypeInfo& tiSrc ) ;
		// テンポラリレジスタ解放とオブジェクト参照の解放
		void FreeTemporaryRegister( TypeInfo& ti ) ;

		// ローカル変数をキャッシュレジスタへ割り当ててロード
		int CommitLocalVariable( const LocalVariable * plv ) ;
		// ローカル変数をキャッシュレジスタへロード
		void LoadLocalVariable( const LocalVariable * plv, int reg ) ;
		// ローカル変数へキャッシュレジスタからライト
		void WriteLocalVariable( const LocalVariable * plv, int reg ) ;
		// ローカル変数のレジスタキャッシュをライトバック
		void WriteBackLocalVariable( int reg ) ;
		// すべてのローカル変数のレジスタキャッシュをライトバック
		void WriteBackAllLocalVariables( bool fClearModifiedFlag = true ) ;
		// すべてのローカル変数のレジスタキャッシュを再ロード
		void ReloadAllLocalVariables( void ) ;
		// ローカル変数のレジスタ割り当てを解除する
		void ResetAllLocalVariableCaches( void ) ;
		// ローカル変数のレジスタキャッシュをライトバックし割り当てを解除する
		void FlushAllLocalVariableCaches( void ) ;
		// ローカル変数のレジスタキャッシュをロック（解除不可）にする
		void LockCacheRegister( int reg ) ;
		// ローカル変数のレジスタキャッシュをアンロックにする
		void UnlockCacheRegister( int reg ) ;
		// すべてのローカル変数のレジスタキャッシュがアンロックされているか検証
		void VerifyUnlockAllLocalCache( void ) ;
		// ローカル変数の割り当てを保存する
		void SaveAllLocalVariableCaches( RegisterAssign& ra ) ;
		// ローカル変数の割り当てを復元する
		void RestoreAllLocalVariableCaches( const RegisterAssign& ra ) ;

		// テンポラリレジスタを一時 PUSH してコンテキストをリセット
		void PushAllTemporaryRegisters( ExprRegContext& xrc ) ;
		// テンポラリレジスタを POP してコンテキストを復元
		void PopAllTemporaryRegisters( const ExprRegContext& xrc ) ;

		// 未確保レジスタ番号取得
		int GetFreeTemporaryRegister( int nCount = 0 ) ;
		// テンポラリレジスタ確保
		int AllocateTemporaryRegister( void ) ;
		// テンポラリレジスタ解放
		void FreeTemporaryRegister( int reg ) ;
		// 所有オブジェクトとレジスタの解放
		void FreeTemporaryRegisterAndOwnerObject( TypeInfo& ti ) ;
		// 全テンポラリレジスタ解放
		void FreeAllTemporaryRegister( void ) ;
		// 全テンポラリレジスタの解放検証
		void VerifyFreeAllTemporaryRegister( void ) ;

	public:
		// 文コンパイル
		void CompileStatements( RSCodeStream& cstrm ) ;
		// 複文コンパイル
		void CompileMultiStatements( RSCodeStream& cstrm ) ;
		// 一文コンパイル
		void CompileAStatement( RSCodeStream& cstrm ) ;
		// 定義文コンパイル
		void CompileDeclareVariable
			( RSCodeStream& cstrm, uint32_t accMod, RSClass * pClass ) ;
		// 数式コンパイル
		bool CompileExpression
			( TypeInfo& tiExpr, RSCodeStream& cs,
				int priority = RSCodeOperator::priorityNothing ) ;

	protected:
		// 数式コンパイル（メンバ参照）
		bool CompileExpressionRefMemberOf
			( TypeInfo& tiExpr, RSCodeStream& cs,
				RSCodeOperator::OperatorIndex iOp ) ;
		// 数式コンパイル（expr ? expr : expr）
		bool CompileExpressionSelector
			( TypeInfo& tiExpr, RSCodeStream& cs ) ;

	protected:
		// 関数呼び出し
		bool CompileCallFunction
			( TypeInfo& tiThisRet,
				RSFunctionObject& func, bool flagVirtual,
				SSystem::SObjectArray<TypeInfo>& arg, bool fStructCast ) ;
		bool CompileCallFunction
			( TypeInfo& tiThisRet,
				TypeInfo& tiFunc,
				SSystem::SObjectArray<TypeInfo>& arg ) ;
		// システム定義済み関数判定
		bool CompileCallSystemFunction
			( RSCompiler::TypeInfo& tiThisRet,
				RSClass * pThisClass,
				RSFunctionPrototype * pProto,
				SSystem::SObjectArray<TypeInfo>& arg ) ;
		// 関数引数解釈
		bool CompileArgument
			( SSystem::SObjectArray<TypeInfo>& arg, RSParenthesis& prth ) ;
		// 関数引数をオブジェクトに変換する
		bool ConvertArgumentToObject( SSystem::SObjectArray<TypeInfo>& arg ) ;
		// 関数引数プッシュ（個数＋引数）
		void CompilePushArgument( SSystem::SObjectArray<TypeInfo>& arg ) ;
		// 関数適合プロトタイプ取得
		RSFunctionPrototype * GetMatchPrototype
			( size_t& iVirtual, RSFunctionObject& func, SSystem::SObjectArray<TypeInfo>& arg ) ;
		bool IsMatchPrototype
			( RSFunctionPrototype * pProto, SSystem::SObjectArray<TypeInfo>& arg ) ;
		// 一時オブジェクト参照の解放
		void FreeTemporaryRegisters( SSystem::SObjectArray<TypeInfo>& arg ) ;

	protected:
		// シンボル参照
		bool GetVariableAs
			( TypeInfo& tiExpr, const wchar_t * pwszName /* must be static */ ) ;
		// メンバへのアクセス保護判定と例外のスロー
		bool VerifyMemberAccessModifier
			( RSClass * pClass, RSObject * pMember, const wchar_t * pwszName ) ;
		// 構造体メンバ参照
		bool ReferenceStructureMember
			( TypeInfo& tiExpr,
				const wchar_t * pwszName,
				RSStructuredPointerClass::ElementInfo * peiMember ) ;
		// new 演算子   
		bool CompileNewOperator( TypeInfo& tiExpr, RSCodeStream& cstrm ) ;
		// 単項演算子
		bool CompileUnaryOperator
			( TypeInfo& tiExpr, RSCodeOperator::OperatorIndex opIndex ) ;
		// 二項演算子
		bool CompileBinaryOperator
			( TypeInfo& tiExpr, TypeInfo& tiRight,
					RSCodeOperator::OperatorIndex opIndex ) ;
		// 間接要素参照
		bool CompileReferenceElement
			( TypeInfo& tiExpr, TypeInfo& tiIndex ) ;

	protected:
		// NumberType -> DataType 変換
		static ECSSakura2Processor::DataType
					NumberTypeToDataType( NumberType type ) ;
		// TypeInfo -> DataType 変換
		static ECSSakura2Processor::DataType
					TypeInfoToDataType( const TypeInfo& ti ) ;
		// LocalVariable -> DataType 変換
		static ECSSakura2Processor::DataType
					LocalVariableToDataType( const LocalVariable * plv ) ;
		// RSReferenceNumber::NumberType -> NumberType 変換
		static NumberType ConvertNumberType( RSReferenceNumber::NumberType type ) ;
		// NumberType -> RSClass 変換
		RSClass * NumberTypeToClass( NumberType type ) const ;
		// ローカル変数への参照を作成
		void ReferenceToLocalVariable
			( TypeInfo& ti, const LocalVariable * plv ) ;
		// 参照や即値をレジスタにロード
		bool LoadReferenceTemporary( TypeInfo& ti, bool fTempReg = false ) ;
		// 数値やポインタオブジェクトを数値やポインタへ変換
		bool RealizeObjectToNumber( TypeInfo& ti ) ;
		// ブール値へ変換しレジスタにロード
		bool RealizeToBoolean( TypeInfo& ti ) ;
		// オブジェクトやポインタのオブジェクト参照を保持するように正規化
		bool TakeObjectReference( TypeInfo& ti ) ;

	protected:
		// 代入操作
		bool OperatorMove( TypeInfo& tiDst, TypeInfo& tiSrc ) ;
		// キャスト
		bool OperatorCast
			( TypeInfo& tiDst, RSClass * pClass, bool fForceCast = false ) ;
		// オブジェクトキャスト
		bool OperatorObjectCast
			( TypeInfo& tiDst, RSClass * pClass, bool fForceCast = false ) ;
		// Rosetta オブジェクトへ変換
		bool ConvertToObject( TypeInfo& tiDst ) ;
		// 数値やオブジェクトを複製する
		bool CloneObject( TypeInfo& tiDst, TypeInfo& tiSrc ) ;

	protected:
		typedef void (RSCompiler::*PFUNC_COMPILE_STATEMENT)
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		static const PFUNC_COMPILE_STATEMENT
						m_pfnCompileStatement[RSCodeControl::wiCount + 1] ;
	public:
		// import 文
		void CompileStatementImport
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// class 文
		void CompileStatementClass
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// struct 文
		void CompileStatementStruct
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// function 文
		void CompileStatementFunction
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// for 文
		void CompileStatementFor
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// while 文
		void CompileStatementWhile
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// do 文
		void CompileStatementDo
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// if 文
		void CompileStatementIf
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// switch 文
		void CompileStatementSwitch
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// case 文
		void CompileStatementCase
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// default 文
		void CompileStatementDefault
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// break 文
		void CompileStatementBreak
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// continue 文
		void CompileStatementContinue
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// try 文
		void CompileStatementTry
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// throw 文
		void CompileStatementThrow
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// return 文
		void CompileStatementReturn
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// with 文
		void CompileStatementWith
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// synchronized 文
		void CompileStatementSynchronized
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// static|abstract|const|public|protected|private 文
		void CompileStatementAccessModifier
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// var|void|boolean|byte|short|char|int|long|float|double 文
		void CompileStatementVar
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// this|super 文
		void CompileStatementExpression
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// デバッグポイント
		void CompileStatementDebugPoint
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
		// 不正文
		void CompileStatementInvalid
			( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ポインタオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSPointerMapper	: public ECSSakura2::Object
	{
	public:
		RSArrayBuffer *	m_pBuf ;
		size_t			m_iOffset ;
		size_t			m_nLimit ;
		atomic_int_t	m_nRef ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSPointerMapper, Object )
		// 構築関数
		RSPointerMapper( void ) ;
		// 消滅関数
		virtual ~RSPointerMapper( void ) ;
		// メモリマッピング
		virtual ECSSakura2Processor::LinearAddressCache *
				GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

}

// void __nrs_add_ref( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_add_ref) ;

// void __nrs_release_ref( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_release_ref) ;

// void __nrs_add_ptr_ref( void * ptr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_add_ptr_ref) ;

// void __nrs_release_ptr_ref( void * ptr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_release_ptr_ref) ;

// void * __nrs_realize_pointer( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_realize_pointer) ;

// int64 __nrs_realize_integer( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_realize_integer) ;

// double __nrs_realize_real_number( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_realize_real_number) ;

// void * __nrs_bound_pointer( void * ptr, int bytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_bound_pointer) ;

// void * __nrs_create_pointer( int bytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_create_pointer) ;

// void * __nrs_new_Array( void * cls, int limit ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_Array) ;

// void * __nrs_new_String( const char * pszInit ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_String) ;

// void * __nrs_new_Object( void * cls, int nArgCount, ... ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_Object) ;

// void * __nrs_new_Pointer( void * cls, int nArgCount, ... ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_Pointer) ;

// void * __nrs_new_pointer( void * cls, void * ptr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_pointer) ;

// void * __nrs_new_integer( int num ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_integer) ;

// void * __nrs_new_real_number( double num ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_new_real_number) ;

// void * __nrs_cast_object( void * cls, void * obj, int castMethod ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_cast_object) ;

// void * __nrs_clone_object( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_clone_object) ;

// void * __nrs_call_function_virtual
//	( void * strFuncName, int iVirtual, void * objThis, int nArgCount, ... ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_call_function_virtual) ;

// void * __nrs_call_function_obj
//	( void * objFunc, void * objThis, int nArgCount, ... ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_call_function_obj) ;

// void * __nrs_call_function_proto
//	( void * protoFunc, void * objThis, int nArgCount, ... ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_call_function_proto) ;

// void __nrs_set_currrent_position( void * prth, int index ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_set_currrent_position) ;

// void __nrs_set_exception_position( void * prth, int index ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_set_exception_position) ;

// bool __nrs_is_exception() ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_is_exception) ;

// void __nrs_throw_exception( void * strMsg, void * strClass ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_throw_exception) ;

// void * __nrs_get_member_as( void * obj, void * strName ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_get_member_as) ;

// void * __nrs_get_element_as( void * obj, void * objIndex ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_get_element_as) ;

// bool __nrs_operator_boolean( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_boolean) ;

// void * __nrs_operator_plus( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_plus) ;

// void * __nrs_operator_negate( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_negate) ;

// void * __nrs_operator_bit_not( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_bit_not) ;

// void * __nrs_operator_increment( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_increment) ;

// void * __nrs_operator_decrement( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_decrement) ;

// void * __nrs_operator_logical_not( void * obj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_logical_not) ;

// void * __nrs_operator_add( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_add) ;

// void * __nrs_operator_sub( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_sub) ;

// void * __nrs_operator_mul( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_mul) ;

// void * __nrs_operator_div( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_div) ;

// void * __nrs_operator_mod( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_mod) ;

// void * __nrs_operator_bit_and( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_bit_and) ;

// void * __nrs_operator_bit_or( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_bit_or) ;

// void * __nrs_operator_bit_xor( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_bit_xor) ;

// void * __nrs_operator_shift_right( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_shift_right) ;

// void * __nrs_operator_shift_left( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_shift_left) ;

// void * __nrs_operator_shift_right_arithmetic( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_shift_right_arithmetic) ;

// void * __nrs_operator_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_equal) ;

// void * __nrs_operator_not_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_not_equal) ;

// void * __nrs_operator_less_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_less_equal) ;

// void * __nrs_operator_less_than( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_less_than) ;

// void * __nrs_operator_grater_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_grater_equal) ;

// void * __nrs_operator_grater_than( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_grater_than) ;

// void * __nrs_operator_pointer_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_pointer_equal) ;

// void * __nrs_operator_pointer_not_equal( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_pointer_not_equal) ;

// void * __nrs_operator_logical_and( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_logical_and) ;

// void * __nrs_operator_logical_or( void * obj, void * obj2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_logical_or) ;

// void * __nrs_operator_move( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move) ;

// void * __nrs_operator_move_add( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_add) ;

// void * __nrs_operator_move_sub( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_sub) ;

// void * __nrs_operator_move_mul( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_mul) ;

// void * __nrs_operator_move_div( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_div) ;

// void * __nrs_operator_move_mod( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_mod) ;

// void * __nrs_operator_move_bit_and( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_bit_and) ;

// void * __nrs_operator_move_bit_or( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_bit_or) ;

// void * __nrs_operator_move_bit_xor( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_bit_xor) ;

// void * __nrs_operator_move_shift_right( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_shift_right) ;

// void * __nrs_operator_move_shift_left( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_shift_left) ;

// void * __nrs_operator_move_shift_left_arithmetic( void * obj, void * src ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_move_shift_left_arithmetic) ;

// bool __nrs_operator_instance_of( void * obj, void * cls ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_operator_instance_of) ;

// bool __nrs_ptr_operator_equal( void * ptr, void * ptr2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_ptr_operator_equal) ;

// bool __nrs_ptr_operator_not_equal( void * ptr, void * ptr2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(__nrs_ptr_operator_not_equal) ;


#endif
