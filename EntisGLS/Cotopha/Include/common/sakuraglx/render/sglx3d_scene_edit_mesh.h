
#if	!defined(__SAKURAGLX3D_SCENE_EDIT_MESH_H__)
#define	__SAKURAGLX3D_SCENE_EDIT_MESH_H__	1

#include <sakura/ssys_array_set.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メッシュ編集クラス
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshEditor	: public ESLObject
	{
	public:
		// 縮退頂点フラグ
		//////////////////////////////////////////////////////////////////////
		enum	DegenerateFlag
		{
			degenerateDivNormal	= 0x0001,		// 法線を共有しない
		} ;

		// 縮退頂点
		//////////////////////////////////////////////////////////////////////
		struct	DegenerateEntry
		{
			uint32_t	nFlags ;		// complex of enum DegenerateFlag
			uint32_t	nCount ;
			uint32_t	iRef ;
		} ;
		class	DegenerateCollection
		{
		public:
			struct	SerializedHeader
			{
				uint32_t	nEntryCount ;
				uint32_t	nIndexCount ;
			} ;
			SSystem::SArray<DegenerateEntry>	m_entries ;
			SSystem::SArray<uint32_t>			m_indexes ;
		public:
			// 追加
			size_t AddDegenerate
				( uint32_t nFlags, const uint32_t * pIndex, size_t nCount ) ;
			size_t AddDegenerateNullEntry( uint32_t nFlags ) ;
			void AddDegenerateEntryAt( size_t iEntry, uint32_t iVertex ) ;
			// 削除
			void RemoveAt( size_t nIndex ) ;
			// 頂点削除
			void RemoveIndex( size_t iFirst, size_t nCount ) ;
			// すべて削除（逆引きインデックスのみ）
			void RemoveAllIndex( void ) ;
			// すべて削除
			void RemoveAll( void ) ;
			// 頂点指標オフセット
			void ShiftIndexOffset( size_t iFirst, size_t iEnd, ssize_t nOffset ) ;
			// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
			void SwapIndexes( size_t iFirst1, size_t iFirst2, size_t nCount ) ;
			// 取得（縮退頂点配列ポインタ返却）
			const uint32_t * GetDegenerateAt( DegenerateEntry& de, size_t nIndex ) const ;
			// 頂点検索
			const uint32_t * FindVertex( DegenerateEntry& de, size_t iVertex ) const ;
			// 縮退頂点フラグ取得
			uint32_t GetDegenerateFlagAt( size_t nIndex ) const ;
			// 縮退頂点フラグ変更
			bool ChangeDegenerateFlagAt( size_t nIndex, uint32_t nFlags ) ;
			// 複製
			const DegenerateCollection& operator = ( const DegenerateCollection& dc ) ;
			// シリアライズ
			void Serialize( SSystem::SArray<uint8_t>& buf ) const ;
			// デシリアライズ
			void Deserialize( const SSystem::SArray<uint8_t>& buf ) ;
		} ;


		// パッチ頂点
		//////////////////////////////////////////////////////////////////////
		class	Patch ;
		struct	PatchPoint
		{
			Patch *	pPatch ;
			size_t	iVertex ;		// 頂点指標

			PatchPoint( void ) : pPatch( NULL ), iVertex( 0 ) { }
			PatchPoint( Patch * p, size_t i ) : pPatch( p ), iVertex( i ) { }
			PatchPoint( const PatchPoint& pp ) : pPatch( pp.pPatch ), iVertex( pp.iVertex ) { }
			bool operator == ( const PatchPoint& pp ) const
			{
				return	(pPatch == pp.pPatch) && (iVertex == pp.iVertex) ;
			}
			bool operator != ( const PatchPoint& pp ) const
			{
				return	(pPatch != pp.pPatch) || (iVertex != pp.iVertex) ;
			}
			bool operator > ( const PatchPoint& pp ) const
			{
				return	((ulong_ptr_t) pPatch > (ulong_ptr_t) pp.pPatch)
					|| ((pPatch == pp.pPatch) && (iVertex > pp.iVertex)) ;
			}
			bool operator < ( const PatchPoint& pp ) const
			{
				return	((ulong_ptr_t) pPatch < (ulong_ptr_t) pp.pPatch)
					|| ((pPatch == pp.pPatch) && (iVertex < pp.iVertex)) ;
			}
		} ;
		typedef	SSystem::SArraySet<PatchPoint>	PatchPointSet ;


		// 選択頂点（※ソート済み状態で使用する）
		//////////////////////////////////////////////////////////////////////
		struct	SelectPoint	: public PatchPoint
		{
			float32_t	fpWeight ;

			SelectPoint( Patch * p, size_t i, float32_t w = 1.0f )
				: PatchPoint( p, i ), fpWeight( w ) { }
			SelectPoint( const PatchPoint& pp, float32_t w = 1.0f )
				: PatchPoint( pp ), fpWeight( w ) { }
			SelectPoint( const SelectPoint& sp )
				: PatchPoint( sp ), fpWeight( sp.fpWeight ) { }
			bool operator == ( const PatchPoint& pp ) const
			{
				return	(pPatch == pp.pPatch) && (iVertex == pp.iVertex) ;
			}
			bool operator != ( const PatchPoint& pp ) const
			{
				return	(pPatch != pp.pPatch) || (iVertex != pp.iVertex) ;
			}
			bool operator > ( const PatchPoint& pp ) const
			{
				return	((ulong_ptr_t) pPatch > (ulong_ptr_t) pp.pPatch)
					|| ((pPatch == pp.pPatch) && (iVertex > pp.iVertex)) ;
			}
			bool operator < ( const PatchPoint& pp ) const
			{
				return	((ulong_ptr_t) pPatch < (ulong_ptr_t) pp.pPatch)
					|| ((pPatch == pp.pPatch) && (iVertex < pp.iVertex)) ;
			}
		} ;
		typedef	SSystem::SArraySet<SelectPoint>	SelectPointSet ;


		// パッチ稜線
		//////////////////////////////////////////////////////////////////////
		struct	Edge
		{
			size_t	iVertex0 ;		// 頂点指標
			size_t	iVertex1 ;		// ※iVertex0 < iVertex1

			Edge( size_t v0, size_t v1 )
			{
				if ( v0 < v1 )
				{
					iVertex0 = v0 ;
					iVertex1 = v1 ;
				}
				else
				{
					iVertex0 = v1 ;
					iVertex1 = v0 ;
				}
			}
			Edge( const Edge& e )
				: iVertex0( e.iVertex0 ), iVertex1( e.iVertex1 ) { }
			bool operator == ( const Edge& edge ) const
			{
				ESLAssert( iVertex0 < iVertex1 ) ;
				return	(iVertex0 == edge.iVertex0)
						&& (iVertex1 == edge.iVertex1) ;
			}
			bool operator != ( const Edge& edge ) const
			{
				ESLAssert( iVertex0 < iVertex1 ) ;
				return	(iVertex0 != edge.iVertex0)
						|| (iVertex1 != edge.iVertex1) ;
			}
			bool operator > ( const Edge& edge ) const
			{
				ESLAssert( iVertex0 < iVertex1 ) ;
				return	(iVertex0 > edge.iVertex0)
						|| ((iVertex0 == edge.iVertex0)
								&& (iVertex1 > edge.iVertex1)) ;
			}
			bool operator < ( const Edge& edge ) const
			{
				ESLAssert( iVertex0 < iVertex1 ) ;
				return	(iVertex0 < edge.iVertex0)
						|| ((iVertex0 == edge.iVertex0)
								&& (iVertex1 < edge.iVertex1)) ;
			}
		} ;

		struct	PatchEdge	: public Edge
		{
			Patch *	pPatch ;

			PatchEdge( void ) : Edge( 0, 0 ), pPatch( NULL ) { }
			PatchEdge( Patch * p, size_t v0, size_t v1 ) : Edge( v0, v1 ), pPatch( p ) { }
			PatchEdge( Patch * p, const Edge& e ) : Edge( e ), pPatch( p ) { }
			bool operator == ( const PatchEdge& pe ) const
			{
				return	(pPatch == pe.pPatch)
						&& Edge::operator == ( pe ) ;
			}
			bool operator != ( const PatchEdge& pe ) const
			{
				return	(pPatch != pe.pPatch)
						|| Edge::operator != ( pe ) ;
			}
			bool operator > ( const PatchEdge& pe ) const
			{
				return	((ulong_ptr_t) pPatch > (ulong_ptr_t) pe.pPatch)
					|| ((pPatch == pe.pPatch) && (Edge::operator > (pe))) ;
			}
			bool operator < ( const PatchEdge& pe ) const
			{
				return	((ulong_ptr_t) pPatch < (ulong_ptr_t) pe.pPatch)
					|| ((pPatch == pe.pPatch) && (Edge::operator < (pe))) ;
			}
		} ;

		struct	EdgeDiv	: public Edge
		{
			float32_t	tDiv ;

			EdgeDiv( void ) : Edge( 0, 0 ), tDiv( 0.0f ) { }
			EdgeDiv( size_t v0, size_t v1, float32_t t )
				: Edge( v0, v1 ), tDiv( (v0 < v1) ? t : 1.0f - t ) { }
			EdgeDiv( const EdgeDiv& edv )
				: Edge( edv ), tDiv( edv.tDiv ) { }
		} ;

		class	EdgeSet	: public SSystem::SArraySet<Edge>
		{
		public:
			EdgeSet( void ) { }
			EdgeSet( const EdgeSet& es ) : SSystem::SArraySet<Edge>( es ) { }
			// 頂点削除
			void RemovePointIndex( size_t iFirst, size_t nCount ) ;
			// 頂点指標オフセット
			void ShiftIndexOffset( size_t iFirst, size_t iEnd, ssize_t nOffset ) ;
			// 複製
			const EdgeSet& operator = ( const EdgeSet& es ) ;
			// シリアライズ
			void Serialize( SSystem::SArray<uint8_t>& buf ) const ;
			// デシリアライズ
			void Deserialize( const SSystem::SArray<uint8_t>& buf ) ;
		} ;

		typedef	SSystem::SArraySet<PatchEdge>	PatchEdgeSet ;


		// パッチ線（水平又は垂直に連なる稜線）
		//////////////////////////////////////////////////////////////////////
		enum	LineDirection
		{
			lineHorizontal,
			lineVertical,
		} ;
		struct	Line
		{
			LineDirection	lineDir ;
			size_t			iLine ;

			Line( LineDirection dir = lineHorizontal, size_t l = 0 ) : lineDir( dir ), iLine( l ) { }
			Line( const Line& line ) : lineDir( line.lineDir ), iLine( line.iLine ) { }
			bool operator == ( const Line& line ) const
			{
				return	(lineDir == line.lineDir)
						&& (iLine == line.iLine) ;
			}
			bool operator != ( const Line& line ) const
			{
				return	(lineDir != line.lineDir)
						|| (iLine != line.iLine) ;
			}
			bool operator > ( const Line& line ) const
			{
				return	(lineDir > line.lineDir)
					|| ((lineDir == line.lineDir) && (iLine > line.iLine)) ;
			}
			bool operator < ( const Line& line ) const
			{
				return	(lineDir < line.lineDir)
					|| ((lineDir == line.lineDir) && (iLine < line.iLine)) ;
			}
		} ;

		struct	PatchLine	: public Line
		{
			Patch *			pPatch ;

			PatchLine( void ) : Line( lineHorizontal, 0 ), pPatch( NULL ) { }
			PatchLine( Patch * p, LineDirection dir, size_t l ) : Line( dir, l ), pPatch( p ) { }
			PatchLine( const PatchLine& pl ) : Line( pl ), pPatch( pl.pPatch ) { }
			bool IsLoop( void ) const
			{
				return	pPatch->IsLineLoop( lineDir ) ;
			}
			bool operator == ( const PatchLine& pl ) const
			{
				return	(pPatch == pl.pPatch)
						&& (Line::operator == (pl)) ;
			}
			bool operator != ( const PatchLine& pl ) const
			{
				return	(pPatch == pl.pPatch)
						&& (Line::operator != (pl)) ;
			}
			bool operator < ( const PatchLine& pl ) const
			{
				return	((ulong_ptr_t) pPatch < (ulong_ptr_t) pl.pPatch)
						|| ((pPatch == pl.pPatch) && (Line::operator < (pl))) ;
			}
			bool operator > ( const PatchLine& pl ) const
			{
				return	((ulong_ptr_t) pPatch > (ulong_ptr_t) pl.pPatch)
						|| ((pPatch == pl.pPatch) && (Line::operator > (pl))) ;
			}
		} ;
		typedef	SSystem::SArraySet<PatchLine>	PatchLineSet ;


		// パッチ面
		//////////////////////////////////////////////////////////////////////
		struct	PatchFace
		{
			Patch *	pPatch ;
			size_t	iFace ;		// 面指標

			PatchFace( Patch * p, size_t f ) : pPatch( p ), iFace( f ) { }
			PatchFace( const PatchFace& pf ) : pPatch( pf.pPatch ), iFace( pf.iFace ) { }
			bool operator == ( const PatchFace& pf ) const
			{
				return	(pPatch == pf.pPatch) && (iFace == pf.iFace) ;
			}
			bool operator != ( const PatchFace& pf ) const
			{
				return	(pPatch != pf.pPatch) || (iFace != pf.iFace) ;
			}
			bool operator > ( const PatchFace& pf ) const
			{
				return	((ulong_ptr_t) pPatch > (ulong_ptr_t) pf.pPatch)
					|| ((pPatch == pf.pPatch) && (iFace > pf.iFace)) ;
			}
			bool operator < ( const PatchFace& pf ) const
			{
				return	((ulong_ptr_t) pPatch < (ulong_ptr_t) pf.pPatch)
					|| ((pPatch == pf.pPatch) && (iFace < pf.iFace)) ;
			}
		} ;
		typedef	SSystem::SArraySet<PatchFace>	PatchFaceSet ;


		// パッチ間の接続情報
		//////////////////////////////////////////////////////////////////////
		class	SewPatchPoints	: public SSystem::SArray<PatchPoint>
		{
		public:
			uint32_t	m_nFlags ;		// complex of enum DegenerateFlag
		public:
			// 構築関数
			SewPatchPoints( void ) ;
			SewPatchPoints( const SewPatchPoints& spp ) ;
			// 検索
			ssize_t FindPoint( const PatchPoint& pp ) const ;
			// 削除
			ssize_t RemoveAs( const PatchPoint& pp ) ;
			// 結合
			void Merge( const SewPatchPoints& spp ) ;
			// 頂点削除
			void RemovePointIndex( const Patch * pPatch, size_t iFirst, size_t nCount ) ;
			// 頂点指標オフセット
			void ShiftIndexOffset
				( const Patch * pPatch, size_t iFirst, size_t iEnd, ssize_t nOffset ) ;
			// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
			void SwapPointIndexes
				( const Patch * pPatch, size_t iFirst1, size_t iFirst2, size_t nCount ) ;
			// 複製
			const SewPatchPoints& operator = ( const SewPatchPoints& spp ) ;
			// Patch 参照の置き換え
			void RepointerPatch( const Patch * pPatchOld, Patch * pPatchNew ) ;
		} ;
		class	SeamPatchCollection	: public SSystem::SObjectArray<SewPatchPoints>
		{
		public:
			// 構築関数
			SeamPatchCollection( void ) { }
			SeamPatchCollection( const SeamPatchCollection& spc )
				: SObjectArray<SewPatchPoints>( spc ) { }
			// 検索
			ssize_t FindPoint( const PatchPoint& pp ) const ;
			// 追加
			size_t AddSewPatchPoints
				( uint32_t nFlags, const PatchPoint * ppp, size_t nCount ) ;
			// 削除
			ssize_t RemoveAs( const PatchPoint& pp ) ;
			// 頂点削除
			void RemovePointIndex( const Patch * pPatch, size_t iFirst, size_t nCount ) ;
			// 頂点指標オフセット
			void ShiftIndexOffset
				( const Patch * pPatch, size_t iFirst, size_t iEnd, ssize_t nOffset ) ;
			// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
			void SwapPointIndexes
				( const Patch * pPatch, size_t iFirst1, size_t iFirst2, size_t nCount ) ;
			// 複製
			const SeamPatchCollection& operator = ( const SeamPatchCollection& spc ) ;
			// Patch 参照の置き換え
			void RepointerPatch( const Patch * pPatchOld, Patch * pPatchNew ) ;
			// 結合
			void Merge( const SeamPatchCollection& spc ) ;
		public:
			typedef	DegenerateCollection::SerializedHeader	SerializedHeader ;
			typedef	DegenerateEntry							SerializedEntry ;
			struct	PatchPointIndex
			{
				uint32_t	iPatch ;
				uint32_t	iVertex ;
			} ;
			// シリアライズ
			void Serialize
				( SSystem::SArray<uint8_t>& buf, const S3DMeshEditor& me ) const ;
			// デシリアライズ
			void Deserialize
				( const SSystem::SArray<uint8_t>& buf, const S3DMeshEditor& me ) ;
		} ;


		// メッシュバッファ
		//////////////////////////////////////////////////////////////////////
		class	MeshBuffer
		{
		public:
			size_t						m_countVertex ;
			size_t						m_countIndex ;
			size_t						m_nWeightLayers ;	// = m_bufExAttrIndex.GetLength()
			SSystem::SArray<size_t>		m_bufExAttrIndex ;	// ボーンでないウェイトマップレイヤー番号
			SSystem::SArray<S3DVector4>	m_bufVertex ;
			SSystem::SArray<S3DVector4>	m_bufNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<float32_t>	m_bufWeight ;	// ※レイヤー数分を1つの要素とする2次元配列 [y][x][layer]
														//（Path::m_bufWeight とは配列要素の順序が異なる）
			SSystem::SArray<size_t>		m_bufSrcVertex ;// 各頂点のソース頂点指標
			SSystem::SArray<uint32_t>	m_bufIndex ;	// 各四角形毎の頂点指標
			EdgeSet						m_edegs ;

		public:
			// 構築
			MeshBuffer( void ) ;
			MeshBuffer( const MeshBuffer& mbuf ) ;
			// クリア
			void ClearBuffer( void ) ;
			// 複製
			void CopyBufferFrom( const MeshBuffer& mbuf ) ;
			const MeshBuffer& operator = ( const MeshBuffer& mbuf ) ;
			// 外接直方体
			bool GetCircumscribedBox( S3DVector& vMin, S3DVector& vMax ) const ;
		} ;

		// メッシュ生成の基本設定
		enum	MeshParamFlag
		{
			flagMeshPartialEdge			= 0x0000,	// 稜線ごとに分割判定
			flagMeshAllSmooth			= 0x0001,	// 全ての頂点の法線を分割しない
			flagMeshAllDivPoints		= 0x0002,	// 全ての稜線の法線を分割する
			flagMeshAllFlat				= 0x0003,	// 全ての稜線の法線をフラットにする
			flagMeshEdgeMethodMask		= 0x0003,
			flagMeshEdgeByAngle			= 0x0004,	// 角度によって稜線を分割する
			flagMeshNoRemoveDegenerated	= 0x0008,	// 縮退ポリゴンを削除しない
			flagMeshPrimitiveType		= 0x0010,	// nPrimitiveType 指定
			flagMeshEdgeSingleLine		= 0x0020,	// 稜線描画（2重描画しない）must be nPrimitiveType == primitiveLine
		} ;
		enum	DivInterpolationMethod
		{
			divMethodLerp,
			divMethodBezier,
		} ;
		struct	MeshParam
		{
			uint32_t	nFlags ;
			float32_t	fpEdgeAngle ;		// 稜線法線を分割する限界角 [deg]
			uint32_t	nDivHorz ;			// メッシュ分割数
			uint32_t	nDivVert ;
			uint32_t	nDivMethod ;		// enum DivInterpolationMethod
			uint32_t	nPrimitiveType ;	// enum S3DPrimitiveType
			uint32_t	nReserved[10] ;

			MeshParam( void )
				: nFlags( flagMeshAllSmooth | flagMeshEdgeByAngle ),
					fpEdgeAngle( 30.0f ),
					nDivHorz( 1 ), nDivVert( 1 ), nDivMethod( divMethodBezier )
			{
				for ( int i = 0; i < sizeof(nReserved)/sizeof(nReserved[0]); i ++ )
				{
					nReserved[i] = 0 ;
				}
			}
		} ;


		// パッチメッシュ情報
		//////////////////////////////////////////////////////////////////////
		class	Patch	: public ESLObject
		{
		public:
			enum	PatchFlag
			{
				flagVertLoop		= 0x00000001,		// 垂直ループ
				flagHorzLoop		= 0x00000002,		// 水平ループ
				flagInvisible		= 0x00000004,		// 非表示
				flagCollision		= 0x00000008,		// 当たり判定
				flagDisableModifier	= 0x00000010,		// コントローラーの影響から除外
				flagFreezeNormal	= 0x00010000,		// 法線は更新しない
			} ;
			enum	FaceFlag
			{
				faceNull	= 0,				// 空面
				faceFront	= 1,				// 表面
				faceBack	= -1,				// 裏面
			} ;
			SeamPatchCollection *		m_pspc ;		// 参照 SeamPatchCollection
			S3DMeshBufferPropertySerializer::MeshBuffer *
										m_pSerializer ;	// 関連付けられたシリアライザ
			SSystem::SString			m_strName ;		// パッチ名

			size_t						m_wPatch ;		// サイズ [頂点数(==面数)]
			size_t						m_hPatch ;
			size_t						m_nWeightLayers ;
			size_t						m_iMaterial ;		// マテリアル
			bool						m_flagUpdateVertex ;	// 頂点編集フラグ（法線の更新の必要性）
			bool						m_flagUpdateFace ;		// 面編集フラグ（使用頂点参照配列更新の必要性）
			bool						m_flagUpdateSerialize ;	// シリアライズフラグ
			uint32_t					m_nFlags ;			// フラグ enum PatchFlag 複合
			SSystem::SArray<S3DVector>	m_bufVertex ;		// 頂点
			SSystem::SArray<S3DVector>	m_bufNormal ;		// 法線（自動生成）
			SSystem::SArray<S3DVector>	m_bufFaceNormal ;	// 面法線（自動生成）
			SSystem::SArray<S2DVector>	m_bufUVMap ;		// UV
			SSystem::SArray<S3DColor>	m_bufColor ;		// 色
			SSystem::SArray<float32_t>	m_bufWeight ;		// ウェイトマップ（3次元配列）[layer][y][x]
			EdgeSet						m_edges ;			// 法線分割稜線（常にソート）
			SSystem::SArray<int8_t>		m_bufFace ;			// {-1|0|1} [面数] (面情報はループを常に前提とする配列を保持)
			SSystem::SArray<uint8_t>	m_bufShifter ;		// {0|1} [面数] 三角化の際の頂点シフト数
			SSystem::SArray<uint32_t>	m_bufUsedVertex ;	// 使用頂点のみの頂点指標配列
			SSystem::SArray<uint32_t>	m_bufIUsedVertex ;	// m_bufUsedVertex の逆変換 [頂点指標]
			SSystem::SArray<int8_t>		m_bufUsedNormalFace ;// 使用頂点の法線の反転フラグ
			SSystem::SArray<uint32_t>	m_bufDegenerate ;	// 縮退指標 (m_degenerates 参照用指標) [頂点指標]
			DegenerateCollection		m_degenerates ;
			SSystem::SArray<uint32_t>	m_bufTriangles ;	// パッチ外変則ポリゴン

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Patch, ESLObject )
			// 構築
			Patch( SeamPatchCollection * pspc = NULL ) ;
			// 名前
			const SSystem::SString& GetName( void ) const ;
			void SetName( const wchar_t * pwszName ) ;
			// シリアライザ
			void AttachSerializer
				( S3DMeshBufferPropertySerializer::MeshBuffer * pSerializer ) ;
			S3DMeshBufferPropertySerializer::MeshBuffer * GetSerializer( void ) const ;
			// 複製
			void CopyFrom( const Patch& patch ) ;
			void CopyRect
				( int xDst, int yDst,
					const Patch& patch, const SGLImageRect& rectSrc ) ;
			// サイズ（頂点数＝面数（常にループ））
			size_t GetWidth( void ) const ;
			size_t GetHeight( void ) const ;
			size_t GetAreaSize( void ) const ;
			// ループの有無を考慮した面数
			size_t GetFaceWidth( void ) const ;
			size_t GetFaceHeight( void ) const ;
			// フラグ (PatchFlag)
			uint32_t GetFlags( void ) const ;
			void SetFlags( uint32_t nFlags ) ;
			void ModifyFlags( uint32_t nAdd, uint32_t nRemove ) ;
			// マテリアル
			size_t GetMaterialIndex( void ) const ;
			void SetMaterialIndex( size_t iMaterial ) ;
			// 頂点更新フラグ設定（法線更新判定）
			void SetUpdateVertexFlag( void ) ;
			// 頂点更新フラグ取得
			bool GetUpdateVertexFlag( void ) const ;
			// 面更新フラグ設定（使用頂点参照配列更新判定）
			void SetUpdateFaceFlag( void ) ;
			// 面更新フラグ取得
			bool GetUpdateFaceFlag( void ) const ;
			// シリアライズ用更新フラグ設定
			void SetUpdateSerializeFlag( void ) ;
			// シリアライズ用更新フラグクリア
			void ResetUpdateSerializeFlag( void ) ;
			// シリアライズ用更新フラグ取得
			bool GetUpdateSerializeFlag( void ) const ;

		public:
			// 初期サイズ設定
			void CreatePatch( size_t nWidth, size_t nHeight ) ;
			// 水平ライン挿入
			void InsertLine( size_t iLine, float_t w = 1.0f ) ;
			// 垂直ライン挿入
			void InsertColumn( size_t iCol, float_t w = 1.0f ) ;
			// 水平ライン削除
			void RemoveLine( size_t iLine ) ;
			// 垂直ライン削除
			void RemoveColumn( size_t iCol ) ;
			// ライン入れ替え
			void SwapLine( size_t iLine1, size_t iLine2 ) ;

		public:
			// 頂点削除
			void RemovePoints( size_t iFirst, size_t nCount ) ;
			// 頂点挿入
			void InsertPoints( size_t iFirst, size_t nCount ) ;
			// 頂点参照インデックス操作
			void ShiftIndexOffset( size_t iFirst, size_t iEnd, ssize_t nOffset ) ;

		public:
			// 面削除
			void RemoveFaces( size_t iFirst, size_t nCount ) ;
			// 面挿入
			void InsertFaces( size_t iFirst, size_t nCount ) ;

		public:
			// 頂点指標変換
			uint32_t IndexFromPoint( size_t x, size_t y ) const ;
			void PointFromIndex( SGLPoint& pt, uint32_t i ) const ;
			bool IsValidPoint( const SGLPoint& pt ) const ;
			bool IsValidPoint( size_t x, size_t y ) const ;
			// パッチ面の頂点指標（4要素）取得（up-left, up-right, down-left, down-right）
			void QuadIndexesFromFaceIndex( size_t * pIndexes /*[4]*/, size_t iFace ) const ;
			void QuadIndexesFromFacePoint( size_t * pIndexes /*[4]*/, size_t x, size_t y ) const ;
			// 指定頂点を含む稜線取得
			bool EnumeratePointToEdge
				( SSystem::SArraySet<size_t>& asEdgePoint, size_t iOrgPoint ) const ;
			// 稜線判定（パッチ内のみ）
			bool IsPatchEdge( size_t iVertex0, size_t iVertex1 ) const ;
			// 稜線を挟む面を取得（パッチ内のみ）
			// （iVertex0->iVertex1 に向かって pFaces[0]:右側, pFaces[1]:左側）
			bool GetSideFaceByEdge
				( size_t * pFaces /*[2]*/, size_t iVertex0, size_t iVertex1 ) const ;
			// 有効面が存在しないか？
			bool IsFaceEmpty( void ) const ;
			// ラインはループか？
			bool IsLineLoop( LineDirection lineDir ) const ;

		public:
			// パッチ外頂点も含めた全頂点数・面数
			size_t GetTotalVertexCount( void ) const ;
			size_t GetTotalFaceCount( void ) const ;
			// 座標
			const S3DVector& GetPoint( size_t x, size_t y ) const ;
			const S3DVector& GetPointAt( size_t i ) const ;
			const S3DVector * GetConstPointArray( size_t i, size_t n ) const ;
			void SetPoint( size_t x, size_t y, const S3DVector& v ) ;
			void SetPointAt( size_t i, const S3DVector& v ) ;
			// 法線
			const S3DVector& GetNormal( size_t x, size_t y ) const ;
			const S3DVector& GetNormalAt( size_t i ) const ;
			bool ShouldNormalInverseAt( size_t i ) const ;
			const S3DVector * GetConstNormalArray( size_t i, size_t n ) const ;
			void SetNormal( size_t x, size_t y, const S3DVector& v ) ;
			void SetNormalAt( size_t i, const S3DVector& v ) ;
			// 面法線
			const S3DVector& GetFaceNormal( size_t x, size_t y ) const ;
			const S3DVector& GetFaceNormalAt( size_t i ) const ;
			void SetFaceNormal( size_t x, size_t y, const S3DVector& v ) ;
			void SetFaceNormalAt( size_t i, const S3DVector& v ) ;
			// UV
			const S2DVector& GetUV( size_t x, size_t y ) const ;
			const S2DVector& GetUVAt( size_t i ) const ;
			const S2DVector * GetConstUVArray( size_t i, size_t n ) const ;
			void SetUV( size_t x, size_t y, const S2DVector& uv ) ;
			void SetUVAt( size_t i, const S2DVector& uv ) ;
			// 色
			const S3DColor& GetColor( size_t x, size_t y ) const ;
			const S3DColor& GetColorAt( size_t i ) const ;
			const S3DColor * GetConstColorArray( size_t i, size_t n ) const ;
			void SetColor( size_t x, size_t y, const S3DColor& color ) ;
			void SetColorAt( size_t i, const S3DColor& color ) ;
			// 面
			int8_t GetFace( size_t x, size_t y ) const ;
			int8_t GetFaceAt( size_t i ) const ;
			bool IsValidFaceArea( size_t x, size_t y ) const ;
			bool IsValidFaceAreaAt( size_t i ) const ;
			const int8_t * GetConstFaceArray( size_t i, size_t n ) const ;
			void SetFace( size_t x, size_t y, int8_t nFace ) ;
			void SetFaceAt( size_t i, int8_t nFace ) ;
			// 面三角化シフタ
			uint8_t GetFaceShifterAt( size_t i ) const ;
			const uint8_t * GetConstFaceShifterArray( size_t i, size_t n ) const ;
			void SetFaceShifterAt( size_t i, uint8_t nShifter ) ;
			// ウェイト
			float32_t GetWeight( size_t x, size_t y, size_t z ) const ;
			float32_t GetWeightAt( size_t i, size_t z ) const ;
			const float32_t * GetConstWeightArrayAt( size_t z ) const ;
			void SetWeight( size_t x, size_t y, size_t z, float32_t w ) ;
			void SetWeightAt( size_t i, size_t z, float32_t w ) ;
			// 全要素
			struct	Elements
			{
				S3DVector	pos ;
				S3DVector	normal ;
				S2DVector	uv ;
				S3DColor	color ;
				size_t		nWeights ;
				float32_t *	pWeights ;
			} ;
			void GetElements( Elements& el, size_t x, size_t y ) const ;
			void GetElementsAt( Elements& el, size_t i ) const ;
			void SetElements( size_t x, size_t y, const Elements& el ) ;
			void SetElementsAt( size_t i, const Elements& el ) ;
			static void LerpElements( Elements& el0, const Elements& el1, float32_t t ) ;
			// 縮退頂点（逆引き番号）
			uint32_t GetDegenerateNumber( size_t x, size_t y ) const ;
			uint32_t GetDegenerateNumberAt( size_t iVertex ) const ;
			const uint32_t * GetDegenerateNumberArrayAt( size_t iVertex, size_t nCount ) const ;
			void SetDegenerateNumber( size_t x, size_t y, uint32_t nDeg ) ;
			void SetDegenerateNumberAt( size_t iVertex, uint32_t nDeg ) ;
			void RebuildDegenerateByNumber( void ) ;
			// 縮退エントリ配列
			DegenerateCollection& GetDegenerateCollection( void ) ;
			const DegenerateCollection& GetDegenerateCollection( void ) const ;
			// 縮退頂点とエントリの整合性検証
			bool VerifyDegenerate( void ) const ;
			// 縮退頂点
			const uint32_t * GetDegenerate
					( DegenerateEntry& de, size_t x, size_t y ) const ;
			const uint32_t * GetDegenerateAt
					( DegenerateEntry& de, size_t iVertex ) const ;
			bool GetDegeneratedPoints( PatchPointSet& pps, size_t iVertex ) const ;
			void ReleaseDegenerate( size_t x, size_t y ) ;
			void ReleaseDegenerateAt( size_t iVertex ) ;
			void SetDegenerate
				( uint32_t nFlags, const SGLPoint * pDegenerate, size_t nCount ) ;
			void SetDegenerate
				( uint32_t nFlags, const uint32_t * pIndexes, size_t nCount ) ;
			bool ChangeDegenerateFlagAt( size_t iVertex, uint32_t nFlags ) ;
			// ライン縮退判定
			bool IsDegeneratedLines
				( LineDirection lineDir, size_t iLine0, size_t iLine1 ) const ;
			// 縮退頂点を収束させる
			void ShrinkDegeneratedPoints( void ) ;
			// 縮退頂点のない空のエントリを削除する
			void NormalizeDegenerateEntry( void ) ;
			// 全ての縮退頂点をクリアする
			void ClearAllDegeneration( void ) ;

		public:
			// ウェイトマップ数
			size_t GetWeightLayerCount( void ) const ;
			// ウェイトマップレイヤー追加
			void InsertWeightLayer( size_t iLayer, size_t nCount ) ;
			// ウェイトマップレイヤー削除
			void RemoveWeightLayer( size_t iLayer, size_t nCount ) ;
			// ウェイトマップレイヤー入れ替え
			void SwapWeightLayer( size_t iLayer0, size_t iLayer1 ) ;

		public:
			// パッチ外頂点数
			size_t GetExVertexCount( void ) const ;
			// パッチ外面数
			size_t GetExFaceCount( void ) const ;
			// 面追加
			void InsertExFace( size_t iExFace, int8_t nFace, const uint32_t * pVertics ) ;
			// 面削除
			void RemoveExFace( size_t iExFace ) ;
			// パッチ外頂点追加
			void InsertExVertics( size_t iExPoint, const S3DVector * pVertics, size_t nCount ) ;
			// パッチ外頂点削除
			void RemoveExVertics( size_t iExPoint, size_t nCount ) ;
			// パッチ外面検索
			ssize_t FindExFaceVertexOf( size_t iVertex, size_t iFirstExFace = 0 ) const ;
			// パッチ外面数の頂点指標（3要素）取得
			void TriangleIndexesFromExFaceIndex( size_t * pIndexes, size_t iExFace ) const ;
			// パッチ外三角頂点配列取得
			const uint32_t * GetExFaceTriangleIndexes( size_t iExFace, size_t nCount ) const ;
			// パッチ外三角頂点変更
			void ModifyExFaceTriangleIndexes
				( size_t iExFace, size_t nCount, const uint32_t * pIndexes ) ;

		public:
			// 三角ポリゴンメッシュを構築
			void MakeTriangleMeshFrom( const S3DRenderBuffer::MeshBuffer& mbuf ) ;

		public:
			// BuildPatchMesh 前に実行すべき更新処理を実行する
			void UpdatePatchMesh( void ) ;
			// 法線計算
			void UpdateVertexNormal( void ) ;
			void UpdateFaceNormal( void ) ;
			// 使用頂点のみの頂点配列へ変換する参照配列を構築
			void MakeUsedVertexIndexArray( void ) ;

		public:
			// パッチ(QUAD)メッシュの構築
			void BuildPatchMesh( MeshBuffer& mbuf, const MeshParam& mparam ) const ;
		protected:
			// 部分的に稜線を分割するメッシュ構築
			void BuildPatchMeshPartialEdge
					( MeshBuffer& mbuf, const MeshParam& mparam ) const ;
			// 全てスムージングするメッシュ構築
			void BuildPatchMeshAllSmooth
					( MeshBuffer& mbuf, const MeshParam& mparam ) const ;
			// 全頂点を分割するメッシュ構築
			void BuildPatchMeshAllDivPoints
					( MeshBuffer& mbuf, const MeshParam& mparam ) const ;
			// 使用頂点のみを MeshBuffer へ複製する
			void BuildPatchUsedVertex( MeshBuffer& mbuf ) const ;
		public:
			// 全ポリゴンの単純な構築（当たり判定用）
			void BuildAllPolygonsSimply( MeshBuffer& mbuf ) const ;

		public:
			struct	SerializedPatchHeader
			{
				uint32_t	nFlags ;
				uint32_t	wPatch ;
				uint32_t	hPatch ;
				uint32_t	nWeightLayers ;
				uint32_t	nExFaces ;
				uint32_t	iMaterial ;
				uint32_t	nReserved[10] ;
			} ;
			// シリアライズ (UpdatePatchMesh を実行してから)
			void SerializeToMesh
				( S3DMeshBufferPropertySerializer::MeshBuffer& mesh ) const ;
			// デシリアライズ
			void DeserializeFromMesh
				( const S3DMeshBufferPropertySerializer::MeshBuffer& mesh ) ;
			// 通常のメッシュから変換
			void ConvertFromMeshBuffer
				( const S3DMeshBufferPropertySerializer::MeshBuffer& mesh ) ;
			void ConvertFromVertexBuffer
				( const S3DVertexBufferInterface& vbo ) ;
		} ;

		// ウェイトマップ情報
		enum	WeightMapFlag
		{
			weightBone	= 0x0001,
		} ;
		struct	WeightMapInfo
		{
			uint32_t	nFlags ;		// complex of enum WeightMapFlag
			uint32_t	nReserved[3] ;	// must be zero
			S4DMatrix	mat4Bone ;		// ボーンの初期状態

			WeightMapInfo( void )
				: nFlags( 0 ), mat4Bone( 1, 1, 1, 1 )
			{
				nReserved[0] = 0 ;
				nReserved[1] = 0 ;
				nReserved[2] = 0 ;
			}
		} ;

	protected:
		SSystem::SObjectArray<Patch>	m_patchs ;
		SeamPatchCollection				m_seams ;
		SSystem::SObjectArray
				<SSystem::SString>		m_aWeightLayerIDs ;
		SSystem::SObjectArray
				<WeightMapInfo>			m_aWeightLayerInfos ;
		bool							m_flagUpdateVertex ;	// 頂点編集フラグ（法線の更新の必要性）
		bool							m_flagUpdateSerialize ;	// シリアライズ更新フラグ

		struct	SerializedMeshInfo
		{
			uint32_t	nHeaderBytes ;		// sizeof(SerializedMeshInfo)
			uint32_t	nHeaderFlags ;		// zero
			uint32_t	nWeightLayerCount ;
			uint32_t	addrWeightLayerID ;	// ID名先頭アドレス
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMeshEditor, ESLObject )
		// 構築関数
		S3DMeshEditor( void ) ;
		// 消滅関数
		~S3DMeshEditor( void ) ;

	public:
		// すべて削除
		void ClearAllMeshs( void ) ;
		// 複製
		void DuplicateMesh( const S3DMeshEditor& mesh ) ;

	public:
		// パッチ接続点
		SeamPatchCollection& SeamPatch( void )
		{
			return	m_seams ;
		}
		const SeamPatchCollection& GetSeamPatch( void ) const
		{
			return	m_seams ;
		}
		// 選択点の縮退フラグ取得
		uint32_t GetDegeneratedPointFlag( const PatchPointSet& pps ) const ;
		// 選択点の縮退フラグ変更
		void SetDegeneratedPointFlag( const PatchPointSet& pps, uint32_t nFlags ) ;
		// 縮退頂点取得
		bool GetDegeneratedPoints( PatchPointSet& pps, const PatchPoint& pp ) const ;
		// 縮退頂点とエントリの整合性検証
		bool VerifyDegenerate( void ) const ;
		// 頂点縮退処理（頂点座標操作は無し）
		void ShrinkPoints( const PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点座標を一点に収束
		void BundlePoints( const PatchPointSet& pps ) ;
		// 頂点縮退解除
		void UntiShrinkPoints( const PatchPoint& pp ) ;
		// 縮退頂点の座標を収束させる
		void ShrinkDegeneratedPoints( void ) ;

	public:
		// 頂点更新フラグ設定
		void SetUpdateVertexFlag( void ) ;
		// 頂点更新フラグ取得
		bool GetUpdateVertexFlag( void ) const ;
		// 頂点更新フラグを検証
		void VerifyUpdateFlagByPatchUpdate( void ) ;
		// 更新パッチを再構築する
		virtual void UpdateAllPatchs( const MeshParam& param ) ;
	protected:
		// パッチ間の縮退頂点の法線を合成する
		void SeamPatchNormals( const MeshParam& param ) ;

	public:
		// バーテックスバッファ構築
		enum	RenderVertexBufferFlag
		{
			renderAll			= 0x0001,
			renderCollision		= 0x0002,
			renderModifiable	= 0x0004,
		} ;
		void RenderVertexBuffers
			( S3DVertexBufferInterface *const* ppVBOs,
				S3DMaterial *const* ppMaterials,
				size_t * pExistingCount, size_t nDstBufferCount,
				const MeshParam& param, uint32_t nFlags ) ;
		static void RenderQuadMeshBuffer
			( S3DVertexBufferInterface& vbo, S3DMaterial * pMaterial,
				const S3DMatrix& matMesh, const S3DVector& vMesh,
				MeshBuffer& mesh, const MeshParam& param ) ;
		static void RenderExPatchTriangles
			( S3DVertexBufferInterface& vbo, S3DMaterial * pMaterial,
				const S3DMatrix& matMesh, const S3DVector& vMesh,
				const Patch& patch, MeshBuffer& bufTemp, const MeshParam& param ) ;

	public:
		// シリアライズ更新フラグ設定
		void SetUpdateSerializeFlag( void ) ;
		// シリアライズ更新フラグ取得
		bool GetUpdateSerializeFlag( void ) const ;

	public:
		// シリアライズ
		void SerializeMesh( S3DMeshBufferPropertySerializer& mesh ) ;
	protected:
		void SerializeMeshInfo
				( S3DMeshBufferPropertySerializer::MeshBuffer& meshBuf ) ;
	public:
		// デシリアライズ
		void DeserializeMesh( const S3DMeshBufferPropertySerializer& mesh ) ;
	protected:
		void DeserializeMeshInfo
				( S3DMeshBufferPropertySerializer::MeshBuffer& meshBuf ) ;
	public:
		// 通常のメッシュバッファから変換
		void ConvertFromMeshBuffer( const S3DMeshBufferPropertySerializer& mesh ) ;

	public:
		// パッチ総数取得
		size_t GetPatchCount( void ) const ;
		// パッチ取得
		Patch * GetPatchAt( size_t i ) const ;
		// パッチ生成
		Patch * NewPatch( void ) ;
		// パッチ追加
		size_t AddPatch( Patch * pPatch ) ;
		size_t InsertPatchAt( size_t i, Patch * pPatch ) ;
		// パッチ削除
		void RemovePatchAt( size_t i ) ;
		void RemoveAllPatchs( void ) ;
		// パッチ検索
		ssize_t FindPatch( Patch * pPatch ) const ;
		// パッチ順序入れ替え
		void SwapPatchOrder( size_t iPatch0, size_t iPatch1 ) ;

	public:
		// ウェイトマップ数
		size_t GetWeightLayerCount( void ) const ;
		// ウェイトマップレイヤーID取得
		const wchar_t * GetWeightLayerIDAt( size_t iLayer ) const ;
		// ウェイトマップ情報取得
		const WeightMapInfo * GetWeightLayerInfoAt( size_t iLayer ) const ;
		// ウェイトマップレイヤーID設定
		void SetWeightLayerIDAt( size_t iLayer, const wchar_t * pwszID ) ;
		// ウェイトマップ情報設定
		void SetWeightLayerInfoAt( size_t iLayer, const WeightMapInfo& wminf ) ;
		// ウェイトマップレイヤー検索
		ssize_t FindWeightLayerAs( const wchar_t * pwszID ) const ;
		// ウェイトマップレイヤー追加
		virtual void InsertWeightLayerAt( size_t iLayer, const wchar_t * pwszID ) ;
		// ウェイトマップレイヤー削除
		virtual void RemoveWeightLayer( size_t iLayer, size_t nCount ) ;
		// ウェイトマップレイヤー入れ替え
		virtual void SwapWeightLayer( size_t iLayer0, size_t iLayer1 ) ;
		// 拡張属性（ボーン用ウェイトマップではない）レイヤー取得
		void GetExAttrWeightLayers( SSystem::SArray<size_t>& aLayers ) const ;
		// ボーン用ウェイトマップを正規化
		void NormalizeBoneWeightMap( void ) ;
		// ボーン用ウェイトマップをGPU用に最適化
		void OptimizeBoneWeightMap( void ) ;

	public:
		// 選択頂点座標操作
		void TransformPoints
			( const SelectPointSet& sps,
				const S3DMatrix& matTrans, const S3DVector& vMove ) ;
		void TransformUVPoints
			( const SelectPointSet& sps,
				const S3DMatrix& matTrans, const S3DVector& vMove ) ;

	public:
		// 選択頂点から有意な頂点へ変換
		void PatchPointsFromSelectedPoints
			( PatchPointSet& pps, const SelectPointSet& sps, float32_t wThreshold = 0.0f ) const ;
		// 選択頂点へ変換（ソート済み）
		void SelectPointsFromPoints
			( SelectPointSet& sps,
				const PatchPointSet& pps, const float32_t * pWeights = NULL ) const ;
		void SelectPointsFromEdges( SelectPointSet& sps, const PatchEdgeSet& pes ) const ;
		void SelectPointsFromLines( SelectPointSet& sps, const PatchLineSet& pls ) const ;
		void SelectPointsFromFaces( SelectPointSet& sps, const PatchFaceSet& pfs ) const ;
		void SelectPointsFromPatchs( SelectPointSet& sps, const SSystem::SArraySet<Patch*>& sp ) const ;
		void AllPatchFacesOfPatchs( PatchFaceSet& pfs, const SSystem::SArraySet<Patch*>& sp ) const ;
		// 選択頂点の正規化（縮退頂点がある場合ペアを必ず含む＆ソート済み）
		void NormalizeSelectPointSet( SelectPointSet& sps ) const ;
		// （ソート済み）選択頂点が対象にする Patch を列挙
		void EnumerateSelectedPatchSet
			( SSystem::SArraySet<Patch*>& aSelPatchs, const SelectPointSet& sps ) const ;
		// （ソート済み）選択頂点から面と稜線へ変換
		void PatchFacesAndEdgesFromPoints
			( PatchFaceSet& pfs, PatchFaceSet& pfsNull,
				PatchEdgeSet& pes, PatchLineSet& pls,
				const SelectPointSet& sps ) const ;
		// （ソート済み）縮退頂点を1つだけ選択して除去
		void DenormalizeSelectPoints( SelectPointSet& sps ) const ;
		// （ソート済み）選択頂点情報のパッチをインデックスに変換してソート
		void IndexPatchForSelectPoints( SelectPointSet& sps ) const ;
		// （ソート済み）選択頂点情報のインデックス化されたパッチをポインタに変換してソート
		void PointerPatchForSelectPoints( SelectPointSet& sps ) const ;
		// （ソート済み）面集合のパッチをインデックスに変換してソート
		void IndexPatchForPatchFaceSet( PatchFaceSet& pfs ) const ;
		// （ソート済み）面集合のインデックス化されたパッチをポインタに変換してソート
		void PointerPatchForPatchFaceSet( PatchFaceSet& pfs ) const ;
		// 含まれないパッチを削除する
		void RemoveInvalidPatchOfSelectPoints( SelectPointSet& sps ) const ;
		void RemoveInvalidPatchOfFaceSet( PatchFaceSet& pfs ) const ;
		void RemoveInvalidPatchOfEdgeSet( PatchEdgeSet& pes ) const ;
		// 稜線からラインへ変換
		bool PatchLineFromEdge( PatchLine& line, const PatchEdge& edge ) const ;
		// パッチの全ての有効面を取得
		void EnumeratePatchValidFaces( PatchFaceSet& pfs, const Patch& patch ) const ;
		// インデックス化された選択点の情報をラインストリップへ
		Patch * NewPatchFromIndexedSelectPoints( const SelectPointSet& spsIndexed ) ;
		// インデックス化された選択点へパッチの頂点情報をペースト
		enum	VertexElementFlag
		{
			vertexElementPoint	= 0x0001,
			vertexElementUV		= 0x0002,
			vertexElementColor	= 0x0004,
			vertexElementWeight	= 0x0008,
		} ;
		void PasteVertexByIndexedSelectPoints
			( const SelectPointSet& spsIndexed,
				const Patch& patch, uint32_t nElementFlags, bool flagReverse ) ;
		void PasteVertexElements
			( const PatchPoint& ppDst,
				const PatchPoint& ppSrc, uint32_t nElementFlags ) ;
		// 頂点要素をテキスト（XML）形式にエクスポートする
		void ExportVertexElements
			( SSystem::SXMLDocument& xmlDoc, const Patch& patch ) const ;
		// 頂点要素をテキスト（XML）形式にインポートする
		Patch * NewImportVertexElements( const SSystem::SXMLDocument& xmlDoc ) ;

	public:
		// メッシュ稜線のパッチ間の縮退情報
		class	PatchEdgeSetSeem	: public PatchEdgeSet
		{
		public:
			uint32_t	m_nFlags ;
		} ;
		typedef	SSystem::SSortObjectArray
					< SSystem::SSortObjectElement<Edge, PatchEdgeSetSeem> >	SeemEdges ;
		struct	EdgeDivRef	: public EdgeDiv
		{
			size_t	iDst ;

			EdgeDivRef( void ) : iDst( 0 ) { }
			EdgeDivRef( const EdgeDiv& edv, size_t i ) : EdgeDiv( edv ), iDst( i ) { }
		} ;
		typedef	SSystem::SPtrSortObjectArray
					< Patch, SSystem::SArray<EdgeDivRef> >	EdgeDivRefMap ;

		// メッシュを細分化複製
		void RedivideMeshFrom
			( const S3DMeshEditor& mesh, const MeshParam& param ) ;
		// メッシュ稜線のパッチ間の縮退情報取得
		void GetPatchSeemEdges( SeemEdges& sedges, const Patch& patch ) const ;
		void GetAllPatchSeemEdges
			( SSystem::SPtrSortObjectArray<Patch,SeemEdges>& psoaEdges ) const ;
		// メッシュ分割サイズ計算
		void CalcRedividedPatchSize
			( size_t& nDivWidth, size_t& nDivHeight,
				const Patch& patch, size_t nDivHorz, size_t nDivVert ) const ;
		// メッシュ稜線補完処理
		static void EdgeBezierInterpolation
			( S3DMeshEditor::Patch::Elements& el0,
				const S3DMeshEditor::Patch::Elements& el1, double t ) ;
		// メッシュ細分化処理
		Patch * NewRedividedPatch
			( size_t nDivWidth, size_t nDivHeight,
				SSystem::SArray<EdgeDivRef>& aSeemDivEdge,
				const Patch& patch, const SeemEdges& sedges,
				DivInterpolationMethod interpolation ) ;
		// パッチポインタ置き換え
		void RemapPatchPointers
			( SeemEdges& sedges,
				const SSystem::SPtrSortArray<Patch,Patch*>& psaMap ) const ;
		void RemapAllPatchPointers
			( SSystem::SObjectArray<SeemEdges>& aSeemEdges,
				const SSystem::SPtrSortArray<Patch,Patch*>& psaMap ) const ;
		// 分割パッチの縮退点を設定
		void DegenerateRedividedPatch
			( Patch * pPatch, const SeemEdges& sedges, const EdgeDivRefMap& edrm ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// メッシュ編集インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshEditorInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMeshEditorInterface, ESLObject )

	public:
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const = 0 ;
		virtual S3DMeshEditor& MeshEditor( void ) = 0 ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) = 0 ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;

	public:
		// パッチ総数取得
		virtual size_t GetPatchCount( void ) const ;
		// パッチ取得
		virtual S3DMeshEditor::Patch * GetPatchAt( size_t i ) const ;
		// パッチ生成
		virtual S3DMeshEditor::Patch * NewPatch( void ) ;
		// パッチ追加
		virtual size_t AddPatch( S3DMeshEditor::Patch * pPatch ) ;
		virtual size_t InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch ) ;
		// パッチ削除
		virtual void RemovePatchAt( size_t i ) ;
		// パッチ検索
		virtual ssize_t FindPatch( S3DMeshEditor::Patch * pPatch ) const ;
		// パッチ順序入れ替え
		virtual void SwapPatchOrder( size_t iPatch0, size_t iPatch1 ) ;

	public:
		// 水平ライン挿入
		virtual void InsertLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine, float_t w = 1.0f ) ;
		// 垂直ライン挿入
		virtual void InsertColumn
			( S3DMeshEditor::Patch * pPatch, size_t iCol, float_t w = 1.0f ) ;
		// 水平ライン削除
		virtual void RemoveLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine ) ;
		// 垂直ライン削除
		virtual void RemoveColumn
			( S3DMeshEditor::Patch * pPatch, size_t iCol ) ;
		// 水平ライン入れ替え
		virtual void SwapLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine1, size_t iLine2 ) ;
		// 頂点縮退フラグ変更
		virtual void ChangeDegeneratedPointFlag
			( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点縮退処理（頂点座標操作は無し）
		virtual void ShrinkPoints
			( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点縮退解除
		virtual void UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp ) ;
		// 値更新通知
		virtual void NotifyUpdateVertex
				( const S3DMeshEditor::SelectPointSet& selPoints ) ;
		virtual void NotifyUpdateFace
				( const S3DMeshEditor::PatchFaceSet& selFaces ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// メッシュ編集ブリッジ
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshEditorBridge	: public S3DMeshEditorInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMeshEditorBridge, S3DMeshEditorInterface )

	public:
		// パッチ追加
		virtual size_t AddPatch( S3DMeshEditor::Patch * pPatch ) ;
		virtual size_t InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch ) ;
		// パッチ削除
		virtual void RemovePatchAt( size_t i ) ;
		// パッチ順序入れ替え
		virtual void SwapPatchOrder( size_t iPatch0, size_t iPatch1 ) ;
		// 水平ライン挿入
		virtual void InsertLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine, float_t w = 1.0f ) ;
		// 垂直ライン挿入
		virtual void InsertColumn
			( S3DMeshEditor::Patch * pPatch, size_t iCol, float_t w = 1.0f ) ;
		// 水平ライン削除
		virtual void RemoveLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine ) ;
		// 垂直ライン削除
		virtual void RemoveColumn
			( S3DMeshEditor::Patch * pPatch, size_t iCol ) ;
		// 水平ライン入れ替え
		virtual void SwapLine
			( S3DMeshEditor::Patch * pPatch, size_t iLine1, size_t iLine2 ) ;
		// 頂点縮退フラグ変更
		virtual void ChangeDegeneratedPointFlag
			( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点縮退処理（頂点座標操作は無し）
		virtual void ShrinkPoints
			( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点縮退解除
		virtual void UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp ) ;
		// 値更新通知
		virtual void NotifyUpdateVertex
				( const S3DMeshEditor::SelectPointSet& selPoints ) ;
		virtual void NotifyUpdateFace
				( const S3DMeshEditor::PatchFaceSet& selFaces ) ;

	protected:
		class	Function
		{
		public:
			virtual void Invoke( S3DMeshEditorBridge * pBridge ) = 0 ;
		} ;
		void InvokeFunction( Function& proc )
		{
			S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
			if ( (pItem == NULL) || !IsSynchronizedMeshEditor() )
			{
				return ;
			}
			S3DMeshEditorBridge *
				pBridge = ESLTypeCast<S3DMeshEditorBridge>( pItem ) ;
			if ( (pBridge != NULL) && (pBridge != this)
					&& pBridge->IsSynchronizedMeshEditor() )
			{
				proc.Invoke( pBridge ) ;
			}
			const size_t	nCtrlCount = pItem->GetControllerCount() ;
			for ( size_t i = 0; i < nCtrlCount; i ++ )
			{
				pBridge = ESLTypeCast<S3DMeshEditorBridge>
								( pItem->GetControllerAt( i ) ) ;
				if ( (pBridge != NULL) && (pBridge != this)
					&& pBridge->IsSynchronizedMeshEditor() )
				{
					proc.Invoke( pBridge ) ;
				}
			}
		}

	public:
		// オーナーアイテム取得
		virtual S3DSceneComposer::ItemSerializer * GetOwnerItem( void ) const = 0 ;
		// メッシュ同期
		virtual bool IsSynchronizedMeshEditor( void ) const = 0 ;

	public:	// メッシュ同期処理
		// パッチ追加
		virtual size_t OnAddPatch( S3DMeshEditor::Patch * pPatch ) ;
		virtual size_t OnInsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch ) ;
		// パッチ削除
		virtual void OnRemovePatchAt( size_t i ) ;
		// パッチ順序入れ替え
		virtual void OnSwapPatchOrder( size_t iPatch0, size_t iPatch1 ) ;
		// 水平ライン挿入
		virtual void OnInsertLine
			( size_t iPatch, size_t iLine, float_t w = 1.0f ) ;
		// 垂直ライン挿入
		virtual void OnInsertColumn
			( size_t iPatch, size_t iCol, float_t w = 1.0f ) ;
		// 水平ライン削除
		virtual void OnRemoveLine( size_t iPatch, size_t iLine ) ;
		// 垂直ライン削除
		virtual void OnRemoveColumn( size_t iPatch, size_t iCol ) ;
		// 水平ライン入れ替え
		virtual void OnSwapLine
			( size_t iPatch, size_t iLine1, size_t iLine2 ) ;
		// 頂点縮退フラグ変更
		virtual void OnChangeDegeneratedPointFlag
			( const S3DMeshEditor::SelectPointSet& sps, uint32_t nFlags ) ;
		// 頂点縮退処理（頂点座標操作は無し）
		virtual void OnShrinkPoints
			( const S3DMeshEditor::SelectPointSet& sps, uint32_t nFlags ) ;
		// 頂点縮退解除
		virtual void OnUntiShrinkPoints( size_t iPatch, size_t iVertex ) ;
		// 値更新通知
		virtual void OnNotifyUpdateVertex
			( const S3DMeshEditor::SelectPointSet& selPointsIndexed,
				const S3DMeshEditor& meshSrc ) ;
		virtual void OnNotifyUpdateFace
			( const S3DMeshEditor::PatchFaceSet& selFacesIndexed,
				const S3DMeshEditor& meshSrc ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// メッシュ編集オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshEditorObject
			: public S3DMeshEditorInterface, public S3DMeshEditor
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DMeshEditorObject, S3DMeshEditorInterface, S3DMeshEditor )
		// 構築関数
		S3DMeshEditorObject( void ) ;
		// 消滅関数
		virtual ~S3DMeshEditorObject( void ) ;

	public:
		class	SelectionWeights	: public SSystem::SArray<float32_t>
		{
		} ;

	protected:
		S3DMeshEditor::MeshParam		m_param ;

		bool							m_flagUpdateEditCollision ;
		S3DCollision					m_colliEditor ;	// 編集用メッシュ当たり判定

		S3DMeshEditor::SelectPointSet	m_spsUVPoints ;	// UV選択用（縮退正規化なし）
		S3DMeshEditor::SelectPointSet	m_spsPoints ;	// 選択頂点（縮退頂点含む）
		S3DMeshEditor::PatchEdgeSet		m_pesEdges ;	// 選択稜線
		S3DMeshEditor::PatchFaceSet		m_pfsFaces ;	// 選択面
		S3DMeshEditor::PatchFaceSet		m_pfsNullFaces ;
		S3DMeshEditor::PatchLineSet		m_plsLines ;	// 選択ライン
		SSystem::SArraySet<Patch*>		m_asSelPatchs ;	// 選択パッチ
		SSystem::SObjectArray<SelectionWeights>
										m_aSelWeights ;	// 選択マスク（全 Patch に対応）
		S3DVector						m_vSelMin ;		// 選択範囲
		S3DVector						m_vSelMax ;
		S2DVector						m_vSelUVMin ;
		S2DVector						m_vSelUVMax ;

		ssize_t							m_iCurPatch ;
		ssize_t							m_iCurWeight ;
		size_t							m_iCurMaterial ;

		S3DMeshBufferPropertySerializer	m_serializer ;

		ESLObject *						m_pEditContext ;
		bool							m_flagOwnEditContext ;

		S3DMeshEditorInterface *		m_pExtraInfo ;

	public:
		// パラメータ
		const S3DMeshEditor::MeshParam& GetMeshParameter( void ) const ;
		S3DMeshEditor::MeshParam& MeshParameter( void ) ;
		void SetMeshParameter( const S3DMeshEditor::MeshParam& param ) ;

		// GetMeshMaterial, GetMeshItemMatrix リダイレクト先
		void AttachExtraMeshInfo( S3DMeshEditorInterface * pMeshEditor ) ;

	public:
		// 更新パッチを再構築する
		virtual void UpdateAllPatchs( const MeshParam& param ) ;
		// 編集メッシュコリジョン更新
		void UpdateEditCollision( void ) ;
		// 編集メッシュコリジョン範囲取得
		double GetEditCollisionCircumscribedSpher( S3DVector& vCenter ) ;
		// 当たり判定
		enum	HitRayFlag
		{
			hitRayNoNullFace	= 0x0001,
			hitRayNoBackFace	= 0x0002,
		} ;
		struct	HitRayResult
		{
			Patch *		pPatch ;
			size_t		iFace ;
			S3DVector	vHitPos ;
			S3DVector	vNormal ;
			S2DVector	vUV ;
			S3DColor	clrVertex ;
		} ;
		bool HitRayToEditMesh
			( const S3DDVector& vPos0, const S3DDVector& vPos1,
				float fpErrorGap, uint32_t nFlags,
				HitRayResult& hrrResult,
				Patch *const * ppExclusionPatchs = NULL, size_t nExclusionPatchs = 0 ) ;
	protected:
		struct	HitRayToEditMeshInstance
		{
			S3DDVector		vRay ;
			uint32_t		nFlags ;
			Patch *const *	ppExclusionPatchs ;
			size_t			nExclusionPatchs ;
		} ;
		static S3DCollision::HitColliderCallback
			Callback_OnHitRayToEditMesh
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;

	public:
		// シリアライズ
		const SSystem::SString& Serialize( void ) ;
		void SerializeBinary
			( uint8_t * pbytBinary, size_t nBufBytes, size_t nHeaderBytes ) ;
		size_t SerializeBuffer( size_t& nHeaderBytes ) ;
	public:
		// デシリアライズ
		void Deserialize( const wchar_t * pwszBase64 ) ;
		bool DeserializeBinary( const void * pbytBinary, size_t nBytes ) ;

	public:
		// 現在のパッチ編集選択状態
		ssize_t GetCurrentPatch( void ) const ;
		void SetCurrentPatch( ssize_t iPatch ) ;
		// 現在のウェイトマップ編集選択状態
		ssize_t GetCurrentWeightLayer( void ) const ;
		void SetCurrentWeightLayer( ssize_t iLayer ) ;
		// 現在選択中のマテリアル
		size_t GetCurrentMaterial( void ) const ;
		void SetCurrentMaterial( size_t iMaterial ) ;

	public:
		// 全選択解除
		void ClearAllSelection( void ) ;
		// 頂点選択マスク取得
		const SSystem::SArray<float32_t> *
					GetSelectionWeights( size_t iPatch ) const ;
		// 選択領域取得
		const S3DVector& GetSelectedRectMin( void ) const ;
		const S3DVector& GetSelectedRectMax( void ) const ;
		const S2DVector& GetSelectedRectUVMin( void ) const ;
		const S2DVector& GetSelectedRectUVMax( void ) const ;
		// 頂点選択
		const S3DMeshEditor::SelectPointSet& GetSelectedPoints( void ) const ;
		const S3DMeshEditor::SelectPointSet& GetSelectedUVPoints( void ) const ;
		void SetSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints ) ;
		void AddSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints ) ;
		void RemoveSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints ) ;
		void UpdateSelectedPointWeights( void ) ;
		void UpdateSelectedPointMinMax( void ) ;
		void NormalizeSelecedPoints( void ) ;
		// 稜線選択
		const S3DMeshEditor::PatchEdgeSet& GetSelectedEdges( void ) const ;
		void SetSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges ) ;
		void AddSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges ) ;
		void RemoveSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges ) ;
		void UpdateSelecedPointByEdges( void ) ;
		void NormalizeSelecedEdges( void ) ;
		// 面選択
		const S3DMeshEditor::PatchFaceSet& GetSelectedFaces( void ) const ;
		const S3DMeshEditor::PatchFaceSet& GetSelectedNullFaces( void ) const ;
		void SetSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		void AddSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		void RemoveSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		void UpdateSelecedPointByFaces( void ) ;
		void NormalizeSelecedFaces( void ) ;
		// ライン選択
		const S3DMeshEditor::PatchLineSet& GetSelectedLines( void ) const ;
		void SetSelectedLines( const S3DMeshEditor::PatchLineSet& selLines ) ;
		void AddSelectedLines( const S3DMeshEditor::PatchLineSet& selLines ) ;
		void RemoveSelectedLines( const S3DMeshEditor::PatchLineSet& selLines ) ;
		void UpdateSelecedPointByLines( void ) ;
		void NormalizeSelecedLines( void ) ;
		// パッチ選択
		const SSystem::SArraySet<Patch*>& GetSelectedPatchs( void ) const ;
		void SetSelectedPatchs( const SSystem::SArraySet<Patch*>& selPatchs ) ;
		void AddSelectedPatchs( const SSystem::SArraySet<Patch*>& selPatchs ) ;
		void RemoveSelectedPatchs( const SSystem::SArraySet<Patch*>& selPatchs ) ;
		void UpdateSelecedPointByPatchs( void ) ;
		void NormalizeSelecedPatchs( void ) ;
		// 選択1要素取得
		const S3DMeshEditor::SelectPoint * GetSelectedPoint( void ) const ;
		const S3DMeshEditor::PatchLine * GetSelectedLine( void ) const ;

	public:
		// 稜線連鎖
		typedef	SSystem::SArray<S3DMeshEditor::PatchPoint>	EdgeChain ;
		// 稜線集合の中から指定頂点を含む稜線検索
		static ssize_t FindEdgeIncludePointAs
			( const S3DMeshEditor::PatchEdgeSet& pes, const S3DMeshEditor::PatchPoint& pp ) ;
		// 選択面輪郭取得
		void GetEdgeChainOfFaceOutline
			( SSystem::SObjectArray<EdgeChain>& aEdgeChains,
					const S3DMeshEditor::PatchFaceSet& selFaces ) const ;
		// 輪郭順序正規化
		static void NormalizeEdgeChainOrders
			( SSystem::SObjectArray<EdgeChain>& aEdgeChains,
					const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		static void NormalizeEdgeChainOrder
			( EdgeChain& edgeChain, const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		// 輪郭線が単一パッチの単一ラインか？
		static bool IsEdgeChainSinglePatchLine
			( S3DMeshEditor::PatchLine& pl, const EdgeChain& edgeChain ) ;
		// 面部分選択か全体選択か？
		static bool IsFullSelectedFace
			( const S3DMeshEditor::Patch& patch,
				const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		// 面選択部分のみを複製して新規パッチ作成
		S3DMeshEditor::Patch * DuplicatePartialPatch
			( SGLPoint& ptOffset,
				const S3DMeshEditor::Patch& patch,
				const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		// 面選択部分の面更新
		void ChangeSelectedFace
			( S3DMeshEditor::Patch& patch, int8_t face,
				const S3DMeshEditor::PatchFaceSet& selFaces ) ;
		// 面非選択部分の面更新
		void ChangeUnselectedFace
			( S3DMeshEditor::Patch& patch, int8_t face,
				const S3DMeshEditor::PatchFaceSet& selFaces ) ;

	public:
		// 編集コンテキスト
		void AttachEditContext( ESLObject * pContext, bool flagAutoDelete ) ;
		ESLObject * GetEditContext( void ) const ;

	public:	// 編集ツール
		// 選択頂点座標操作
		void DoTransformPoints
			( const S3DMatrix& matTrans, const S3DVector& vMove,
				S3DMeshEditorInterface * pEdit ) ;
		void DoTransformUVPoints
			( const S3DMatrix& matTrans, const S3DVector& vMove,
				S3DMeshEditorInterface * pEdit ) ;
		// 選択点の縮退フラグ取得
		uint32_t GetSelectedDegenerateFlag( void ) const ;
		// 選択点の縮退フラグ変更
		void DoChangeSelectedDegenerateFlag
			( uint32_t nFlags, S3DMeshEditorInterface * pEdit ) ;
		// 選択頂点縮退
		void DoShrink( uint32_t nFlags, S3DMeshEditorInterface * pEdit ) ;
		// 近接頂点縮退
		enum	ShrinkTarget
		{
			shrinkAny,
			shrinkHorz,
			shrinkVert,
		} ;
		struct	ShrinkParam
		{
			ShrinkTarget	target ;
			float32_t		gap ;
		} ;
		void DoShrinkNearPoints
			( const ShrinkParam& param, uint32_t nFlags, S3DMeshEditorInterface * pEdit ) ;
		// 選択頂点縮退解除
		void DoUntiShrink( S3DMeshEditorInterface * pEdit ) ;
		// 頂点整列
		enum	ArrangeAxis
		{
			arrangeAxisX,
			arrangeAxisY,
			arrangeAxisZ,
			arrangeAxisU,
			arrangeAxisV,
		} ;
		enum	ArrangeOperation
		{
			arrangeMin,
			arrangeMax,
			arrangeCenter,
			arrangeNumber,
		} ;
		struct	ArrangeParam
		{
			ArrangeAxis			axis ;
			ArrangeOperation	op ;
			float32_t			num ;
		} ;
		void DoArrange( const ArrangeParam& param, S3DMeshEditorInterface * pEdit ) ;
	protected:
		typedef float32_t (*PFUNC_GET_VECTOR_ELEMENT)
						( const S3DVector& v, const S2DVector& uv ) ;
		typedef void (*PFUNC_SET_VECTOR_ELEMENT)
						( S3DVector& v, S2DVector& uv, float32_t n ) ;
		static const PFUNC_GET_VECTOR_ELEMENT	m_pfnGetVectorElement[5] ;
		static const PFUNC_SET_VECTOR_ELEMENT	m_pfnSetVectorElement[5] ;
		static float32_t GetVectorElementOfX( const S3DVector& v, const S2DVector& uv ) ;
		static float32_t GetVectorElementOfY( const S3DVector& v, const S2DVector& uv ) ;
		static float32_t GetVectorElementOfZ( const S3DVector& v, const S2DVector& uv ) ;
		static float32_t GetVectorElementOfU( const S3DVector& v, const S2DVector& uv ) ;
		static float32_t GetVectorElementOfV( const S3DVector& v, const S2DVector& uv ) ;
		static void SetVectorElementOfX( S3DVector& v, S2DVector& uv, float32_t n ) ;
		static void SetVectorElementOfY( S3DVector& v, S2DVector& uv, float32_t n ) ;
		static void SetVectorElementOfZ( S3DVector& v, S2DVector& uv, float32_t n ) ;
		static void SetVectorElementOfU( S3DVector& v, S2DVector& uv, float32_t n ) ;
		static void SetVectorElementOfV( S3DVector& v, S2DVector& uv, float32_t n ) ;
	public:
		// 頂点要素フィル
		void DoFillElementOfColorMul( const SGLPalette& rgb, S3DMeshEditorInterface * pEdit ) ;
		void DoFillElementOfColorAdd( const SGLPalette& rgb, S3DMeshEditorInterface * pEdit ) ;
		void DoFillElementOfColorAlpha( uint8_t alpha, S3DMeshEditorInterface * pEdit ) ;
		void DoFillElementOfColorAddAlpha( uint8_t alpha, S3DMeshEditorInterface * pEdit ) ;
		void DoFillElementOfWeightMap( size_t iWeight, float32_t w, S3DMeshEditorInterface * pEdit ) ;
		// 頂点要素サンプリング
		bool DoSampleElementOfColorMul( SGLPalette& rgb ) const ;
		bool DoSampleElementOfColorAdd( SGLPalette& rgb ) const ;
		bool DoSampleElementOfColorAlpha( uint8_t& alpha ) const ;
		bool DoSampleElementOfColorAddAlpha( uint8_t& alpha ) const ;
		bool DoSampleElementOfWeightMap( size_t iWeight, float32_t& w ) const ;

	public:
		// 選択点を共有するパッチ外ポリゴンの頂点を分離する
		void DoDivideExFaceVertex( S3DMeshEditorInterface * pEdit ) ;

	public:
		// 面掃引準備
		void PrepareToSweepFace
			( bool flagMakeBack, S3DMeshEditorInterface * pEdit ) ;
	protected:
		struct	SweepedPatch
		{
			S3DMeshEditor::Patch *	pPatch ;
			SGLPoint				ptOffset ;
		} ;

	public:
		// 面反転
		void DoInverseFace
			( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit ) ;
		// 面更新
		void DoChangeFace
			( const S3DMeshEditor::PatchFaceSet& pfs,
					int8_t face, S3DMeshEditorInterface * pEdit ) ;
		// 面シフタ変更
		void DoShiftFace
			( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit ) ;
		// 選択面を別パッチへ分離
		void DoSeparatePatchFace
			( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit ) ;

	public:
		// パッチの順序反転（表裏反転）
		void DoInversePatch
			( const SSystem::SArraySet<Patch*>& ps, S3DMeshEditorInterface * pEdit ) ;

	public:
		// ライン縮退
		void DoShrinkLine
			( const S3DMeshEditor::PatchLine& line0,
				const S3DMeshEditor::PatchLine& line1,
				uint32_t nFlags, bool flagGap,
				const S3DMeshEditor::PatchPointSet& xptPoints,
				S3DMeshEditorInterface * pEdit ) ;
		// 縮退済みライン判定
		bool IsShrinkedLine
			( const S3DMeshEditor::PatchLineSet& pls ) const ;
		// ライン融合
		void DoMeltLines
			( const S3DMeshEditor::PatchLineSet& pls, S3DMeshEditorInterface * pEdit ) ;
		// ライン削除
		void DoRemoveLines
			( const S3DMeshEditor::PatchLineSet& pls, S3DMeshEditorInterface * pEdit ) ;

	public:
		// ラインに面を張る
		void DoPutUpFaceIntoLine
			( const S3DMeshEditor::PatchLine& line0, S3DMeshEditorInterface * pEdit ) ;

	public:
		// ベベル処理準備
		struct	BevelPoint
		{
			S3DMeshEditor::PatchPoint	pp ;
			S3DVector					vPos0 ;
			S3DVector					vPos1 ;
		} ;
		void PrepareBevel
			( SSystem::SArray<BevelPoint>& aBevelPoints,
				S3DMeshEditor::SelectPointSet& spsPoints,
				bool flagBoth, bool flagGap, S3DMeshEditorInterface * pEdit ) ;
		// ベベル処理
		void DoBevelPoints
			( const SSystem::SArray<BevelPoint>& aBevelPoints,
				const S3DMeshEditor::SelectPointSet& spsPoints,
				double fpBevel, S3DMeshEditorInterface * pEdit ) ;

	public:
		// ループスライス
		struct	SlicePoint
		{
			S3DMeshEditor::PatchLine	pl ;
			double						w ;
		} ;
		static bool SlicePointFromEdgePoint
			( SlicePoint& sp, const S3DMeshEditor::PatchEdge& pe, double w ) ;
		static size_t CountPatchOfSlicePoints
			( const SSystem::SArray<SlicePoint>& aChain, Patch * pPatch ) ;
		static ssize_t FindSlicePoints
			( const SSystem::SArray<SlicePoint>& aChain, const SlicePoint& sp ) ;
		static void OffsetIndexByInsertLine
			( S3DMeshEditor::PatchPointSet& ppsLine, Patch * pPatch, size_t iLine ) ;
		static void OffsetIndexByInsertColumn
			( S3DMeshEditor::PatchPointSet& ppsLine, Patch * pPatch, size_t iCol ) ;
		bool EnumerateChainedSlice
			( SSystem::SArray<SlicePoint>& aChain,
				const S3DMeshEditor::PatchEdge& pe, double w ) const ;
		void CalcSlicePoints
			( SSystem::SArray<S3DVector4>& aPoints,
				const SlicePoint& sp, bool flagCloseEndPoint ) const ;
		void DoLoopSlice
			( S3DMeshEditor::PatchPointSet& ppsLine, const SlicePoint& sp,
				bool flagCloseEndPoint, S3DMeshEditorInterface * pEdit ) ;
		void DoLoopSlices
			( S3DMeshEditor::PatchPointSet& ppsLine,
				const SSystem::SArray<SlicePoint>& aChain,
				S3DMeshEditorInterface * pEdit ) ;

	public:
		// UV展開
		enum	UVDevelopMethod
		{
			uvDevOrthogonal,
			uvDevPolarMap,
			uvDevSphereMap,
			uvDevPatchOrder,
			uvDevPatchModified,
			uvDevPatchOrthogonal,
		} ;
		struct	UVMapDevParam
		{
			UVDevelopMethod	method ;
			ArrangeAxis		axis ;
			S3DVector		vCenter ;
			S3DMatrix		matSpace ;
			S3DMatrix		matISpace ;
			S3DVector		vSpace ;
			bool			arrangePatch ;
			bool			autoPolarMap ;
			bool			onlySelected ;
			bool			globalSpace ;
		} ;
		void DoUVMapDevelopment
			( const UVMapDevParam& param, S3DMeshEditorInterface * pEdit ) ;
	protected:
		struct	UVMapDevResult
		{
			S2DVector	vMinUV ;
			S2DVector	vMaxUV ;
			double		fpPolarRadius ;
		} ;
		struct	LineWidth
		{
			float32_t	fpPos ;
			float32_t	fpWidth ;
			float32_t	fpMaxWidth ;
		} ;
		struct	LineRange
		{
			size_t		nIndex ;
			size_t		nCount ;
			float32_t	fpWidth ;
			float32_t	fpMaxWidth ;
		} ;
		typedef S2DVector (*PFUNC_UV_PROJECTION)
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionOrthogonalX
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionOrthogonalY
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionOrthogonalZ
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionPolarMapX
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionPolarMapY
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionPolarMapZ
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionSphereMapX
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionSphereMapY
				( const UVMapDevParam& param,
					UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static S2DVector UVProjectionSphereMapZ
				( const UVMapDevParam& param,
						UVMapDevResult& uvmapdRes, const S3DVector& vPos ) ;
		static const PFUNC_UV_PROJECTION	m_pfnUVProjection[3][3] ;
	protected:
		bool IsPatchUVSelected( Patch * pPatch ) const ;
		bool DoUVMapDevelopmentOfPatch
			( Patch * pPatch, UVMapDevResult& uvmapdRes,
				const UVMapDevParam& param, S3DMeshEditorInterface * pEdit ) ;
		bool DoUVMapDevelopmentOfPatchOrder
			( Patch * pPatch, UVMapDevResult& uvmapdRes,
				const UVMapDevParam& param, S3DMeshEditorInterface * pEdit ) ;
		struct	PolarUVMapInfo
		{
			Line		linePolar ;
			size_t		nLineWidth ;
			size_t		nColHeight ;
			SGLPoint	ptPolar ;
			SGLPoint	ptHorzDelta ;
			SGLPoint	ptVertDelta ;
			bool		flagHorzLoop ;
		} ;
		bool IsPolarUVMapOfPatchOrderBlock
			( PolarUVMapInfo& puvInfo,
				Patch * pPatch, const LineRange& lrXRange, const LineRange& lrYRange ) ;
		void DoUVMapDevelopmentOfPatchOrderBlock
			( Patch * pPatch, const LineRange& lrXRange,
				const LineRange& lrYRange,
				float32_t xRangeLeft, float32_t yRangeTop,
				float32_t xRangeWidth, float32_t yRangeHeight,
				float32_t fpTotalWidth, float32_t fpTotalHeight,
				const UVMapDevParam& param ) ;
		void DoUVMapDevelopmentOfPatchOrderModified
			( Patch * pPatch, const LineRange& lrXRange,
				const LineRange& lrYRange,
				float32_t xRangeLeft, float32_t yRangeTop,
				float32_t xRangeWidth, float32_t yRangeHeight,
				float32_t fpTotalWidth, float32_t fpTotalHeight,
				const UVMapDevParam& param ) ;
		void DoPolarUVMapDevelopmentOfPatchOrder
			( Patch * pPatch, const PolarUVMapInfo& puvInfo,
				float32_t xRangeLeft, float32_t yRangeTop,
				float32_t xRangeWidth, float32_t yRangeHeight,
				const UVMapDevParam& param ) ;
		void DoUVMapDevelopmentOfPatchBlockOrthogonal
			( Patch * pPatch, const LineRange& lrXRange,
				const LineRange& lrYRange,
				float32_t xRangeLeft, float32_t yRangeTop,
				float32_t xRangeWidth, float32_t yRangeHeight,
				float32_t fpTotalWidth, float32_t fpTotalHeight,
				const UVMapDevParam& param ) ;
		bool GetUVMinMaxOfPatchOrderBlock
			( S2DVector& vUVMin, S2DVector& vUVMax,
				Patch * pPatch, const PolarUVMapInfo& puvInfo ) const ;
		void ScaleUVMapOfPatchOrderBlock
			( Patch * pPatch, const PolarUVMapInfo& puvInfo,
				const float32_t fpScaleUV, const S2DVector& vOffsetUV ) ;
		void AffineUVMapOfPatchOrderBlock
			( Patch * pPatch, const PolarUVMapInfo& puvInfo, const SGLAffine& affine ) ;
		void DoScaleUVMapOfPatch
			( Patch * pPatch, UVMapDevResult& uvmapdRes,
				const S2DVector& vScale, const S2DVector& vOffset,
				const UVMapDevParam& param, S3DMeshEditorInterface * pEdit ) ;

	public:	// S3DMeshEditorInterface
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const ;
		virtual S3DMeshEditor& MeshEditor( void ) ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;

	public:
		// パッチ総数取得
		virtual size_t GetPatchCount( void ) const ;
		// パッチ取得
		virtual S3DMeshEditor::Patch * GetPatchAt( size_t i ) const ;
		// パッチ生成
		virtual S3DMeshEditor::Patch * NewPatch( void ) ;
		// パッチ追加
		virtual size_t AddPatch( S3DMeshEditor::Patch * pPatch ) ;
		virtual size_t InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch ) ;
		// パッチ削除
		virtual void RemovePatchAt( size_t i ) ;
		// パッチ検索
		virtual ssize_t FindPatch( S3DMeshEditor::Patch * pPatch ) const ;
		// パッチ順序入れ替え
		virtual void SwapPatchOrder( size_t iPatch0, size_t iPatch1 ) ;

	public:
		// ウェイトマップレイヤー追加
		virtual void InsertWeightLayerAt( size_t iLayer, const wchar_t * pwszID ) ;
		// ウェイトマップレイヤー削除
		virtual void RemoveWeightLayer( size_t iLayer, size_t nCount ) ;
		// ウェイトマップレイヤー入れ替え
		virtual void SwapWeightLayer( size_t iLayer0, size_t iLayer1 ) ;

	public:
		// 頂点縮退処理（頂点座標操作は無し）
		virtual void ShrinkPoints
			( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags ) ;
		// 頂点縮退解除
		virtual void UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// ボーン物理演算パラメータ
	//////////////////////////////////////////////////////////////////////////

	class	S3DBonePhysMaterialSerializer
					: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramAttenuation	= ItemBasicSerializer::paramItemTotalCount,
			paramShrinkable,
			paramElasticity,
			paramMinStretch,
			paramMaxStretch,
			paramHardness,
			paramEffect,
			paramLimitedAngle,
			paramCollisionRadius,
			paramFrictionalResistance,
			paramPhysExFlags1,
			paramPhysMaterialTotalCount,
			paramPhysMaterialCount = paramPhysMaterialTotalCount - paramAttenuation,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramPhysMaterialCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		S3DModelBoneSpace::PhysMaterial	m_physMaterial ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBonePhysMaterialSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DBonePhysMaterialSerializer, bone_phys_material )
		// 構築関数
		S3DBonePhysMaterialSerializer( void ) ;
		// 消滅関数
		virtual ~S3DBonePhysMaterialSerializer( void ) ;

	public:
		// 物理演算パラメータ
		S3DModelBoneSpace::PhysMaterial& PhysMaterial( void ) ;
		const S3DModelBoneSpace::PhysMaterial& GetPhysMaterial( void ) const ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ボーンアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DBoneSerializer	: public S3DSceneComposer::SpaceSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramBoneHandle	= SpaceSerializer::paramSpaceTotalCount,
			paramEnableBendDir,
			paramUseParentAxis,
			paramIKTerminate,
			paramBendDir,
			paramBendMaxAngle,
			paramBendWeight,
			paramOrgMatrix,
			paramEnablePhysics,
			paramDisableCollision,
			paramPoseWeight,
			paramPhysMaterialRef,
			paramAttenuation,
			paramShrinkable,
			paramElasticity,
			paramMinStretch,
			paramMaxStretch,
			paramHardness,
			paramEffect,
			paramLimitedAngle,
			paramCollisionRadius,
			paramFrictionalResistance,
			paramPhysExFlags1,
			paramCmdParamRefPoseLib,
			paramCmdParamPoseID,
			paramCmdResetMatrix,
			paramCmdReverseBone,
			paramCmdApplyRefPhysParams,
			paramCmdCopyPhysParams,
			paramCmdRegisterPose,
			paramCmdRestorePose,
			paramBoneTotalCount,
			paramBoneCount = paramBoneTotalCount - paramBoneHandle,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramBoneCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;
		static const wchar_t *	m_pwszPresetName[S3DModelBoneSpace::PhysMaterial::presetCount] ;

		S3DDVector						m_vBoneHandle ;
		S3DDVector						m_vBendDir ;
		S4DDMatrix						m_mat4OrgMatrix ;
		bool							m_flagBendDir ;
		bool							m_flagParentAxis ;
		bool							m_flagIKTerminate ;
		bool							m_flagBonePhysics ;
		bool							m_flagNoHitCollision ;
		double							m_fpBendMaxAngle ;
		double							m_fpBendWeight ;
		double							m_fpPoseWeight ;
		S3DDQuaternion					m_qPhysRotation ;
		S3DModelBoneSpace::PhysMaterial	m_physMaterial ;
		SSystem::SString				m_strPhysMaterialRef ;
		S3DBonePhysMaterialSerializer *	m_pPhysMaterial ;

		bool							m_flagPhysCurrent ;
		S3DModelBoneSpace::PhysVertex	m_physCurrent ;

		SSystem::SString				m_strRefPoseLib ;
		SSystem::SString				m_strRefPoseID ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBoneSerializer, SpaceSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DBoneSerializer, bone_space )
		// 構築関数
		S3DBoneSerializer( void ) ;
		// 消滅関数
		virtual ~S3DBoneSerializer( void ) ;

	public:
		// ボーンハンドル
		const S3DDVector& GetBoneHandle( void ) const ;
		void SetBoneHandle( const S3DDVector& vHandle ) ;
		// IK曲げ方向ベクトル
		bool IsEnabledBendDir( void ) const ;
		void EnableBendDir( bool flagBendDir ) ;
		const S3DDVector& GetBendDir( void ) const ;
		void SetBendDir( const S3DDVector& vDir ) ;
		// IK親ボーン回転軸ベクトル・フラグ
		bool GetBendParentAxisFlag( void ) const ;
		void SetBendParentAxisFlag( bool flagParentAxis ) ;
		// IK終端フラグ
		bool GetIKTerminateFlag( void ) const ;
		void SetIKTerminateFlag( bool flagIKTerminate ) ;
		// IK曲げ最大角 [deg]
		double GetBendMaxAngle( void ) const ;
		void SetBendMaxAngle( double degAngle ) ;
		// IK曲げ重み
		double GetBendWeight( void ) const ;
		void SetBendWeight( double fpWeight ) ;
		// オリジナル行列
		const S4DDMatrix& GetOrgMatrix( void ) const ;
		void SetOrgMatrix( const S4DDMatrix& mat4Org ) ;
		// 物理演算有効
		bool IsEnabledPhysics( void ) const ;
		void EnablePhysics( bool flagPhys ) ;
		// 物理演算当たり判定無効
		bool IsDisabledCollision( void ) const ;
		void DisableCollision( bool flagNoHit ) ;
		// ポーズ適用度
		double GetPoseWeight( void ) const ;
		void SetPoseWeight( double fpWeight ) ;
		// 物理演算パラメータ
		S3DModelBoneSpace::PhysMaterial& PhysMaterial( void ) ;
		const S3DModelBoneSpace::PhysMaterial& GetPhysMaterial( void ) const ;
		// 物理演算パラメータパレット
		const SSystem::SString& GetPhysMaterialRef( void ) const ;
		void SetPhysMaterialRef( const wchar_t * pwszMaterial ) ;
		void UpdatePhysMaterialRef( void ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:	// CommonSerializer
		// 変換行列更新
		virtual void UpdateSpaceMatrix( void ) ;

	public:
		// タイマー処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	protected:
		// 物理演算
		void CalculateSubPhysics
			( const S3DModelBoneSpace::PhysMaterial& mtrl,
				const S3DDMatrix& matBone,
				const S3DDVector& vBone,
				double secPast, S3DCollider * pCollider ) ;

	protected:
		// ボーン行列リセット
		void CmdResetBoneMatrix( S3DCompositionEditorInterface * pEditor ) ;
		// ボーン左右反転
		void CmdReverseBone( S3DCompositionEditorInterface * pEditor ) ;
		// 物理演算マテリアルをボーンに反映
		void CmdApplyPhysParams( S3DCompositionEditorInterface * pEditor ) ;
		// 全子ボーンに物理マテリアルを複製
		void CmdCopyChildrenPhysPamras( S3DCompositionEditorInterface * pEditor ) ;
		void CmdCopyPhysPamrasFrom
			( const S3DModelBoneSpace::PhysMaterial& physMaterial,
				const wchar_t * pwszPhysMaterial, S3DCompositionEditorInterface * pEditor ) ;
		// ポーズ登録
		void CmdRegisterPose
			( S3DSceneComposer::Composition * pComp, S3DModelPose * pPose ) ;
		// ポーズ展開
		void CmdRestorePose
			( S3DSceneComposer::Composition * pComp,
				S3DModelPose * pPose,
				S3DCompositionEditorInterface * pEditor ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// マーカーアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DMarkerItemSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramMarkerType		= ItemBasicSerializer::paramItemTotalCount,
			paramShapeType,
			paramColliderClass,
			paramDirection,
			paramRadius,
			paramLength,
			paramMarkerTotalCount,
			paramMarkerCount		= paramMarkerTotalCount - paramMarkerType,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramMarkerCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		S3DModelData::MarkerInfo::Type	m_type ;
		S3DModelData::MarkerInfo::Shape	m_shape ;
		int32_t							m_iCollider ;
		S3DDVector						m_vDirection ;
		double							m_fpRadius ;
		double							m_fpLength ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DMarkerItemSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DMarkerItemSerializer, marker )
		// 構築関数
		S3DMarkerItemSerializer( void ) ;
		// 消滅関数
		virtual ~S3DMarkerItemSerializer( void ) ;

	public:
		// マーカータイプ
		S3DModelData::MarkerInfo::Type GetMarkerType( void ) const ;
		void SetMarkerType( S3DModelData::MarkerInfo::Type type ) ;
		// 形状
		S3DModelData::MarkerInfo::Shape GetShapeType( void ) const ;
		void SetShapeType( S3DModelData::MarkerInfo::Shape shape ) ;
		// 当たり判定クラス
		int32_t GetColliderClass( void ) const ;
		void SetColliderClass( int32_t iCollider ) ;
		// 方向
		S3DDVector GetMarkerDirection( void ) const ;
		void SetMarkerDirection( const S3DDVector& vDir ) ;
		// 半径
		double GetMarkerRadius( void ) const ;
		void SetMarkerRadius( double fpRadius ) ;
		// 長さ
		double GetMarkerLength( void ) const ;
		void SetMarkerLength( double fpLength ) ;
		// マーカー情報
		void GetMarkerInfo( S3DModelData::MarkerInfo& marker ) const ;

	public:	// ParameterProperty
		// パラメータ値取得
			virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// 当たり判定追加
		//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
		virtual void OnItemRenderCollision
			( const S3DScene& scene, S3DCollision& render ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// メッシュ編集アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshEditorSerializer
				: public S3DSceneComposer::ItemBasicSerializer,
					public S3DMeshEditorBridge,
					public S3DParticleSerializer::RenderTarget,
					public S3DInstancingItemInterface
	{
	public:
		// メッシュ・コントローラー
		class	MeshController	: public S3DSceneComposer::Controller
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MeshController, Controller )
			// 構築関数
			MeshController( const wchar_t * pwszClassID ) ;
			// メッシュの変形
			virtual S3DMeshEditor * ModifyMeshEditor
				( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) = 0 ;
		} ;

	public:
		enum	ParameterIndex
		{
			paramMeshEditor		= ItemBasicSerializer::paramItemTotalCount,
			paramMeshPrimitiveType,
			paramMeshEdgeMethod,
			paramMeshEdgeByAngle,
			paramMeshEdgeAngle,
			paramMeshDivAhead,
			paramMeshDivMethod,
			paramMeshDivHorz,
			paramMeshDivVert,
			paramMaterial0,
			paramMaterial1,
			paramMaterial2,
			paramMaterial3,
			paramCollision,
			paramCollisionAll,
			paramColliderFlags,
			paramCollisionAlpha,
			paramColDivHorz,
			paramColDivVert,
			paramFreezeMesh,
			paramEnableBone,
			paramBoneRoot,
			paramBoneAutoMap,
			paramBoneClearMap,
			paramBoneMatrixMap,
			paramBoneNormalizeWeight,
			paramBoneOptimizeWeight,
			paramInstancing,
			paramSortInstance,
			paramBakeInstanceToMesh,
			paramMeshTotalCount,
			paramMeshCount		= paramMeshTotalCount - paramMeshEditor,
			paramMaterialCount	= 4,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramMeshCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiMeshEdgeMethods[5] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiMeshDivMethods[3] ;

	protected:
		S3DMeshEditorObject			m_mesh ;
		S3DItemInstancingSerializer	m_instancing ;

		S3DMaterial *				m_pMaterials[paramMaterialCount] ;
		SSystem::SString			m_strMaterialIDs[paramMaterialCount] ;

		S3DVertexBuffer				m_vbEditMesh[paramMaterialCount] ;
		size_t						m_nMeshBufCount[paramMaterialCount] ;

		bool						m_flagCollision ;
		bool						m_flagCollisionAll ;
		bool						m_flagCollisionUpdate ;
		uint32_t					m_maskCollisionFlags ;
		int32_t						m_nCollisionAlpha ;
		uint32_t					m_nCollisionDivHorz ;
		uint32_t					m_nCollisionDivVert ;
		S3DCollision				m_collision[paramMaterialCount] ;
		size_t						m_nColBufCount[paramMaterialCount] ;

		bool						m_flagDivMeshAhead ;
		bool						m_flagFreezeMesh ;
		bool						m_flagMeshUpdated ;
		bool						m_flagLastMeshCtrled ;

		bool						m_flagEnableBone ;
		SSystem::SString			m_strBoneRoot ;
		SSystem::SSmartReference<S3DScene::Space>
									m_refBoneRoot ;

		SSystem::SArray<S4DMatrix>	m_aTempMatrixs ;
		SSystem::SArray<S3DColor>	m_aTempColors ;

		SSystem::SCriticalSection	m_csLock ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO4
			( S3DMeshEditorSerializer,
				ItemBasicSerializer,
				S3DMeshEditorBridge,
				RenderTarget, S3DInstancingItemInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DMeshEditorSerializer, mesh_editor )
		// 構築関数
		S3DMeshEditorSerializer( void ) ;
		// 消滅関数
		virtual ~S3DMeshEditorSerializer( void ) ;

	public:
		// マテリアル関連付け
		void AttachMaterial
			( size_t i, S3DMaterial * pMaterial, const wchar_t * pwszMaterialID ) ;
		// マテリアル参照更新
		void UpdateMaterialRef( void ) ;
		// ボーン参照更新
		void UpdateBoneRef( void ) ;
		// モデルバッファ更新（静的メッシュ）
		void UpdateEditorMesh( S3DMeshEditor& mesh ) ;
		void UpdateEditorCollision( S3DMeshEditor& mesh ) ;
		// MeshController 処理
		void ProcessMeshControllers( S3DScene& scene ) ;
		bool MakeMeshWithControllers
			( S3DScene& scene, S3DMeshEditor*& pMesh,
				SSystem::SSmartPointer<S3DMeshEditor>& pTempMesh ) ;
		bool ProcessMeshWithBone( S3DMeshEditor& mesh ) const ;
		void BakeMeshBufferController
			( S3DMeshEditor& meshDst,
				S3DMeshBufferItemSerializer::MeshController& ctrl,
				S3DScene& scene,
				S3DRenderBuffer* pTempRenderBuf[paramMaterialCount] ) ;

	public:
		// ボーン有効
		bool IsEnabledBone( void ) const ;
		bool IsValidBoneReference( void ) const ;
		void EnableBone( bool flagBone ) ;

	protected:
		struct	AutoBoneWeightInfo
		{
			S3DBoneSerializer *	pBone ;
			S3DVector			vBone ;
			S3DVector			vHandleDir ;
			float32_t			fpHandleLen ;
			const wchar_t *		pwszID ;
			ssize_t				iLayer ;
			float32_t			wTemp ;
		} ;
		// ボーンを列挙
		void EnumerateAllChildBones
			( SSystem::SArray<AutoBoneWeightInfo>& aBones,
				S3DSceneComposer::Composition * pComp, S3DBoneSerializer * pBone ) const ;
	public:
		// ボーンウェイトマップを自動的に生成
		void DoAutoMapBoneWeightLayers
			( S3DSceneComposer::Composition * pComp, S3DBoneSerializer * pBoneRoot ) ;
		// ボーン用ウェイトマップを削除
		void DoClearBoneWeightLayers( void ) ;
		// ボーン用ウェイトマップの行列を設定
		void DoInitBoneMatrix( S3DSceneComposer::Composition * pComp ) ;
		// ボーン用ウェイトマップを正規化
		void DoNormalizeBoneWeightMap( void ) ;
		// ボーン用ウェイトマップをGPU用に最適化
		void DoOptimizeBoneWeightMap( void ) ;
		// インスタンスをメッシュにベイク
		void DoBakeInstanceToMeshEditor( void ) ;
		void DoMakeMeshEditorPatchInstance
			( S3DMeshEditor::Patch& patch,
				const S4DMatrix& matrix, const S3DColor& color ) ;

	public:
		// コンポジションをモデルファイルに変換する
		enum	ConvertModelFlag
		{
			cvtFlagRefTexture			= 0x0001,	// テクスチャは直接参照する
			cvtFlagRefMaterial			= 0x0002,	// マテリアルは直接参照する
			cvtFlagOptimizeBoneWeights	= 0x0004,	// ボーン用ウェイトマップをGPU用に最適化する
			cvtFlagMergeMeshs			= 0x0010,	// MeshEditor の各 Patch を同じマテリアルで結合する
			cvtFlagMergeMaterials		= 0x0020,	// 全メッシュにおいて同じマテリアルを結合する
			cvtFlagMergeMeshByName		= 0x0040,	// メッシュ名末尾 @ 記号より末尾部で結合する
			cvtFlagMergeMaterialByName	= 0x0080,	// マテリアル名末尾 @ 記号より末尾部で結合する
			cvtFlagSerializeComposition	= 0x0100,	// ソースコンポジションをモデルファイルに保存する
			cvtFlagWithoutAnimation		= 0x0200,	// アニメーショントラックを出力しない
			cvtFlagWithoutBone			= 0x0400,	// ボーンを出力しない
			cvtFlagValidResourceRef		= 0x1000,	// 参照マテリアル・テクスチャが存在することを要求する
			cvtFlagAllMaterials			= 0x2000,	// 使用されてないマテリアルも出力する
			cvtFlagAllItems				= 0x00010000,	// 不可視アイテムも変換する
			cvtFlagAllSpaces			= 0x00020000,	// 不可視空間も変換する
		} ;
		struct	ConvertModelParam
		{
			uint32_t		nFlags ;
			const wchar_t *	pwszCompID ;
			S3DDMatrix		matBase ;
			S3DDVector		vBase ;

			ConvertModelParam( void )
				: nFlags( 0 ), pwszCompID( nullptr ),
					matBase( 1, 1, 1 ), vBase( 0, 0, 0 ) { }
		} ;
		static SGLError ConvertModelFromComposition
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceRoot,
				const ConvertModelParam& param ) ;

	protected:
		enum	ConvertModelItemFlag
		{
			cvtItemFlagBone				= 0x0001,
			cvtItemFlagMesh				= 0x0002,
			cvtItemFlagAnimation		= 0x0004,
		} ;
		struct	BuildBoneInfo
		{
			S3DModelBoneSpace *					pBone ;
			S3DSceneComposer::ItemSerializer *	pItem ;
		} ;
		struct	TextureInfo
		{
			SGLImageObject *	pTexture ;
			SSystem::SString	idTexture ;
		} ;
		typedef	SSystem::SPtrSortObjectArray
					<S3DSceneComposer::ItemSerializer,BuildBoneInfo>	BoneInfoSortMap ;
		typedef	SSystem::SPtrSortObjectArray
					<SGLImageObject,TextureInfo>	TextureInfoSortMap ;
		typedef	SSystem::SStrSortObjectArray
					< SSystem::SPointerArray<S3DModelData::MeshGroup> >	MeshGroupArray ;

		// マテリアル統合
		static void MergeMaterialByName( S3DModelBuffer& model ) ;

		// メッシュ統合
		static void MergeMeshAllMaterials( S3DModelBuffer& model ) ;
		static void MergeMeshEachMeshGroups
			( S3DModelBuffer& model, const MeshGroupArray& mapMeshGroupArrays ) ;
		static void MergeMeshByName( S3DModelBuffer& model ) ;

		// ボーン構築
		static uint32_t CreateBoneFromComposition
			( S3DModelBuffer& model, S3DModelPose& pose,
				BoneInfoSortMap& mapBoneInfos,
				S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & space, bool flagBoneSpace,
				const S3DDMatrix& matBase, const S3DDVector& vBase ) ;
		static void BuildBoneByInfoMap
			( S3DModelBuffer& model,
				BoneInfoSortMap& mapBoneInfos,
				S3DSceneComposer::Composition & comp, const S3DDVector& vBase ) ;
		static S3DModelBoneSpace * GetParentBoneByInfoMap
			( BoneInfoSortMap& mapBoneInfos,
				S3DSceneComposer::Composition & comp,
				S3DSceneComposer::ItemSerializer * pItem ) ;
		static BuildBoneInfo * GetParentBuildBoneByInfoMap
			( BoneInfoSortMap& mapBoneInfos,
				S3DSceneComposer::Composition & comp,
				S3DSceneComposer::ItemSerializer * pItem ) ;
		static S3DModelPose::MeshSelector *
			ConvertMeshSelectorTimeline
				( S3DSceneComposer::Composition & comp,
					S3DSceneComposer::Sequencer * pSeq ) ;
		static S3DModelPose::JointAnimation *
			ConvertBoneTimeline
				( S3DSceneComposer::Composition & comp,
					S3DModelBoneSpace & bone,
					S3DSceneComposer::Sequencer * pSeqPos,
					S3DSceneComposer::Sequencer * pSeqRot,
					S3DSceneComposer::Sequencer * pSeqZoom,
					S3DSceneComposer::Sequencer * pSeqBlend ) ;

		// 全マテリアル変換
		static SGLError ConvertAllMaterials
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				TextureInfoSortMap& mapTexInfos,
				const ConvertModelParam& param ) ;

		// 各アイテム変換
		static SGLError ConvertCompositionItems
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DSceneComposer::SpaceSerializer & space,
				const ConvertModelParam& param ) ;
		static S3DMaterial * ConvertMeshMaterial
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				TextureInfoSortMap& mapTexInfos,
				S3DMaterial * pMaterial,
				const wchar_t * pwszMaterialID,
				const ConvertModelParam& param ) ;
		static void ConvertMaterialTexture
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				TextureInfoSortMap& mapTexInfos,
				S3DMaterial * pDstMaterial,
				S3DMaterial * pSrcMaterial,
				int iTexture, bool flagBack,
				const wchar_t * pwszMaterialID,
				const ConvertModelParam& param ) ;
		static SGLError ConvertInstancingMesh
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DSceneComposer::ItemSerializer & mesh,
				S3DSceneComposer::ItemSerializer & itemOwner,
				const ConvertModelParam& param ) ;
		static SGLError ConvertInstancingMesh
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DSceneComposer::ItemSerializer & mesh,
				S3DSceneComposer::ItemSerializer & itemOwner,
				S3DItemInstancingSerializer & instancing,
				const ConvertModelParam& param ) ;
		static SGLError ConvertInstanceMeshAt
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				size_t iInstancing,
				const S4DMatrix& mat4Instance,
				S3DSceneComposer::ItemSerializer & mesh,
				S3DSceneComposer::ItemSerializer & itemOwner,
				const ConvertModelParam& param ) ;
		static SGLError ConvertMeshEditor
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DMatrix& matMeshBase,
				const S3DVector& vMeshBase,
				S3DMeshEditorSerializer & mesh,
				S3DSceneComposer::ItemSerializer & itemOwner,
				size_t iInstancing,
				const ConvertModelParam& param ) ;
		static SGLError ConvertMeshBuffer
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DMatrix& matMeshBase,
				const S3DVector& vMeshBase,
				S3DMeshBufferItemSerializer & mesh,
				size_t iInstancing,
				S3DSceneComposer::ItemSerializer * pOptMesh,
				const ConvertModelParam& param ) ;
		static SGLError ConvertIndirectMeshBuffer
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DIndirectMeshBuilderSerializer & mesh,
				const ConvertModelParam& param ) ;
		static SGLError ConvertInstancingItem
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				TextureInfoSortMap& mapTexInfos,
				MeshGroupArray& mapMeshGroupArrays,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DMultiInstanceSerializer & item,
				const ConvertModelParam& param ) ;
		static void ConvertBonePhysMaterialItem
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				S3DBonePhysMaterialSerializer& bonePhys,
				const ConvertModelParam& param ) ;
		static void ConvertMarkerItem
			( S3DModelBuffer& model,
				S3DSceneComposer::Composition & comp,
				BoneInfoSortMap& mapBoneInfos,
				const S3DDMatrix& matIBaseSpace,
				const S3DDVector& vIBaseSpace,
				S3DMarkerItemSerializer& marker,
				const ConvertModelParam& param ) ;

	public:
		// モデルファイルを編集可能なコンポジションに変換する
		struct	BuildMeshEditorItemsParam
		{
			uint32_t		nFlags ;

			BuildMeshEditorItemsParam( void ) : nFlags( 0 ) { }
		} ;
		static SGLError BuildMeshEditorItemsFromModel
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceRoot,
				S3DModelBuffer& model,
				const BuildMeshEditorItemsParam& param ) ;

	protected:
		static bool AddEditorItemChild
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceParent,
				const wchar_t * pwszID,
				S3DSceneComposer::ItemSerializer * pChild ) ;
		static void BuildEditorBonesFromModel
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceParent,
				SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
				S3DModelBuffer& model,
				S3DModelBoneSpace * pBoneParent,
				const BuildMeshEditorItemsParam& param ) ;
		static void BuildEditorBonePhysFromModel
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceParent,
				S3DModelBuffer& model,
				const BuildMeshEditorItemsParam& param ) ;
		static void BuildEditorMarkersFromModel
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceParent,
				const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
				S3DModelBuffer& model,
				const BuildMeshEditorItemsParam& param ) ;
		static void BuildEditorMeshsFromModel
			( S3DSceneComposer::Composition & comp,
				S3DSceneComposer::SpaceSerializer & spaceParent,
				const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
				S3DModelBuffer& model,
				const BuildMeshEditorItemsParam& param ) ;
		void BuildMeshVertexFromModel
			( S3DModelBuffer& model,
				const wchar_t * pwszName,
				const S3DModelBuffer::MeshObject& meshObj,
				const BuildMeshEditorItemsParam& param ) ;
		bool BuildMeshBoneRefFromModel
			( S3DModelBuffer& model,
				const S3DModelBuffer::MeshObject& meshObj,
				const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
				const BuildMeshEditorItemsParam& param ) ;
		void BuildMeshMorphFromModel
			( S3DModelBuffer& model,
				const S3DModelBuffer::MeshObject& meshObj,
				const BuildMeshEditorItemsParam& param ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	protected:	// ItemSerializer
		// アイテムのプライマリモデル取得
		virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
		// アイテムのコリジョンバッファ取得
		virtual S3DCollider * GetItemPrimaryCollider( void ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, size_t nExParamBytes ) ;
		// レンダリングの為のデバイスリソース準備
		virtual void OnPrepareToRender
			( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;

	public:	// ItemBasicSerializer
		// フレームを適用
		virtual void SetFrameParameters
				( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
		// 当たり判定追加
		virtual void OnItemRenderCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// 表示モデル追加
		virtual void OnItemRenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:	// S3DInstancingItemInterface
		// インスタンス処理（classPreRender で呼び出す）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors,
				size_t nCount, ESLObject * pSrcItem ) ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) ;

	public:	// S3DMeshEditorInterface
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const ;
		virtual S3DMeshEditor& MeshEditor( void ) ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;

	public:	// S3DParticleSerializer::RenderTarget
		// アニメーション長取得
		virtual bool GetTargetAnimationLength( double& secLength ) const ;
		// 全フレーム数取得
		virtual size_t GetTargetAnimationFrames( void ) const ;
		// ターゲット空間（逆変換用）
		virtual void GetTargetSpaceTransformation
				( S3DDMatrix& matITarget, S3DDVector& vITarget ) ;
		// パーティクル追加
		virtual void AddParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;
		// AddIndexedParticles を使うか？
		virtual bool IsUsingIndexedParticles( void ) ;
		// パーティクル追加（高機能）（classPreRender で呼び出す）
		virtual void AddIndexedParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const S3DParticleSerializer::ParticleIndex * pIndexes,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const S3DMatrix * pFaceDirs = NULL ) ;

	public:	// S3DMeshEditorBridge
		// オーナーアイテム取得
		virtual S3DSceneComposer::ItemSerializer * GetOwnerItem( void ) const ;
		// メッシュ同期
		virtual bool IsSynchronizedMeshEditor( void ) const ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// モーフィング・メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DMorphMeshController
				: public S3DMeshEditorSerializer::MeshController,
					public S3DMeshEditorBridge
	{
	public:
		enum	ParameterIndex
		{
			paramMorphMesh,
			paramBlendParam,
			paramAutoSync,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DMorphMeshController, MeshController, S3DMeshEditorBridge )
		S3D_DECLARE_COMPOSER_ITEM( S3DMorphMeshController, morph_mesh )
		// 構築関数
		S3DMorphMeshController( void ) ;

	protected:
		S3DMeshEditorObject	m_mesh ;
		double				m_fpBlendParam ;
		bool				m_flagAutoSync ;

	public:
		// 適用度
		double GetBlendParam( void ) const ;
		void SetBlendParam( double fpBlend ) ;
		// メッシュ同期
		bool IsMeshAutoSync( void ) const ;
		void SetMeshAutoSync( bool flagSync ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	public:	// S3DMeshEditorInterface
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const ;
		virtual S3DMeshEditor& MeshEditor( void ) ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;
		// 値更新通知
		virtual void NotifyUpdateVertex
				( const S3DMeshEditor::SelectPointSet& selPoints ) ;

	public:	// S3DMeshEditorBridge
		// オーナーアイテム取得
		virtual S3DSceneComposer::ItemSerializer * GetOwnerItem( void ) const ;
		// メッシュ同期
		virtual bool IsSynchronizedMeshEditor( void ) const ;
		// 値更新通知
		virtual void OnNotifyUpdateVertex
			( const S3DMeshEditor::SelectPointSet& selPointsIndexed,
				const S3DMeshEditor& meshSrc ) ;

	} ;
	


	//////////////////////////////////////////////////////////////////////////
	// 法線制御メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DNormalMeshController
				: public S3DMeshEditorSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramRefWightMap,
			paramNormal,
			paramCoverage,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DNormalMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DNormalMeshController, mesh_normal )
		// 構築関数
		S3DNormalMeshController( void ) ;

	protected:
		SSystem::SString	m_strRefWeight ;
		S3DDVector			m_vNormal ;
		double				m_fpCoverage ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// ディスプレイスメント・メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DDisplacementMeshController
				: public S3DMeshEditorSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramRefWightMap,
			paramHeight,
			paramBaseWeight,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DDisplacementMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DDisplacementMeshController, mesh_displacement )
		// 構築関数
		S3DDisplacementMeshController( void ) ;

	protected:
		SSystem::SString	m_strRefWeight ;
		double				m_fpHeight ;
		double				m_fpBaseWeight ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 鏡映反転メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DMirrorMeshController
				: public S3DMeshEditorSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramBasePosition,
			paramMirrorAxis,
			paramSeamFlag,
			paramSeamGap,
			paramMirrorWeight,
			paramWeightNameRule,
			paramWeightRightName,
			paramWeightLeftName,
			paramCount,
		} ;
		enum	MirrorAxis
		{
			mirrorX,
			mirrorY,
			mirrorZ,
		} ;
		enum	WeightNameRule
		{
			ruleInclusive,
			ruleMatchLead,
			ruleMatchEnd,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiMirrorAxis[4] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiNameRule[4] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DMirrorMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DMirrorMeshController, mesh_mirror )
		// 構築関数
		S3DMirrorMeshController( void ) ;

	protected:
		S3DDVector			m_vBasePos ;
		MirrorAxis			m_mirrorAxis ;
		bool				m_flagSeam ;
		double				m_fpSeamGap ;

		bool				m_flagMirrorWeight ;
		WeightNameRule		m_ruleWightName ;
		SSystem::SString	m_strRightName ;
		SSystem::SString	m_strLeftName ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	protected:
		// ウェイトマップ名反転
		SSystem::SString MirrorWeightName( const SSystem::SString& strName ) const ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 配列複製メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DArrayMeshController
				: public S3DMeshEditorSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramBasePosition,
			paramBaseRotate,
			paramBaseZoom,
			paramCopyCount,
			paramKeepOriginal,
			paramDuplicateOriginal,
			paramMoveDelta,
			paramRotationCenter,
			paramRotationAxis,
			paramRotationDelta,
			paramZoomDelta,
			paramSeamFlag,
			paramSeamGap,
			paramCount,
		} ;
		enum	RotationAxis
		{
			rotationOnX,
			rotationOnY,
			rotationOnZ,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiRotationAxis[4] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DArrayMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DArrayMeshController, mesh_array )
		// 構築関数
		S3DArrayMeshController( void ) ;

	protected:
		S3DDVector		m_vBasePos ;
		S3DDMatrix		m_matBaseRotate ;
		S3DDVector		m_vBaseZoom ;
		int32_t			m_nCopyCount ;
		bool			m_flagKeepOrg ;
		bool			m_flagDuplicateOrg ;
		S3DDVector		m_vMoveDelta ;
		S3DDVector		m_vRotationCenter ;
		RotationAxis	m_rotationAxis ;
		double			m_degRotationAngle ;
		S3DDVector		m_vZoomDelta ;
		bool			m_flagSeam ;
		double			m_fpSeamGap ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	protected:
		// 縮退情報
		struct	PointEntry
		{
			size_t	iPatch ;
			size_t	iVertex ;

			bool operator == ( const PointEntry& pe ) const
			{
				return	(iPatch == pe.iPatch) && (iVertex == pe.iVertex) ;
			}
			bool operator != ( const PointEntry& pe ) const
			{
				return	(iPatch != pe.iPatch) || (iVertex != pe.iVertex) ;
			}
			bool operator > ( const PointEntry& pe ) const
			{
				return	(iPatch > pe.iPatch)
					|| ((iPatch == pe.iPatch) && (iVertex > pe.iVertex)) ;
			}
			bool operator < ( const PointEntry& pe ) const
			{
				return	(iPatch < pe.iPatch)
					|| ((iPatch == pe.iPatch) && (iVertex < pe.iVertex)) ;
			}
		} ;
		class	SeamEntry	: public SSystem::SArray<PointEntry>
		{
		public:
			PointEntry	m_peSrc ;
		} ;
		typedef	SSystem::SSortObjectArray
				< SSystem::SSortObjectElement<PointEntry,SeamEntry> >	SeamEntryMap ;

		// 縮退情報構築
		void MakeSeamEntryMap
			( SeamEntryMap& sem, const S3DMeshEditor& mesh,
				const S3DMatrix& matCopy0, const S3DVector& vCopy0,
				const S3DMatrix& matCopy1, const S3DVector& vCopy1 ) const ;
		// 縮退設定
		void AddBuildBySeamEntries
			( S3DMeshEditor& mesh,
				size_t iLastBasePatch, size_t iCurBasePatch,
				const SeamEntryMap& sem, bool flagSeamTest ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ブーリアン・モデリング
	//////////////////////////////////////////////////////////////////////////

	class	S3DBooleanMeshEditor
	{
	public:
		// メッシュ
		class	MeshWorkBuffer ;

		// 稜線 -> 面情報
		struct	FacePair
		{
			ssize_t	iFace[2] ;
		} ;
		typedef SSystem::SSortArray
			< SSystem::SSortElement<S3DMeshEditor::Edge,FacePair> >	EdgeFaceMap ;

		// 面と稜線交差情報
		struct	MeshCrossEdge
		{
			MeshCrossEdge *		pNextEdge ;	// 次の交点
			ssize_t				iFaceEdge ;	// 交差点が内側か辺か (-1:内側, 0:AB, 1:BC, 2:CA)
			MeshWorkBuffer *	pMesh ;		// 交差対象のメッシュ
			ssize_t				iCrossFace ;// 交差対象の面指標 (iFaceEdge >= 0 の時)
			S3DMeshEditor::Edge	edge ;		// 交差対象の辺 (iFaceEdge == -1 の時)
			const FacePair *	pFaces ;	// edge を含む面
			S3DVector			vCross ;	// △ABC 内の交差点
			S2DVector			vCoord ;	// 辺AB, AC を基底とする座標
		} ;
		struct	MeshCrossEdgeList
		{
			MeshCrossEdgeList *	pNextList ;		// 次のリスト
			MeshCrossEdge *		pFirst ;		// 一連の交差点のリスト
			MeshCrossEdge *		pLast ;

			bool IsEmpty( void ) const ;
			void AddFirst( MeshCrossEdge * pmce ) ;
			void AddLast( MeshCrossEdge * pmce ) ;
			void Reverse( void ) ;
			void AddLastFrom( MeshCrossEdgeList * pmcel ) ;
			void AddLastReverseFrom( MeshCrossEdgeList * pmcelRev ) ;
			void AddFirstReverseFrom( MeshCrossEdgeList * pmcelRev ) ;
			void VerifyList( void ) ;
		} ;
		typedef	SSystem::SArray<MeshCrossEdgeList>	FaceCrossEdgeTable ;

		// 頂点が対象の内側か外側か
		enum	VertexBoolean
		{
			elementInner	= -1,
			elementUnknown,
			elementOuter,
		} ;
		typedef	SSystem::SArray<int8_t>	VertexBooleanTable ;

		// メッシュ
		class	MeshWorkBuffer	: public ESLObject
		{
		public:
			S3DRenderBuffer::MeshBuffer	m_mesh ;
			EdgeFaceMap					m_efm ;
			FaceCrossEdgeTable			m_fcet ;
			VertexBooleanTable			m_vbt ;
		public:
			ESL_DECLARE_CLASS_INFO( MeshWorkBuffer, ESLObject )
			MeshWorkBuffer( void ) ;
			~MeshWorkBuffer( void ) ;
		} ;

		// オブジェクト（ブーリアン対象全体）
		class	BooleanObject
		{
		public:
			SSystem::SObjectArray<MeshWorkBuffer>	m_buffers ;
			S3DCollision							m_collision ;
		} ;

	protected:
		SSystem::SStackBuffer		m_bufWork ;
		SSystem::SArray<uint32_t>	m_bufIndex ;
		SSystem::SArray<uint32_t>	m_bufPolyIndex ;
		SSystem::SArray<S3DVector4>	m_bufTrianglizeVertex ;
		SSystem::SArray<uint32_t>	m_bufTrianglizeIndex ;
		SSystem::SArray<uint32_t>	m_bufTrianglizeWork ;
		SSystem::SArray<S3DVector4>	m_bufGougeVertex ;
		SSystem::SArray<uint32_t>	m_bufGougeTrianglizeIndex ;
		SSystem::SArray<float32_t>	m_bufExAttrTemp ;

	public:
		// ブーリアン開始
		void ResetBoolean( void ) ;

	public:
		// オブジェクト構築
		static void BuildObject
			( BooleanObject& bobj, S3DMeshEditor& mesh,
				const S3DMeshEditor::MeshParam& param, uint32_t nFlags,
				S3DMaterial*const* ppMaterials, size_t nCount ) ;
	protected:
		static void BuildFaceCrossEdgeTable( BooleanObject& bobj ) ;

	public:
		// 交差処理
		void CrossTest( BooleanObject& bobj0, BooleanObject& bobj1 ) ;
	protected:
		void CrossTestAgainst
			( BooleanObject& bobjBoolean, BooleanObject& bobjTest,
				const S3DVector& vMin, const S3DVector& vMax ) ;
		void CrossTestEdgeAgainst
			( BooleanObject& bobjBoolean, MeshWorkBuffer * pmwb,
				size_t iVertex0, size_t iVertex1, const FacePair * pfp,
				const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap ) ;
		// 当たり判定関数
		struct	CrossEdgeDesc
		{
			double				fpHit ;
			MeshWorkBuffer *	pmwbCross ;
			size_t				iCrossFace ;
			S3DVector			vHitGlobal ;
			S3DVector			vHitNormal ;
			S2DVector			vHitCoord ;
		} ;
		class	CrossTestEdgeAgainst_Param
		{
		public:
			S3DBooleanMeshEditor::MeshWorkBuffer *	m_pmwb ;
			S3DDVector								m_vPos0 ;
			S3DDVector								m_vPos1 ;
			S3DDVector								m_vDir ;
			SSystem::SObjectArray<CrossEdgeDesc>	m_aHitResult ;

			CrossTestEdgeAgainst_Param
				( S3DBooleanMeshEditor& bme,
					BooleanObject& bobjBoolean, MeshWorkBuffer * pmwb,
					size_t iVertex0, size_t iVertex1, const FacePair * pfp,
					const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap ) ;
		} ;
		static S3DCollision::HitColliderCallback
			CrossTestEdgeAgainst_OnHitCollider
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
		// 頂点の内外判定
		VertexBoolean TestVertexBooleanAgainst
			( BooleanObject& bobjBoolean, const S3DDVector& vVertex,
				const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap ) ;
		// 面の稜線番号を取得
		ssize_t FaceEdgeIndexOf
			( MeshWorkBuffer * pmwb,
				size_t iFace, const S3DMeshEditor::Edge& edge ) ;
		// 面の稜線の交差点を追加（稜線側の面）
		void AddFaceEdgeCrossPoint
			( MeshWorkBuffer * pmwb, size_t iFace,
				size_t iFaceEdge, const S3DVector& vCross,
				MeshWorkBuffer * pmwbCross, size_t iCrossFace ) ;
		// 面に交差する稜線を追加（稜線に貫かれる側の面）
		void AddCrossEdgePointAtFace
			( MeshWorkBuffer * pmwb, size_t iFace,
				const S3DVector& vCross, const S2DVector& vCoord,
				MeshWorkBuffer * pmwbCross,
				const S3DMeshEditor::Edge& edgeCross, const FacePair * pfpEdgeFaces ) ;

	public:
		// 交差処理されたメッシュを出力する
		void BuildBooleanMesh
			( BooleanObject& bobj, VertexBoolean vboolLogic, bool faceBack ) ;
	protected:
		void BuildBooleanMeshBuffer
			( MeshWorkBuffer& mwbuf,
				VertexBoolean vboolLogic, bool faceBack, float32_t fpErrorGap ) ;
		// MeshCrossEdgeList を最終的に結合する
		void MergeMeshCrossEdgeList( MeshCrossEdgeList * pmcel, float32_t fpErrorGap ) ;
		MeshCrossEdge * FindNearCrossPoint
			( MeshCrossEdgeList*& pmcelLast, const S3DVector& vPos, float32_t fpErrorGap ) ;
		// 交差頂点数計算
		size_t CountCrossPointOfEdgeList( MeshCrossEdgeList * pmcel ) ;
		// 交差面を分割して出力
		void BuildMeshAtCrossingFace
			( MeshWorkBuffer& mwbuf,
				MeshCrossEdgeList * pmcel, const uint32_t * pFaceIndex,
				VertexBoolean vboolLogic, bool faceBack ) ;
		// 指定辺の交差点で、指定頂点に最近の交差点を検索する
		MeshCrossEdge * FindNearEdgeCrossPoint
			( MeshCrossEdgeList*& pmcelCur,
				MeshCrossEdgeList*& pmcelLast,
				ssize_t iFaceEdge,
				const S3DVector& vPos, const S3DVector& vNextPos ) ;
		// くり抜き交差点リストを検索し、リストから分離する
		MeshCrossEdgeList * GetGougeOutEdgeList
			( MeshCrossEdgeList * pmcel, const S3DVector4 * pvVertex,
				const uint32_t * pPolyIndex,
				const uint32_t * pTriangleIndex, size_t nTriangleIndexCount ) ;
		static bool IsPointInTriangle
			( const S3DVector& v0, const S3DVector& v1,
				const S3DVector& v2, const S3DVector& vPos ) ;
		// くり抜き処理
		void AppendGougeOutEdgePoints
			( S3DRenderBuffer::MeshBuffer& mesh,
				SSystem::SArray<uint32_t>& bufIndex,
				const uint32_t * pTriangleIndex,
				size_t nTriangleIndexCount,
				const uint32_t * pFaceIndex,
				MeshCrossEdgeList * pmcelGouge ) ;
		// 交差点を頂点として追加する
		void AppendCrossPoints
			( S3DRenderBuffer::MeshBuffer& mesh,
				SSystem::SArray<uint32_t>& bufIndex,
				const uint32_t * pFaceIndex,
				MeshCrossEdge * pmce, MeshCrossEdge * pmceEnd = NULL ) ;

	public:
		// S3DMeshEditor へ変換する
		void ConvertToMeshEditor
			( S3DMeshEditor& mesh,
				const BooleanObject& bobj,
				const wchar_t * pwszBaseName,
				uint32_t nPatchFlags, ssize_t iMaterial = -1 ) ;
		// S3DMeshEditor::Patch へ変換する
		void ConvertToMeshEditorPatch
			( S3DMeshEditor::Patch& patch, const MeshWorkBuffer& mwb ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// ブーリアン・メッシュコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DBooleanMeshController
				: public S3DMeshEditorSerializer::MeshController,
					public S3DMeshEditorInterface
	{
	public:
		enum	ParameterIndex
		{
			paramBooleanMesh,
			paramBooleanType,
			paramEachOperation,
			paramReplaceMaterial,
			paramCount,
		} ;
		enum	BooleanType
		{
			// A: Target Mesh,  B: Boolean Mesh
			booleanNothing,
			booleanCut,			// A & ~B
			booleanCutOut,		// A & B
			booleanAnd,			// (A & B) + (B & A)
			booleanGougeOut,	// (A & ~B) + (B & A)
			booleanOr,			// (A & ~B) + (B & ~A)
			booleanMaterial,	// (A & ~B) + (A & B)
			booleanTypeCount,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiBooleanType[booleanTypeCount+1] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DBooleanMeshController, MeshController, S3DMeshEditorInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DBooleanMeshController, boolean_mesh )
		// 構築関数
		S3DBooleanMeshController( void ) ;

	protected:
		S3DMeshEditorObject		m_mesh ;
		BooleanType				m_type ;
		bool					m_flagEachOp ;
		int32_t					m_iMaterial ;

		S3DBooleanMeshEditor	m_boolean ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	public:	// S3DMeshEditorInterface
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const ;
		virtual S3DMeshEditor& MeshEditor( void ) ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易メッシュ・リダクション・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DSimpleMeshReductionController
					: public S3DMeshEditorSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramReducing,
			paramRepetition,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DSimpleMeshReductionController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DSimpleMeshReductionController, simple_mesh_reductor )
		// 構築関数
		S3DSimpleMeshReductionController( void ) ;

	protected:
		double	m_fpReducing ;
		int32_t	m_nRepetition ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// メッシュデーター上書き・コントローラー（ベイク済みデータ保持用）
	//////////////////////////////////////////////////////////////////////////

	class	S3DBakedMeshController
				: public S3DMeshEditorSerializer::MeshController,
					public S3DMeshEditorInterface
	{
	public:
		enum	ParameterIndex
		{
			paramBakedMesh,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DBakedMeshController, MeshController, S3DMeshEditorInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DBakedMeshController, baked_mesh )
		// 構築関数
		S3DBakedMeshController( void ) ;

	protected:
		S3DMeshEditorObject		m_mesh ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;

	public:	// MeshController
		// メッシュの変形
		virtual S3DMeshEditor * ModifyMeshEditor
			( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef ) ;

	public:	// S3DMeshEditorInterface
		// S3DMeshEditor 取得
		virtual const S3DMeshEditor& GetMeshEditor( void ) const ;
		virtual S3DMeshEditor& MeshEditor( void ) ;
		// 表示用メッシュの更新
		virtual void UpdateViewMesh( void ) ;
		// 表示用マテリアルの取得
		virtual S3DMaterial * GetMeshMaterial( size_t iMaterial = 0 ) const ;
		// 空間
		virtual void GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const ;
	} ;

}

#endif
