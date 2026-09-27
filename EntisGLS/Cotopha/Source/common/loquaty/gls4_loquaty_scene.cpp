
#include <loquaty/gls4_loquaty.h>


//////////////////////////////////////////////////////////////////////////////
// クラス情報を ParameterProperty へ受け渡す実装
//////////////////////////////////////////////////////////////////////////////

const size_t	S3DSceneCustomProperty::s_bytesRawType
					[S3DSceneCustomProperty::typeDataCount] =
{
	sizeof(LMatrix3d),
	sizeof(LMatrix3),
	sizeof(S3DDQuaternion),
	sizeof(S3DQuaternion),
	sizeof(LVector3d),
	sizeof(LVector3),
	sizeof(LMatrix4d),
	sizeof(LMatrix4),
	sizeof(LVector4d),
	sizeof(LVector4),
	sizeof(LVector2d),
	sizeof(LVector2),
	sizeof(LARGB8),
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Loquaty::S3DSceneCustomProperty, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneCustomProperty::S3DSceneCustomProperty( void )
	: m_pObj( nullptr ),
		m_ownObj( false ),
		m_pImageClass( nullptr ),
		m_pAudioClass( nullptr ),
		m_pModelClass( nullptr ),
		m_pPoseClass( nullptr ),
		m_pSceneItemClass( nullptr )
{
}

S3DSceneCustomProperty::S3DSceneCustomProperty
		( LVirtualMachine& vm, const LObjPtr& pObj )
	: S3DSceneCustomProperty()
{
	AttachObject( vm, pObj ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneCustomProperty::~S3DSceneCustomProperty( void )
{
	if ( m_ownObj )
	{
		std::shared_ptr<LReference>
			pItemRef = LNativeObj::GetNative<LReference>( m_pObj ) ;
		if ( pItemRef != nullptr )
		{
			pItemRef->DetachReference() ;
		}
		LObject::ReleaseRef( m_pObj ) ;
	}
}

// デフォルト構築関数の場合、後でオブジェクトを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::AttachObject
			( LVirtualMachine& vm, const LObjPtr& pObj )
{
	ESLAssert( !m_ownObj || (m_pObj == nullptr) ) ;

	m_pTask = vm.new_Task() ;
	m_pObj = pObj.Ptr() ;
	m_ownObj = false ;
	m_strObjClassPath = m_pObj->GetClass()->GetFullClassName() ;
	m_pImageClass = vm.GetClassPathAs( L"EntisGLS4.Image" ) ;
	m_pAudioClass = vm.GetClassPathAs( L"EntisGLS4.AudioPlayer" ) ;
	m_pModelClass = vm.GetClassPathAs( L"EntisGLS4.ModelBuffer" ) ;
	m_pPoseClass = vm.GetClassPathAs( L"EntisGLS4.ModelPose" ) ;
	m_pSceneItemClass = vm.GetClassPathAs( L"EntisGLS4.SceneItem" ) ;

	BuildPropertyList() ;
}

// オブジェクトの参照を保持する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::OwnObject( void )
{
	if ( !m_ownObj )
	{
		m_ownObj = true ;
		LObject::AddRef( m_pObj ) ;
	}
}

// クラス情報のコメントからパラメータ情報を構築する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::BuildPropertyList( void )
{
	LClass *	pClass = m_pObj->GetClass() ;
	ESLAssert( pClass != nullptr ) ;
	m_props.RemoveAll() ;

	LPointerClass *	pPtrClass = dynamic_cast<LPointerClass*>( pClass ) ;
	if ( pPtrClass != nullptr )
	{
		if ( pPtrClass->GetBufferType().IsStructure() )
		{
			pClass = pPtrClass->GetBufferType().GetStructureClass() ;
		}
	}

	LObjPtr	pObj = pClass->GetPrototypeObject() ;
	if ( pObj != nullptr )
	{
		for ( size_t i = 0; i < pObj->GetElementCount(); i ++ )
		{
			LType		type = pObj->GetElementTypeAt( i ) ;
			LString		strName ;
			Property *	pProp =
				CreateProperty
					( type, pObj->GetElementNameAt( strName, i ) ) ;
			if ( pProp == nullptr )
			{
				continue ;
			}
			pProp->m_iElement = (ssize_t) i ;
			m_props.Add( pProp ) ;
		}
	}

	const LArrangementBuffer&	protoBuf = pClass->GetProtoArrangemenet() ;
	std::vector<LString>		nameVars ;
	protoBuf.GetOrderedNameList( nameVars ) ;

	for ( size_t i = 0; i < nameVars.size(); i ++ )
	{
		LArrangement::Desc	desc ;
		if ( protoBuf.GetDescAs( desc, nameVars.at(i).c_str() ) )
		{
			Property *	pProp =
				CreateProperty( desc.m_type, nameVars.at(i).c_str() ) ;
			if ( pProp == nullptr )
			{
				continue ;
			}
			pProp->m_descBuf = desc ;
			m_props.Add( pProp ) ;
		}
	}

	m_params.RemoveAll() ;
	m_params.SetLimit( m_props.GetLength() ) ;
	for ( size_t i = 0; i < m_props.GetLength(); i ++ )
	{
		Property *	pProp = m_props.GetAt( i ) ;
		ESLAssert( pProp != nullptr ) ;

		S3DSceneComposer::ParamEntry	entry ;
		entry.id = pProp->m_strPropID.c_str() ;
		entry.type = pProp->m_paramType ;
		entry.attr = pProp->m_attrFlags ;
		entry.name = pProp->m_strName.c_str() ;
		entry.desc = pProp->m_strDescription.c_str() ;
		entry.minRange = pProp->m_minRange ;
		entry.maxRange = pProp->m_maxRange ;
		m_params.Add( entry ) ;
	}
}

S3DSceneCustomProperty::Property *
	S3DSceneCustomProperty::CreateProperty
		( const LType& type, const wchar_t * pwszVarName )
{
	if ( (type.GetAccessModifier() != LType::modifierPublic)
		|| (type.GetComment() == nullptr) )
	{
		return	nullptr ;
	}
	LType::LComment *	pComment = type.GetComment() ;
	if ( pComment->m_xmlDoc == nullptr )
	{
		LXMLDocParser	xmlParser = *pComment ;
		pComment->m_xmlDoc = xmlParser.ParseDocument() ;
		if ( xmlParser.GetErrorCount() > 0 )
		{
			pComment->m_xmlDoc = nullptr ;
			return	nullptr ;
		}
	}
	LXMLDocPtr	xmlName = pComment->m_xmlDoc->GetTagAs( L"name" ) ;
	if ( xmlName == nullptr )
	{
		return	nullptr ;
	}
	Property *	pProp = new Property ;
	pProp->m_strName = xmlName->GetTextElement() ;
	if ( pProp->m_strName.IsEmpty() )
	{
		delete	pProp ;
		return	nullptr ;
	}
	pProp->m_pComment = pComment ;
	pProp->m_type = type ;
	pProp->m_paramType = S3DSceneComposer::typeInvalid ;
	pProp->m_attrFlags = S3DSceneComposer::attrCategory7 ;
	pProp->m_strPropID = pwszVarName ;

	if ( type.IsPrimitive() )
	{
		if ( type.IsBoolean() )
		{
			pProp->m_typeRaw = typeBoolean ;
		}
		else if ( type.IsInteger() )
		{
			pProp->m_typeRaw = typeInteger ;
		}
		else
		{
			pProp->m_typeRaw = typeNumber ;
		}
	}
	else
	{
		if ( type.GetClass() == nullptr )
		{
			delete	pProp ;
			return	nullptr ;
		}
		static const struct
		{
			const wchar_t *	pwszTypeName ;
			RawType			typeRaw ;
		}
		s_rawTypes[] =
		{
			{ L"Matrix3d", typeMatrix3d },
			{ L"Matrix3", typeMatrix3f },
			{ L"Quaterniond", typeQuaterniond },
			{ L"Quaternion", typeQuaternionf },
			{ L"Vector3d", typeVector3d },
			{ L"Vector3", typeVector3f },
			{ L"Matrix4d", typeMatrix4d },
			{ L"Matrix4", typeMatrix4f },
			{ L"Vector4d", typeVector4d },
			{ L"Vector4", typeVector4f },
			{ L"Vector2d", typeVector2d },
			{ L"Vector2", typeVector2f },
			{ L"ARGB8", typeARGB8 },
			{ L"String", typeString },
			{ L"EntisGLS4.Image", typeImage },
			{ L"EntisGLS4.AudioPlayer", typeAudioPlayer },
			{ L"EntisGLS4.ModelBuffer", typeModelBuffer },
			{ L"EntisGLS4.ModelPose", typeModelPose },
			{ L"EntisGLS4.SceneItem", typeSceneItem },
			{ nullptr, typeNull },
		} ;
		LString	strClassName = type.GetClass()->GetFullClassName() ;
		pProp->m_typeRaw = typeNull ;
		for ( int i = 0; s_rawTypes[i].pwszTypeName != nullptr; i ++ )
		{
			if ( strClassName == s_rawTypes[i].pwszTypeName )
			{
				pProp->m_typeRaw = s_rawTypes[i].typeRaw ;
				break ;
			}
		}
		if ( pProp->m_typeRaw == typeNull )
		{
			if ( type.GetClass()->GetVirtualVectorOf( m_pSceneItemClass ) == nullptr )
			{
				delete	pProp ;
				return	nullptr ;
			}
			else
			{
				pProp->m_typeRaw = typeSceneItem ;
			}
		}
	}
	LString		strSubType ;
	LXMLDocPtr	xmlSubType = pComment->m_xmlDoc->GetTagAs( L"sub_type" ) ;
	if ( xmlSubType != nullptr )
	{
		strSubType = xmlSubType->GetTextElement() ;
	}
	LXMLDocPtr	xmlRange = pComment->m_xmlDoc->GetTagAs( L"range" ) ;
	LXMLDocPtr	xmlSelector = pComment->m_xmlDoc->GetTagAs( L"selector" ) ;
	pProp->m_pxmlSelector = xmlSelector ;

	switch ( pProp->m_typeRaw )
	{
	case	typeMatrix3d:
	case	typeMatrix3f:
		if ( strSubType == L"rotation" )
		{
			pProp->m_paramType = S3DSceneComposer::typeRotation ;
		}
		else
		{
			pProp->m_paramType = S3DSceneComposer::typeMatrix ;
		}
		break ;

	case	typeQuaterniond:
	case	typeQuaternionf:
		pProp->m_paramType = S3DSceneComposer::typeRotation ;
		break ;

	case	typeVector3d:
	case	typeVector3f:
		if ( strSubType == L"direction" )
		{
			pProp->m_paramType = S3DSceneComposer::typeDirection ;
		}
		else if ( strSubType == L"zoom" )
		{
			pProp->m_paramType = S3DSceneComposer::typeZoom ;
		}
		else if ( strSubType == L"color" )
		{
			pProp->m_paramType = S3DSceneComposer::typeColor ;
		}
		else
		{
			pProp->m_paramType = S3DSceneComposer::typePosition ;
		}
		break ;

	case	typeMatrix4d:
	case	typeMatrix4f:
		pProp->m_paramType = S3DSceneComposer::typeMatrix4 ;
		break ;

	case	typeVector4d:
	case	typeVector4f:
		pProp->m_paramType = S3DSceneComposer::typeVector4 ;
		break ;

	case	typeVector2d:
	case	typeVector2f:
		pProp->m_paramType = S3DSceneComposer::typeVector2 ;
		break ;

	case	typeARGB8:
		pProp->m_paramType = S3DSceneComposer::typeColor ;
		break ;

	case	typeBoolean:
		pProp->m_paramType = S3DSceneComposer::typeBoolean ;
		break ;

	case	typeInteger:
		pProp->m_paramType = S3DSceneComposer::typeInteger ;
		if ( xmlSelector != nullptr )
		{
			pProp->m_paramType = S3DSceneComposer::typeSelector ;
			pProp->m_attrFlags |= S3DSceneComposer::attrStringEnumeration
								| S3DSceneComposer::attrUIOnlyEnumeration ;
			for ( size_t i = 0; i < xmlSelector->GetElementCount(); i ++ )
			{
				LXMLDocPtr	xmlEnum = xmlSelector->GetElementAt( i ) ;
				if ( (xmlEnum != nullptr)
					&& (xmlEnum->GetTag() == L"enum") )
				{
					EnumEntry	entry ;
					entry.m_strName = xmlEnum->GetAttrString( L"name" ) ;
					entry.m_numLong = xmlEnum->GetAttrLong( L"num" ) ;
					pProp->m_vSelEntries.push_back( entry ) ;
				}
			}
		}
		else if ( type.IsUnsignedInteger() )
		{
			pProp->m_attrFlags |= S3DSceneComposer::attrFlagSetInteger ;
		}
		break ;

	case	typeNumber:
		pProp->m_paramType = S3DSceneComposer::typeScalar ;
		if ( xmlRange != nullptr )
		{
			pProp->m_attrFlags |= S3DSceneComposer::attrUIScalarSlider ;
			pProp->m_minRange = xmlRange->GetAttrNumber( L"min" ) ;
			pProp->m_maxRange = xmlRange->GetAttrNumber( L"max" ) ;
		}
		break ;

	case	typeString:
		pProp->m_paramType = S3DSceneComposer::typeCommand ;
		if ( xmlSelector != nullptr )
		{
			pProp->m_paramType = S3DSceneComposer::typeSelector ;
			pProp->m_attrFlags |= S3DSceneComposer::attrStringEnumeration
								| S3DSceneComposer::attrUIOnlyEnumeration ;
			for ( size_t i = 0; i < xmlSelector->GetElementCount(); i ++ )
			{
				LXMLDocPtr	xmlEnum = xmlSelector->GetElementAt( i ) ;
				if ( (xmlEnum != nullptr)
					&& (xmlEnum->GetTag() == L"enum") )
				{
					EnumEntry	entry ;
					entry.m_strName = xmlEnum->GetAttrString( L"name" ) ;
					entry.m_strValue = xmlEnum->GetAttrString( L"str" ) ;
					pProp->m_vSelEntries.push_back( entry ) ;
				}
			}
		}
		break ;

	case	typeImage:
	case	typeAudioPlayer:
	case	typeModelBuffer:
	case	typeModelPose:
	case	typeSceneItem:
		pProp->m_paramType = S3DSceneComposer::typeSelector ;
		pProp->m_attrFlags |= S3DSceneComposer::attrStringEnumeration ;
		break ;

	default:
		delete	pProp ;
		return	nullptr ;
	}

	LXMLDocPtr	xmlID = pComment->m_xmlDoc->GetTagAs( L"id" ) ;
	if ( xmlID != nullptr )
	{
		pProp->m_strPropID = xmlID->GetTextElement() ;
		if ( pProp->m_strPropID.IsEmpty() )
		{
			delete	pProp ;
			return	nullptr ;
		}
	}
	LXMLDocPtr	xmlDesc = pComment->m_xmlDoc->GetTagAs( L"desc" ) ;
	if ( xmlDesc != nullptr )
	{
		pProp->m_strDescription = xmlDesc->GetTextElement() ;
	}
	return	pProp ;
}

// 変数ポインタ取得
//////////////////////////////////////////////////////////////////////////////
void * S3DSceneCustomProperty::GetPropPointer
			( S3DSceneCustomProperty::Property& prop ) const
{
	ESLAssert( prop.m_typeRaw < typeDataCount ) ;
	if ( prop.m_typeRaw >= typeDataCount )
	{
		return	nullptr ;
	}
	if ( prop.m_iElement >= 0 )
	{
		return	m_pObj->GetElementPointerAt
					( (size_t) prop.m_iElement, s_bytesRawType[prop.m_typeRaw] ) ;
	}
	else
	{
		LPtr<LPointerObj>	pPtr( m_pObj->GetBufferPoiner() ) ;
		if ( pPtr != nullptr )
		{
			return	pPtr->GetPointer
				( prop.m_descBuf.m_location, s_bytesRawType[prop.m_typeRaw] ) ;
		}
	}
	return	nullptr ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneCustomProperty::GetMatrixParameter( size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		void *	pBuf = GetPropPointer( *pProp ) ;
		if ( pBuf != nullptr )
		{
			switch ( pProp->m_typeRaw )
			{
			case	typeMatrix3d:
				return	*((LMatrix3d*) pBuf) ;

			case	typeMatrix3f:
				return	S3DDMatrix( ((LMatrix3*) pBuf)->ToS3DMatrix() ) ;

			case	typeQuaterniond:
				return	S3DDMatrix( *((S3DDQuaternion*) pBuf) ) ;

			case	typeQuaternionf:
				return	S3DDMatrix( S3DMatrix( *((S3DQuaternion*) pBuf) ) ) ;

			default:
				break ;
			}
		}
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DSceneCustomProperty::GetVectorParameter( size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		void *	pBuf = GetPropPointer( *pProp ) ;
		if ( pBuf != nullptr )
		{
			switch ( pProp->m_typeRaw )
			{
			case	typeVector3d:
				return	*((LVector3d*) pBuf) ;

			case	typeVector3f:
				return	*((LVector3*) pBuf) ;

			case	typeARGB8:
				return	S3DSceneComposer::Parameter::
							VectorFromColor( *((LARGB8*) pBuf) ) ;

			default:
				break ;
			}
		}
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DSceneCustomProperty::GetScalarParameter( size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( pProp->m_iElement >= 0 )
		{
			return	m_pObj->GetElementDoubleAt( (size_t) pProp->m_iElement ) ;
		}
		else
		{
			LPtr<LPointerObj>	pPtr( m_pObj->GetBufferPoiner() ) ;
			if ( pPtr != nullptr )
			{
				return	pPtr->LoadDoubleAt
					( pProp->m_descBuf.m_location,
						pProp->m_descBuf.m_type.GetPrimitive() ) ;
			}
		}
	}
	return	0.0 ;
}

LLong S3DSceneCustomProperty::GetIntegerParameter( size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( pProp->m_iElement >= 0 )
		{
			return	m_pObj->GetElementLongAt( (size_t) pProp->m_iElement ) ;
		}
		else
		{
			LPtr<LPointerObj>	pPtr( m_pObj->GetBufferPoiner() ) ;
			if ( pPtr != nullptr )
			{
				return	pPtr->LoadIntegerAt
					( pProp->m_descBuf.m_location,
						pProp->m_descBuf.m_type.GetPrimitive() ) ;
			}
		}
	}
	return	0 ;
}

bool S3DSceneCustomProperty::GetBooleanParameter( size_t iParam ) const
{
	return	(S3DSceneCustomProperty::GetIntegerParameter( iParam ) != 0) ;
}

const wchar_t * S3DSceneCustomProperty::GetCommandParameter( size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( (pProp->m_iElement >= 0)
			&& (pProp->m_typeRaw >= typeFirstObject) )
		{
			if ( pProp->m_typeRaw == typeString )
			{
				pProp->m_strTempValue =
					m_pObj->GetElementStringAt( (size_t) pProp->m_iElement ) ;
			}
			if ( pProp->m_pxmlSelector != nullptr )
			{
				for ( size_t i = 0; i < pProp->m_vSelEntries.size(); i ++ )
				{
					const EnumEntry&	entry = pProp->m_vSelEntries.at(i) ;
					if ( entry.m_strValue == pProp->m_strTempValue )
					{
						return	entry.m_strName.c_str() ;
					}
				}
				return	nullptr ;
			}
			else
			{
				return	pProp->m_strTempValue.c_str() ;
			}
		}
		else if ( (pProp->m_typeRaw == typeInteger)
					&& (pProp->m_pxmlSelector != nullptr) )
		{
			LLong	num = S3DSceneCustomProperty::GetIntegerParameter( iParam ) ;
			for ( size_t i = 0; i < pProp->m_vSelEntries.size(); i ++ )
			{
				const EnumEntry&	entry = pProp->m_vSelEntries.at(i) ;
				if ( entry.m_numLong == num )
				{
					return	entry.m_strName.c_str() ;
				}
			}
			return	nullptr ;
		}
	}
	return	nullptr ;
}

size_t S3DSceneCustomProperty::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t iParam ) const
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp == nullptr )
	{
		return	0 ;
	}
	switch ( pProp->m_typeRaw )
	{
	case	typeMatrix4d:
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			LMatrix4d *	pBuf = (LMatrix4d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeMatrix4 ) ;
				*((S4DDMatrix*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeMatrix4f:
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			LMatrix4 *	pBuf = (LMatrix4*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeMatrix4 ) ;
				*((S4DDMatrix*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector4d:
		if ( nBufBytes == sizeof(S4DDVector) )
		{
			LVector4d *	pBuf = (LVector4d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector4 ) ;
				*((S4DDVector*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector4f:
		if ( nBufBytes == sizeof(S4DDVector) )
		{
			LVector4 *	pBuf = (LVector4*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector4 ) ;
				*((S4DDVector*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector2d:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			LVector2d *	pBuf = (LVector2d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector2 ) ;
				*((S2DDVector*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector2f:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			LVector2 *	pBuf = (LVector2*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector2 ) ;
				*((S2DDVector*)pDst) = *pBuf ;
				return	nBufBytes ;
			}
		}
		break ;

	default:
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		void *	pBuf = GetPropPointer( *pProp ) ;
		if ( pBuf != nullptr )
		{
			switch ( pProp->m_typeRaw )
			{
			case	typeMatrix3d:
				*((LMatrix3d*) pBuf) = mat ;
				break ;

			case	typeMatrix3f:
				*((LMatrix3*) pBuf) = LMatrix3( S3DMatrix( mat ) ) ;
				break ;

			case	typeQuaterniond:
				*((S3DDQuaternion*) pBuf) = mat ;
				break ;

			case	typeQuaternionf:
				*((S3DQuaternion*) pBuf) = S3DMatrix( mat ) ;
				break ;

			default:
				break ;
			}
		}
	}
}

void S3DSceneCustomProperty::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		void *	pBuf = GetPropPointer( *pProp ) ;
		if ( pBuf != nullptr )
		{
			switch ( pProp->m_typeRaw )
			{
			case	typeVector3d:
				*((LVector3d*) pBuf) = vec ;
				break ;

			case	typeVector3f:
				*((LVector3*) pBuf) = vec ;
				break ;

			case	typeARGB8:
				*((LARGB8*) pBuf) =
					S3DSceneComposer::Parameter::ColorFromVector( vec ) ;

			default:
				break ;
			}
		}
	}
}

void S3DSceneCustomProperty::SetScalarParameter( size_t iParam, double s )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( pProp->m_iElement >= 0 )
		{
			m_pObj->SetElementDoubleAt( (size_t) pProp->m_iElement, s ) ;
		}
		else
		{
			LPtr<LPointerObj>	pPtr( m_pObj->GetBufferPoiner() ) ;
			if ( pPtr != nullptr )
			{
				pPtr->StoreDoubleAt
					( pProp->m_descBuf.m_location,
						pProp->m_descBuf.m_type.GetPrimitive(), s ) ;
			}
		}
	}
}

void S3DSceneCustomProperty::SetIntegerParameter( size_t iParam, LLong n )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( pProp->m_iElement >= 0 )
		{
			m_pObj->SetElementLongAt( (size_t) pProp->m_iElement, n ) ;
		}
		else
		{
			LPtr<LPointerObj>	pPtr( m_pObj->GetBufferPoiner() ) ;
			if ( pPtr != nullptr )
			{
				pPtr->StoreIntegerAt
					( pProp->m_descBuf.m_location,
						pProp->m_descBuf.m_type.GetPrimitive(), n ) ;
			}
		}
	}
}

void S3DSceneCustomProperty::SetBooleanParameter( size_t iParam, bool b )
{
	SetIntegerParameter( iParam, b ) ;
}

void S3DSceneCustomProperty::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp != nullptr )
	{
		if ( (pProp->m_iElement >= 0)
			&& (pProp->m_typeRaw >= typeFirstObject) )
		{
			if ( pProp->m_strTempValue != pwszCmd )
			{
				pProp->m_strTempValue = pwszCmd ;
				if ( pProp->m_typeRaw == typeString )
				{
					if ( pProp->m_pxmlSelector != nullptr )
					{
						for ( size_t i = 0; i < pProp->m_vSelEntries.size(); i ++ )
						{
							const EnumEntry&	entry = pProp->m_vSelEntries.at(i) ;
							if ( entry.m_strName == pwszCmd )
							{
								m_pObj->SetElementStringAt
									( (size_t) pProp->m_iElement, entry.m_strValue.c_str() ) ;
							break ;
							}
						}
					}
					else
					{
						m_pObj->SetElementStringAt( (size_t) pProp->m_iElement, pwszCmd ) ;
					}
				}
				else
				{
					UpdateReferenceParameter( iParam ) ;
				}
			}
		}
		else if ( (pProp->m_typeRaw == typeInteger)
					&& (pProp->m_pxmlSelector != nullptr) )
		{
			for ( size_t i = 0; i < pProp->m_vSelEntries.size(); i ++ )
			{
				const EnumEntry&	entry = pProp->m_vSelEntries.at(i) ;
				if ( entry.m_strName == pwszCmd )
				{
					S3DSceneCustomProperty::
						SetIntegerParameter( iParam, entry.m_numLong ) ;
					break ;
				}
			}
		}
	}
}

size_t S3DSceneCustomProperty::SetBinaryParameter
	( size_t iParam, const void * pSrc, size_t nBufBytes )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp == nullptr )
	{
		return	0 ;
	}
	switch ( pProp->m_typeRaw )
	{
	case	typeMatrix4d:
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			LMatrix4d *	pBuf = (LMatrix4d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeMatrix4 ) ;
				*pBuf = *((const S4DDMatrix*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeMatrix4f:
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			LMatrix4 *	pBuf = (LMatrix4*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeMatrix4 ) ;
				*pBuf = *((const S4DDMatrix*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector4d:
		if ( nBufBytes == sizeof(S4DDVector) )
		{
			LVector4d *	pBuf = (LVector4d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector4 ) ;
				*pBuf = *((const S4DDVector*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector4f:
		if ( nBufBytes == sizeof(S4DDVector) )
		{
			LVector4 *	pBuf = (LVector4*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector4 ) ;
				*pBuf = *((const S4DDVector*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector2d:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			LVector2d *	pBuf = (LVector2d*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector2 ) ;
				*pBuf = *((const S2DDVector*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	case	typeVector2f:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			LVector2 *	pBuf = (LVector2*) GetPropPointer( *pProp ) ;
			if ( pBuf != nullptr )
			{
				ESLAssert( m_params.At(iParam).type == S3DSceneComposer::typeVector2 ) ;
				*pBuf = *((const S2DDVector*)pSrc) ;
				return	nBufBytes ;
			}
		}
		break ;

	default:
		break ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneCustomProperty::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( pProp == nullptr )
	{
		return	false ;
	}
	if ( pProp->m_pxmlSelector != nullptr )
	{
		for ( size_t i = 0; i < pProp->m_vSelEntries.size(); i ++ )
		{
			aStrSet.Add( new SString( pProp->m_vSelEntries.at(i).m_strName.c_str() ) ) ;
		}
		return	true ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer == nullptr )
	{
		return	false ;
	}
	switch ( pProp->m_typeRaw )
	{
	case	typeImage:
		pComposer->GetAssets().EnumerateTextureStringSet( aStrSet ) ;
		return	true ;
	case	typeAudioPlayer:
		pComposer->GetAssets().EnumerateResourceIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(SGLAudioPlayer) ) ;
		S3DSceneComposer::ResourceAssets::SortStringSet( aStrSet ) ;
		return	true ;
	case	typeModelBuffer:
		pComposer->GetAssets().EnumerateResourceIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DModelBuffer) ) ;
		S3DSceneComposer::ResourceAssets::SortStringSet( aStrSet ) ;
		return	true ;
	case	typeModelPose:
		pComposer->GetAssets().EnumeratePoseStringSet( aStrSet ) ;
		S3DSceneComposer::ResourceAssets::SortStringSet( aStrSet ) ;
		return	true ;
	case	typeSceneItem:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				SPointerArray<S3DSceneComposer::ItemSerializer>	aItems ;
				pComp->EnumerateItemsAs
					( aItems, ESL_RUNTIME_CLASS(S3DSceneComposer::ItemSerializer) ) ;
				for ( size_t i = 0; i < aItems.GetLength(); i ++ )
				{
					S3DSceneComposer::ItemSerializer *	pItem = aItems.GetAt( i ) ;
					if ( pItem == nullptr )
					{
						continue ;
					}
					LClass *	pClass = m_pTask->GetClass()->VM().
											GetClassPathAs( pItem->GetLQClassName() ) ;
					if ( (pClass != nullptr)
						&& (pClass->GetVirtualVectorOf
								( pProp->m_type.GetClass() ) != nullptr) )
					{
						aStrSet.Add( new SString( pItem->GetItemIdentity() ) ) ;
					}
				}
			}
		}
		return	true ;
	default:
		break ;
	}
	return	false ;
}

// 参照リソース更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::UpdateReferenceParameter( size_t iParam )
{
	Property *	pProp = m_props.GetAt( iParam ) ;
	if ( (pProp == nullptr) || (pProp->m_iElement < 0) )
	{
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer == nullptr )
	{
		return ;
	}
	SGLImageObject *	pImage = nullptr ;
	SGLAudioPlayer *	pAudio = nullptr ;
	S3DModelBuffer *	pModel = nullptr ;
	S3DModelPose *		pPose = nullptr ;
	switch ( pProp->m_typeRaw )
	{
	case	typeImage:
		pImage = pComposer->GetAssets().GetImageAs( pProp->m_strTempValue.c_str() ) ;
		if ( pImage != nullptr )
		{
			LPtr<LNativeObj>	pImageObj( new LNativeObj( m_pImageClass ) ) ;
			pImageObj->SetNative( std::make_shared<LEntisGLS4_Image>( pImage ) ) ;
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, pImageObj.Get() ) ) ;
		}
		else
		{
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, nullptr ) ) ;
		}
		return ;

	case	typeAudioPlayer:
		pAudio = pComposer->GetAssets().GetAudioAs( pProp->m_strTempValue.c_str() ) ;
		if ( pImage != nullptr )
		{
			LPtr<LNativeObj>	pAudioObj( new LNativeObj( m_pAudioClass ) ) ;
			pAudioObj->SetNative( std::make_shared<LEntisGLS4_AudioPlayer>( pAudio ) ) ;
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, pAudioObj.Get() ) ) ;
		}
		else
		{
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, nullptr ) ) ;
		}
		return ;

	case	typeModelBuffer:
		pModel = pComposer->GetAssets().GetModelAs( pProp->m_strTempValue.c_str() ) ;
		if ( pModel != nullptr )
		{
			LPtr<LNativeObj>	pModelObj( new LNativeObj( m_pModelClass ) ) ;
			pModelObj->SetNative
				( std::make_shared<LEntisGLS4_ModelBuffer>
						( (S3DRenderBufferInterface*) pModel ) ) ;
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, pModelObj.Get() ) ) ;
		}
		else
		{
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, nullptr ) ) ;
		}
		return ;

	case	typeModelPose:
		pPose = pComposer->GetAssets().
					GetPoseLibrary().GetPoseAs( pProp->m_strTempValue.c_str() ) ;
		if ( pPose != nullptr )
		{
			LPtr<LNativeObj>	pPoseObj( new LNativeObj( m_pPoseClass ) ) ;
			pPoseObj->SetNative( std::make_shared<LEntisGLS4_ModelPose>( pPose ) ) ;
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, pPoseObj.Get() ) ) ;
		}
		else
		{
			LObject::ReleaseRef
				( m_pObj->SetElementAt( (size_t) pProp->m_iElement, nullptr ) ) ;
		}
		return ;

	case	typeSceneItem:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				LPtr<LNativeObj>	pItemObj ;
				S3DSceneComposer::ItemSerializer *
					pItem = pComp->GetSceneItemAs( pProp->m_strTempValue.c_str() ) ;
				if ( pItem != nullptr )
				{
					LClass *	pClass = m_pTask->GetClass()->VM().
											GetClassPathAs( pItem->GetLQClassName() ) ;
					ESLAssert( pClass != nullptr ) ;
					if ( pClass == nullptr )
					{
						pClass = m_pSceneItemClass ;
					}
					if ( pClass->GetVirtualVectorOf( pProp->m_type.GetClass() ) != nullptr )
					{
						pItemObj.SetPtr( new LNativeObj( pClass ) ) ;
						pItemObj->SetNative( std::make_shared<LEntisGLS4_SceneItem>( pItem ) ) ;
					}
				}
				LObject::ReleaseRef
					( m_pObj->SetElementAt( (size_t) pProp->m_iElement, pItemObj.Get() ) ) ;
			}
		}
		return ;

	default:
		break ;
	}
}

