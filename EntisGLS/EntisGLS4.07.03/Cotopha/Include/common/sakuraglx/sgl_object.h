
#if	!defined(__SAKURAGLX_SGL_OBJECT_H__)
#define	__SAKURAGLX_SGL_OBJECT_H__	1

#include <sakura/ssys_reference_array.h>


//////////////////////////////////////////////////////////////////////////////
// クラス情報定義マクロ
//////////////////////////////////////////////////////////////////////////////

#define	SGL_DECLARE_CLASS_INFO(class_name,parent_class)	\
	ESL_DECLARE_CLASS_INFO(class_name,parent_class)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DECLARE_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_DECLARE_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DECLARE_CLASS_INFO2_NV(class_name,parent_class1,parent_class2)	\
	ESL_DECLARE_CLASS_INFO2_NONEW(class_name,parent_class1,parent_class2)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DECLARE_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DECLARE_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DECLARE_CLASS_INFO4(class_name,parent_class1,parent_class2,parent_class3,parent_class4)	\
	ESL_DECLARE_CLASS_INFO4(class_name,parent_class1,parent_class2,parent_class3,parent_class4)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DECLARE_CLASS_INFO3_NV(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DECLARE_CLASS_INFO3_NONEW(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_DECLARE_CLASS_INFO(class_name,parent_class)	\
	ESL_DLL_DECLARE_CLASS_INFO(class_name,parent_class)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_DECLARE_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_DLL_DECLARE_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_DECLARE_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DLL_DECLARE_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DECLARE_CLASS_NEW_OBJECT(class_name)

#if	defined(__COTOPHA__)
#define	SGL_IMPLEMENT_CLASS_INFO(class_name,parent_class)	\
	ESL_IMPLEMENT_CLASS_INFO(class_name,parent_class)	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_IMPLEMENT_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_IMPLEMENT_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#else
#define	SGL_IMPLEMENT_CLASS_INFO(class_name,parent_class)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST( class_name, parent_class )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2( class_name, parent_class1, parent_class2 )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST3( class_name, parent_class1, parent_class2, parent_class3 )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO4(class_name,parent_class1,parent_class2,parent_class3,parent_class4)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST4( class_name, parent_class1, parent_class2, parent_class3, parent_class4 )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO_CAST(class_name,parent_class1,cast_member)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST_OBJ( class_name, parent_class1, cast_member )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_IMPLEMENT_CLASS_INFO2_CAST(class_name,parent_class1,parent_class2,cast_member)	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2_OBJ( class_name, parent_class1, parent_class2, cast_member )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_IMPLEMENT_CLASS_INFO(class_name,parent_class)	\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST( class_name, parent_class )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_IMPLEMENT_CLASS_INFO2(class_name,parent_class1,parent_class2)	\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2( class_name, parent_class1, parent_class2 )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#define	SGL_DLL_IMPLEMENT_CLASS_INFO3(class_name,parent_class1,parent_class2,parent_class3)	\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, SGLObject::RegisterObjectCreator(#class_name,reinterpret_cast<ESLRuntimeClass::PFUNC_NEW_OBJECT>(&class_name::NewESLObject)) )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST3( class_name, parent_class1, parent_class2, parent_class3 )	\
	ESL_IMPLEMENT_CLASS_NEW_OBJECT(class_name)

#endif


namespace SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 基底クラス
	//////////////////////////////////////////////////////////////////////////

	class	SGLObject : public SSystem::SObject
	{
	protected:
		struct	ObjectCreatorPair
		{
			const char *						pszType ;
			ESLRuntimeClass::PFUNC_NEW_OBJECT	pfnNewObj ;
			ObjectCreatorPair *					pNext ;
			ESL_FUNCPTR_FREE					pfnFreePair ;
		} ;
		static ESL_DLL_EXPORT ObjectCreatorPair *	s_pFirstObjectCreatorPair ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SakuraGL::SGLObject, SObject )
		// 複製（可能なら）
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		// 復元後処理
		virtual SGLError OnAfterRestore( void ) ;

	public:
		// 実行時クラス名からクラスオブジェクト生成
		static ESLObject * NewObject( const wchar_t * pszType ) ;
		// クラスオブジェクト保存
		static SGLError SaveObject
				( SGLObject * pObj, SSystem::SFileInterface& file ) ;
		// クラスオブジェクト復元
		static SGLObject * LoadObject( SSystem::SFileInterface& file ) ;

	public:
	#if	!defined(__COTOPHA__)
		// 実行時オブジェクト生成関数登録
		ESL_DLL_EXPORT static ESLRuntimeClass::PFUNC_NEW_OBJECT
			RegisterObjectCreator
				( const char * pszType,
					ESLRuntimeClass::PFUNC_NEW_OBJECT pfnNewObj ) ;
		// 実行時オブジェクト生成関数登録情報削除
		static void UnregisterAllObjectCreator( void ) ;
	#endif
	} ;

	// 動的型キャスト（失敗した場合には delete し NULL を返す）
	template <class T> inline T * SGLSmartCast( ESLObject * pObj )
	{
		if ( pObj == NULL )
		{
			return	NULL ;
		}
		T *	pT = (typename T::ClassPointerType)
					(pObj->DynamicCast( T::m_RuntimeClass )) ;
		if ( pT == NULL )
		{
			delete	pObj ;
		}
		return	pT ;
	}
	// 配列バッファ保存
	template <class T> inline SGLError SaveArray
		( SSystem::SFileInterface& file, const SSystem::SArray<T>& arr )
	{
		uint32_t	nCount = (uint32_t) arr.GetLength() ;
		file.Write( &nCount, sizeof(uint32_t) ) ;
		if ( file.Write
			( arr.GetConstArray(), nCount * sizeof(T) ) < nCount * sizeof(T) )
		{
			return	sglErrFailed ;
		}
		return	sglErrSuccess ;
	}
	// 配列バッファ復元
	template <class T> inline SGLError LoadArray
		( SSystem::SFileInterface& file, SSystem::SArray<T>& arr )
	{
		uint32_t	nCount ;
		if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		arr.SetLength( nCount ) ;
		if ( file.Read
			( arr.GetArray(), nCount * sizeof(T) ) < nCount * sizeof(T) )
		{
			arr.FinishArray() ;
			return	sglErrFailed ;
		}
		arr.FinishArray() ;
		return	sglErrSuccess ;
	}
	// 構造体保存
	template <class T> inline SGLError SaveStructure
		( SSystem::SFileInterface& file, const T& obj )
	{
		uint32_t	nBytes = sizeof(T) ;
		file.Write( &nBytes, sizeof(uint32_t) ) ;
		if ( file.Write( &obj, nBytes ) < nBytes )
		{
			return	sglErrFailed ;
		}
		return	sglErrSuccess ;
	}
	// 構造体読み込み
	template <class T> inline SGLError LoadStructure
		( SSystem::SFileInterface& file, T& obj )
	{
		uint32_t	nBytes ;
		if( file.Read( &nBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		size_t	nReadBytes = (nBytes < sizeof(T)) ? nBytes : sizeof(T) ;
		if ( file.Read( &obj, nReadBytes ) < nReadBytes )
		{
			return	sglErrFailed ;
		}
		if ( nReadBytes < nBytes )
		{
			file.Seek( nBytes - nReadBytes, SSystem::SFileInterface::FromCurrent ) ;
		}
		return	sglErrSuccess ;
	}
	// オブジェクト配列複製
	template <class T> inline SGLError DuplicateObjectArray
		( SSystem::SObjectArray<T>& dst,
				const SSystem::SObjectArray<T>& src )
	{
		size_t	nCount = src.GetLength() ;
		dst.SetLength( nCount ) ;
		//
		T*const*	pArray = src.GetConstArray() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( pArray[i] != NULL )
			{
				dst.SetAt( i, SGLSmartCast<T>( pArray[i]->DuplicateObject() ) ) ;
			}
			else
			{
				dst.SetAt( i, NULL ) ;
			}
		}
		return	sglErrSuccess ;
	}
	template <class T> inline SGLError DuplicateReferenceArray
		( SSystem::SReferenceArray<T>& dst,
				const SSystem::SReferenceArray<T>& src )
	{
		size_t	nCount = src.GetLength() ;
		dst.SetLength( nCount ) ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			T *	pObj = src.GetAt(i) ;
			if ( pObj != NULL )
			{
				pObj = SGLSmartCast<T>( pObj->DuplicateObject() ) ;
			}
			dst.SmartSetAt( i, pObj ) ;
		}
		return	sglErrSuccess ;
	}
	// オブジェクト配列保存
	template <class T> inline SGLError SaveObjectArray
		( SSystem::SFileInterface& file,
				const SSystem::SObjectArray<T>& arr )
	{
		uint32_t	nCount = (uint32_t) arr.GetLength() ;
		file.Write( &nCount, sizeof(uint32_t) ) ;
		//
		T*const*	pArray = arr.GetConstArray() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLError	err = SGLObject::SaveObject( pArray[i], file ) ;
			if ( err )
			{
				return	err ;
			}
		}
		return	sglErrSuccess ;
	}
	template <class T> inline SGLError SaveReferenceArray
		( SSystem::SFileInterface& file,
				const SSystem::SReferenceArray<T>& arr )
	{
		uint32_t	nCount = (uint32_t) arr.GetLength() ;
		file.Write( &nCount, sizeof(uint32_t) ) ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLError	err = SGLObject::SaveObject( arr.GetAt(i), file ) ;
			if ( err )
			{
				return	err ;
			}
		}
		return	sglErrSuccess ;
	}
	// オブジェクト配列復元
	template <class T> inline SGLError LoadObjectArray
		( SSystem::SFileInterface& file,
				SSystem::SObjectArray<T>& arr )
	{
		uint32_t	nCount ;
		if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		arr.SetLength( nCount ) ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			arr.SetAt( i, SGLSmartCast<T>( SGLObject::LoadObject( file ) ) ) ;
		}
		return	sglErrSuccess ;
	}
	template <class T> inline SGLError LoadReferenceArray
		( SSystem::SFileInterface& file,
				SSystem::SReferenceArray<T>& arr )
	{
		uint32_t	nCount ;
		if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		arr.RemoveAll() ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			arr.SmartAdd
				( SGLSmartCast<T>( SGLObject::LoadObject(file) ) ) ;
		}
		return	sglErrSuccess ;
	}
	// オブジェクト配列復元後処理
	template <class T> inline SGLError CommitObjectArrayAfterRestore
		( SSystem::SObjectArray<T>& arr )
	{
		size_t	nCount = arr.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			T *	pObj = arr.GetAt( i ) ;
			if ( pObj != NULL )
			{
				pObj->OnAfterRestore() ;
			}
		}
		return	sglErrSuccess ;
	}
	template <class T> inline SGLError CommitReferenceArrayAfterRestore
		( SSystem::SReferenceArray<T>& arr )
	{
		size_t	nCount = arr.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			T *	pObj = arr.GetAt( i ) ;
			if ( pObj != NULL )
			{
				pObj->OnAfterRestore() ;
			}
		}
		return	sglErrSuccess ;
	}


	//////////////////////////////////////////////////////////////////////////
	// ポインタ保存用マッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLObjectSavingMapper	: public SSystem::SObject
	{
	protected:
		class	Entry
		{
		public:
			SSystem::SString	m_strID ;
			ESLObject *			m_pPointer ;
		} ;
		SSystem::SObjectArray<Entry>	m_mapper ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::SGLObjectSavingMapper, SObject )
		// 構築関数
		SGLObjectSavingMapper( void ) ;
		// 消滅関数
		virtual ~SGLObjectSavingMapper( void ) ;

	public:
		// ポインタ登録
		SGLError RegisterObject( const wchar_t * pwszID, ESLObject * pObj ) ;
		// 登録識別子検索
		virtual const wchar_t * GetIdentityOf( ESLObject * pObj ) const ;
		// 登録ポインタ検索
		virtual ESLObject * GetObjectOf( const wchar_t * pwszID ) const ;

	public:
		// クラスオブジェクト保存
		SGLError SaveObject
			( SSystem::SFileInterface& file, SGLObject * pObj, bool fEnableNewObject ) ;
		// クラスオブジェクト復元
		SGLObject * LoadObject
			( SSystem::SFileInterface& file, bool fEnableNewObject ) ;

	public:
		// 現在のスレッドに関連付けられたマッパー取得
		static SGLObjectSavingMapper * GetCurrent( void ) ;
		// 現在のスレッドに関連付ける
		void AttachCurrentThread( void ) ;
		// 現在のスレッドから解除する
		void DetachCurrentThread( void ) ;

	} ;

}

#endif