void S3DSceneCustomProperty::UpdateReferenceAllParameters( void )
{
	for ( size_t i = 0; i < m_props.GetLength(); i ++ )
	{
		UpdateReferenceParameter( i ) ;
	}
}

// コンポーザー取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer * S3DSceneCustomProperty::GetComposer( void ) const
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetComposer() ;
	}
	return	nullptr ;
}

// 例外エラーをデバッグ用に出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomProperty::ExceptionDebugTrace( LObjPtr pException )
{
	if ( pException != nullptr )
	{
		LString	lstr ;
		pException->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "exception:%s\n", str.c_str() ) ;

		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			LStringParser::LineInfo	lineInf ;
			LString			strSource ;
			LExceptionObj *	pExObj = dynamic_cast<LExceptionObj*>( pException.Ptr() ) ;
			if ( (pExObj != nullptr)
				&& pExObj->GetThrownSourceInfo( strSource, lineInf ) )
			{
				pComposer->OutputError
					( lstr.c_str(), strSource.c_str(), (int) lineInf.iLine ) ;
			}
			else
			{
				pComposer->OutputError( lstr.c_str() ) ;
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// SceneCustomItem オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Loquaty::S3DSceneCustomItem, ItemBasicSerializer, S3DSceneCustomProperty )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneCustomItem::S3DSceneCustomItem
		( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID )
	: ItemBasicSerializer( pwszClassID, &m_pscClass ),
		S3DSceneCustomProperty( vm, pObj ),
		m_strClassID( pwszClassID ), m_vm( vm )
{
	m_pwszClassID = m_strClassID ;
	m_flagsBehavior |= S3DScene::itemTimer ;

	m_pscClass.pParent = &ItemCommonSerializer::m_pscClass ;
	m_pscClass.nCount = m_params.GetLength() ;
	m_pscClass.pEntries = m_params.GetConstArray() ;
	m_ppsClass = &m_pscClass ;

	m_pDeviceClass = vm.GetClassPathAs( L"EntisGLS4.RenderDevice" ) ;
	m_pRendererClass = vm.GetClassPathAs( L"EntisGLS4.RenderContext" ) ;
	m_pCollisionClass = vm.GetClassPathAs( L"EntisGLS4.Collision" ) ;
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Composition * S3DSceneCustomItem::GetComposition( void ) const
{
	return	ItemBasicSerializer::GetComposition() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneCustomItem::GetMatrixParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetMatrixParameter( iParam ) ;
	}
	return	S3DSceneCustomProperty::GetMatrixParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

S3DDVector S3DSceneCustomItem::GetVectorParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetVectorParameter( iParam ) ;
	}
	return	S3DSceneCustomProperty::GetVectorParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

double S3DSceneCustomItem::GetScalarParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetScalarParameter( iParam ) ;
	}
	return	S3DSceneCustomProperty::GetScalarParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

int32_t S3DSceneCustomItem::GetIntegerParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetIntegerParameter( iParam ) ;
	}
	return	(int32_t) S3DSceneCustomProperty::GetIntegerParameter
						( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

bool S3DSceneCustomItem::GetBooleanParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetBooleanParameter( iParam ) ;
	}
	return	S3DSceneCustomProperty::GetBooleanParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

const wchar_t * S3DSceneCustomItem::GetCommandParameter( size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetCommandParameter( iParam ) ;
	}
	return	S3DSceneCustomProperty::GetCommandParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

size_t S3DSceneCustomItem::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t iParam ) const
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::GetBinaryParameter( pDst, nBufBytes, iParam ) ;
	}
	return	S3DSceneCustomProperty::GetBinaryParameter
				(  pDst, nBufBytes, iParam - ItemCommonSerializer::paramItemTotalCount ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetMatrixParameter( iParam, mat ) ;
		return ;
	}
	S3DSceneCustomProperty::SetMatrixParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, mat ) ;
}

void S3DSceneCustomItem::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetVectorParameter( iParam, vec ) ;
		return ;
	}
	S3DSceneCustomProperty::SetVectorParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, vec ) ;
}

void S3DSceneCustomItem::SetScalarParameter( size_t iParam, double s )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetScalarParameter( iParam, s ) ;
		return ;
	}
	S3DSceneCustomProperty::SetScalarParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, s ) ;
}

void S3DSceneCustomItem::SetIntegerParameter( size_t iParam, int32_t n )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetIntegerParameter( iParam, n ) ;
		return ;
	}
	S3DSceneCustomProperty::SetIntegerParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, n ) ;
}

void S3DSceneCustomItem::SetBooleanParameter( size_t iParam, bool b )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetBooleanParameter( iParam, b ) ;
		return ;
	}
	S3DSceneCustomProperty::SetBooleanParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, b ) ;
}

void S3DSceneCustomItem::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		ItemBasicSerializer::SetCommandParameter( iParam, pwszCmd ) ;
		return ;
	}
	S3DSceneCustomProperty::SetCommandParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, pwszCmd ) ;
}

size_t S3DSceneCustomItem::SetBinaryParameter
	( size_t iParam, const void * pSrc, size_t nBufBytes )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::SetBinaryParameter( iParam, pSrc, nBufBytes ) ;
	}
	return	S3DSceneCustomProperty::SetBinaryParameter
				( iParam - ItemCommonSerializer::paramItemTotalCount, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneCustomItem::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam < ItemCommonSerializer::paramItemTotalCount )
	{
		return	ItemBasicSerializer::EnumerateStringSet( iParam, aStrSet ) ;
	}
	return	S3DSceneCustomProperty::EnumerateStringSet
				( iParam - ItemCommonSerializer::paramItemTotalCount, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneCustomItem::GetParameterCategoryName( size_t iCategory ) const
{
	if ( iCategory == 7 )
	{
		return	L"プロパティ" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneCustomItem::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;

	S3DSceneCustomProperty::UpdateReferenceAllParameters() ;

	return	nResFlags ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	ItemBasicSerializer::OnExtendNotify
		( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;

	LPtr<LStringObj>	pStrCmd( m_pTask->Context().new_String( pwszCmd ) ) ;
	LPtr<LStringObj>	pStrParam( m_pTask->Context().new_String( pwszParam ) ) ;
	LPtr<LPointerObj>	pPtrExParam( m_pTask->Context().new_Pointer( pExParam, nExParamBytes ) ) ;

	LValue	valArg[4] ;
	valArg[0] = LValue( pStrCmd ) ;
	valArg[1] = LValue( pStrParam ) ;
	valArg[2] = LValue( pPtrExParam ) ;
	valArg[3] = LValue( LType::typeUint64, LValue::MakeLong(nExParamBytes) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onExtendNotify", valArg, 4 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnTimer( SakuraGL::S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>
				( (S3DRenderBufferInterface*) &scene ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(msecPast) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onTimer", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnUpdateFrame
	( double fpFrame, SakuraGL::S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::OnUpdateFrame( fpFrame, seek ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( LType::typeDouble, LValue::MakeDouble(fpFrame) ) ;
	valArg[1] = LValue( LType::typeInt32, LValue::MakeLong(seek) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onUpdateFrame", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnPrepareToRender
	( SakuraGL::S3DRenderDevice * pDevice, uint32_t nFlags )
{
	ItemBasicSerializer::OnPrepareToRender( pDevice, nFlags ) ;

	LPtr<LNativeObj>	pDevObj( new LNativeObj( m_pDeviceClass ) ) ;
	pDevObj->SetNative( std::make_shared<LEntisGLS4_RenderDevice>( pDevice ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pDevObj ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onPrepareToRender", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnItemRenderEvent
	( SakuraGL::S3DScene& scene, SakuraGL::S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(clsItem) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onItemRenderEvent", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnItemRenderCollision
	( const SakuraGL::S3DScene& scene, SakuraGL::S3DCollision& render )
{
	ItemBasicSerializer::OnItemRenderCollision( scene, render ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pCollision( new LNativeObj( m_pCollisionClass ) ) ;
	pCollision->SetNative
		( std::make_shared<LEntisGLS4_Collision>
				( (S3DRenderBufferInterface*) &render ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( pCollision ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onItemRenderCollision", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::OnItemRenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	ItemBasicSerializer::OnItemRenderModel( scene, render, flagsExclusion ) ;

	CallRenderModel( L"onItemRenderModel", scene, render, flagsExclusion ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::BeforeItemRenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	ItemBasicSerializer::BeforeItemRenderModel( scene, render, flagsExclusion ) ;

	CallRenderModel( L"beforeItemRenderModel", scene, render, flagsExclusion ) ;
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneCustomItem::AfterItemRenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	ItemBasicSerializer::AfterItemRenderModel( scene, render, flagsExclusion ) ;

	CallRenderModel( L"afterItemRenderModel", scene, render, flagsExclusion ) ;
}

void S3DSceneCustomItem::CallRenderModel
	( const wchar_t * pwszFuncName,
		const SakuraGL::S3DScene& scene,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pRender( new LNativeObj( m_pRendererClass ) ) ;
	pRender->SetNative
		( std::make_shared<LEntisGLS4_RenderContext>
					( (SGLPaintContextInterface*) &render ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( pRender ) ;
	valArg[2] = LValue( LType::typeUint64, LValue::MakeUint64(flagsExclusion) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszFuncName, valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneCustomItem::GetLQClassName( void ) const
{
	return	m_strObjClassPath.c_str() ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneCustomController コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Loquaty::SceneCustomController, Controller, S3DSceneCustomProperty )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SceneCustomController::SceneCustomController
		( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID )
	: S3DSceneCustomProperty( vm, pObj ),
		Controller( pwszClassID ), m_strClassID( pwszClassID ), m_vm( vm )
{
	m_pwszClassID = m_strClassID ;

	m_pDeviceClass = vm.GetClassPathAs( L"EntisGLS4.RenderDevice" ) ;
	m_pRendererClass = vm.GetClassPathAs( L"EntisGLS4.RenderContext" ) ;
	m_pCollisionClass = vm.GetClassPathAs( L"EntisGLS4.Collision" ) ;
	m_pSceneItemClass = vm.GetClassPathAs( L"EntisGLS4.SceneItem" ) ;

	PrepareParameterEntryCount( m_params.GetLength() ) ;
	for ( size_t i = 0; i < m_params.GetLength(); i ++ )
	{
		AddParameterEntry( m_params.At(i) ) ;
	}
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Composition *
	SceneCustomController::GetComposition( void ) const
{
	return	Controller::GetComposition() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DDMatrix SceneCustomController::GetMatrixParameter( ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetMatrixParameter( iParam ) ;
}

SakuraGL::S3DDVector SceneCustomController::GetVectorParameter( ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetVectorParameter( iParam ) ;
}

double SceneCustomController::GetScalarParameter( ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetScalarParameter( iParam ) ;
}

int32_t SceneCustomController::GetIntegerParameter( ::size_t iParam ) const
{
	return	(int32_t) S3DSceneCustomProperty::GetIntegerParameter( iParam ) ;
}

bool SceneCustomController::GetBooleanParameter( ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetBooleanParameter( iParam ) ;
}

const wchar_t * SceneCustomController::GetCommandParameter( ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetCommandParameter( iParam ) ;
}

::size_t SceneCustomController::GetBinaryParameter
	( void * pDst, ::size_t nBufBytes, ::size_t iParam ) const
{
	return	S3DSceneCustomProperty::GetBinaryParameter( pDst, nBufBytes, iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::SetMatrixParameter( ::size_t iParam, const SakuraGL::S3DDMatrix& mat )
{
	S3DSceneCustomProperty::SetMatrixParameter( iParam, mat ) ;
}

void SceneCustomController::SetVectorParameter( ::size_t iParam, const SakuraGL::S3DDVector& vec )
{
	S3DSceneCustomProperty::SetVectorParameter( iParam, vec ) ;
}

void SceneCustomController::SetScalarParameter( ::size_t iParam, double s )
{
	S3DSceneCustomProperty::SetScalarParameter( iParam, s ) ;
}

void SceneCustomController::SetIntegerParameter( ::size_t iParam, int32_t n )
{
	S3DSceneCustomProperty::SetIntegerParameter( iParam, n ) ;
}

void SceneCustomController::SetBooleanParameter( ::size_t iParam, bool b )
{
	S3DSceneCustomProperty::SetBooleanParameter( iParam, b ) ;
}

void SceneCustomController::SetCommandParameter( ::size_t iParam, const wchar_t * pwszCmd )
{
	S3DSceneCustomProperty::SetCommandParameter( iParam, pwszCmd ) ;
}

::size_t SceneCustomController::SetBinaryParameter
	( ::size_t iParam, const void * pSrc, ::size_t nBufBytes )
{
	return	S3DSceneCustomProperty::SetBinaryParameter( iParam, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool SceneCustomController::EnumerateStringSet
	( ::size_t iParam, SSystem::SStringArray& aStrSet )
{
	return	S3DSceneCustomProperty::EnumerateStringSet( iParam, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SceneCustomController::GetParameterCategoryName( ::size_t iCategory ) const
{
	if ( iCategory == 7 )
	{
		return	L"プロパティ" ;
	}
	return	Controller::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t SceneCustomController::UpdatePropertyReference
	( SakuraGL::S3DSceneComposer::Composition& comp,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;

	S3DSceneCustomProperty::UpdateReferenceAllParameters() ;

	return	nResFlags ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, ::size_t nExParamBytes )
{
	Controller::OnExtendNotify( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;

	LPtr<LStringObj>	pStrCmd( m_pTask->Context().new_String( pwszCmd ) ) ;
	LPtr<LStringObj>	pStrParam( m_pTask->Context().new_String( pwszParam ) ) ;
	LPtr<LPointerObj>	pPtrExParam( m_pTask->Context().new_Pointer( pExParam, nExParamBytes ) ) ;

	LValue	valArg[4] ;
	valArg[0] = LValue( pStrCmd ) ;
	valArg[1] = LValue( pStrParam ) ;
	valArg[2] = LValue( pPtrExParam ) ;
	valArg[3] = LValue( LType::typeUint64, LValue::MakeLong(nExParamBytes) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onExtendNotify", valArg, 4 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::OnTimer
	( SakuraGL::S3DScene& scene,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( pItemObj ) ;
	valArg[2] = LValue( LType::typeUint32, LValue::MakeLong(msecPast) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onTimer", valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::OnUpdateFrame
	( SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, SakuraGL::S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pItemObj ) ;
	valArg[1] = LValue( LType::typeDouble, LValue::MakeDouble(fpFrame) ) ;
	valArg[2] = LValue( LType::typeInt32, LValue::MakeLong(seek) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onUpdateFrame", valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::OnPrepareToRender
	( SakuraGL::S3DRenderDevice * pDevice, uint32_t nFlags )
{
	Controller::OnPrepareToRender( pDevice, nFlags ) ;

	LPtr<LNativeObj>	pDevObj( new LNativeObj( m_pDeviceClass ) ) ;
	pDevObj->SetNative( std::make_shared<LEntisGLS4_RenderDevice>( pDevice ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pDevObj ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onPrepareToRender", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// レンダリングイベント
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::OnRenderEvent
	( SakuraGL::S3DScene& scene,
		SakuraGL::S3DScene::ItemClass clsItem,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem )
{
	Controller::OnRenderEvent( scene, clsItem, pItem ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(clsItem) ) ;
	valArg[2] = LValue( pItemObj ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onRenderEvent", valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::RenderCollision
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DCollision& render )
{
	Controller::RenderCollision( scene, pItem, render ) ;

	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LPtr<LNativeObj>	pCollision( new LNativeObj( m_pCollisionClass ) ) ;
	pCollision->SetNative
		( std::make_shared<LEntisGLS4_Collision>
				( (S3DRenderBufferInterface*) &render ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( pItemObj ) ;
	valArg[2] = LValue( pCollision ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"renderCollision", valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::RenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DScene::ItemClass clsItem,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	Controller::RenderModel( scene, clsItem, pItem, render, flagsExclusion ) ;

	CallRenderModel( L"renderModel", scene, clsItem, pItem, render, flagsExclusion ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::BeforeRenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DScene::ItemClass clsItem,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	Controller::BeforeRenderModel( scene, clsItem, pItem, render, flagsExclusion ) ;

	CallRenderModel( L"beforeRenderModel", scene, clsItem, pItem, render, flagsExclusion ) ;
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void SceneCustomController::AfterRenderModel
	( const SakuraGL::S3DScene& scene,
		SakuraGL::S3DScene::ItemClass clsItem,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	Controller::AfterRenderModel( scene, clsItem, pItem, render, flagsExclusion ) ;

	CallRenderModel( L"afterRenderModel", scene, clsItem, pItem, render, flagsExclusion ) ;
}

void SceneCustomController::CallRenderModel
	( const wchar_t * pwszFuncName,
		const SakuraGL::S3DScene& scene,
		SakuraGL::S3DScene::ItemClass clsItem,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LPtr<LNativeObj>	pRender( new LNativeObj( m_pRendererClass ) ) ;
	pRender->SetNative
		( std::make_shared<LEntisGLS4_RenderContext>
					( (SGLPaintContextInterface*) &render ) ) ;

	LValue	valArg[5] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(clsItem) ) ;
	valArg[2] = LValue( pItemObj ) ;
	valArg[3] = LValue( pRender ) ;
	valArg[4] = LValue( LType::typeUint64, LValue::MakeUint64(flagsExclusion) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszFuncName, valArg, 5 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SceneCustomController::GetLQClassName( void ) const
{
	return	m_strObjClassPath.c_str() ;
}



//////////////////////////////////////////////////////////////////////////////
// ScenePoseController コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Loquaty::ScenePoseController, SceneCustomController, PoseInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ScenePoseController::ScenePoseController
		( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID )
	: SceneCustomController( vm, pObj, pwszClassID )
{
	m_pModelClass = vm.GetClassPathAs( L"EntisGLS4.ModelBuffer" ) ;
}

// ポーズ適用
//////////////////////////////////////////////////////////////////////////////
void ScenePoseController::OnPoseTrack
	( SakuraGL::S3DDynamicModelSerializer& item, SakuraGL::S3DModelBuffer& model )
{
	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), &item ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) &item ) ) ;

	LPtr<LNativeObj>	pModel( new LNativeObj( m_pModelClass ) ) ;
	pModel->SetNative
		( std::make_shared<LEntisGLS4_ModelBuffer>
					( (S3DRenderBufferInterface*) &model ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pItemObj ) ;
	valArg[1] = LValue( pModel ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onPoseTrack", valArg, 2 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}

// モデル変更時の処理
//////////////////////////////////////////////////////////////////////////////
void ScenePoseController::OnChangedModel
	( SakuraGL::S3DDynamicModelSerializer& item, SakuraGL::S3DModelBuffer * pModel )
{
}



//////////////////////////////////////////////////////////////////////////////
// SceneMeshController コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Loquaty::SceneMeshController, SceneCustomController, MeshInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SceneMeshController::SceneMeshController
		( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID )
	: SceneCustomController( vm, pObj, pwszClassID )
{
	m_pVertexBufferClass = vm.GetClassPathAs( L"EntisGLS4.VertexBuffer" ) ;
	m_pVBArrayClass = vm.GetArrayClassAs( m_pVertexBufferClass ) ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void SceneMeshController::AddMesh
	( SakuraGL::S3DScene& scene,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	CallMakeMesh( L"addMesh", scene, pItem, ppVBs, nVBCount ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void SceneMeshController::UpdateMesh
	( SakuraGL::S3DScene& scene,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	CallMakeMesh( L"updateMesh", scene, pItem, ppVBs, nVBCount ) ;
}

void SceneMeshController::CallMakeMesh
	( const wchar_t * pwszFuncName,
		SakuraGL::S3DScene& scene,
		SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
		SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	LPtr<LNativeObj>	pScene
		( new LNativeObj( m_vm.GetClassPathAs( scene.GetLQClassName() ) ) ) ;
	pScene->SetNative
		( std::make_shared<LEntisGLS4_Scene>( (S3DRenderBufferInterface*) &scene ) ) ;

	LPtr<LNativeObj>	pItemObj
		( new LNativeObj( GetSceneItemClass( m_pTask->GetClass()->VM(), pItem ) ) ) ;
	pItemObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
				( (S3DSceneComposer::ItemSerializer*) pItem ) ) ;

	LPtr<LArrayObj>	pArray( new LArrayObj( m_pVBArrayClass ) ) ;
	for ( size_t i = 0; i < nVBCount; i ++ )
	{
		if ( ppVBs[i] != nullptr )
		{
			LPtr<LNativeObj>	pVB( new LNativeObj( m_pVertexBufferClass ) ) ;
			pVB->SetNative
				( std::make_shared<LEntisGLS4_Scene>
					( (S3DRenderBufferInterface*) ppVBs[i] ) ) ;
			LObject::ReleaseRef( pArray->SetElementAt( i, pVB.Get() ) ) ;
		}
	}

	LValue	valArg[3] ;
	valArg[0] = LValue( pScene ) ;
	valArg[1] = LValue( pItemObj ) ;
	valArg[2] = LValue( pArray ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszFuncName, valArg, 3 ) ;
	if ( except != nullptr )
	{
		ExceptionDebugTrace( except ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// S3DBulletItemInterface::Bullet コンテナ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( Loquaty::SceneBulleteItemBulletInstance, SObject ) ;
