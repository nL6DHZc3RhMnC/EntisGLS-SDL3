
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_list.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////
// リスト表示抽象エントリ
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteBasicList::Entry, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLSpriteBasicList::Entry::Entry( void )
	: m_procAsync( SProcedureQueue::NullProcId ),
		m_viewArea( viewOutside ),
		m_flagDetached( false ),
		m_flagFocus( false ),
		m_flagSelected( false ),
		m_flagPressed( false ),
		m_flagDisabled( false )
{
}

// 分離前処理
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::Entry::OnDetachEntry( SGLSpriteBasicList& listView )
{
	m_flagDetached = true ;
	if ( m_procAsync != SProcedureQueue::NullProcId )
	{
		listView.CancelToPrepareEntry( m_procAsync ) ;
	}
}

// フォーカス状態
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::Entry::HasEntryFocus( void ) const
{
	return	m_flagFocus ;
}

void SGLSpriteBasicList::Entry::SetEntryFocus( bool flagFocus )
{
	m_flagFocus = flagFocus ;
}

// 選択状態
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::Entry::IsEntrySelected( void ) const
{
	return	m_flagSelected ;
}

void SGLSpriteBasicList::Entry::SelectEntry( bool flagSelect )
{
	m_flagSelected = flagSelect ;
}

// 禁止状態
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::Entry::IsEntryDisabled( void ) const
{
	return	m_flagDisabled ;
}

void SGLSpriteBasicList::Entry::DisableEntry( bool flagDisable )
{
	m_flagDisabled = flagDisable ;
}

// クリック時
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::Entry::OnPressEntry( SGLSpriteBasicList& listView )
{
	m_flagPressed = true ;
}

void SGLSpriteBasicList::Entry::OnReleasePressEntry( SGLSpriteBasicList& listView )
{
	m_flagPressed = false ;
}

bool SGLSpriteBasicList::Entry::IsPressingEntry( void ) const
{
	return	m_flagPressed ;
}

void SGLSpriteBasicList::Entry::OnClickEntry( SGLSpriteBasicList& listView )
{
}

void SGLSpriteBasicList::Entry::OnDoubleClickEntry( SGLSpriteBasicList& listView )
{
}

// 現在の表示エリア
//////////////////////////////////////////////////////////////////////////
SGLSpriteBasicList::ViewArea
	SGLSpriteBasicList::Entry::CurrentViewArea( void ) const
{
	return	m_viewArea ;
}

// 表示エリア変更
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::Entry::ChangeViewArea
	( SGLSpriteBasicList& listView, ViewArea area )
{
	if ( m_viewArea != area )
	{
		if ( m_viewArea == viewOutside )
		{
			if ( m_procAsync != SProcedureQueue::NullProcId )
			{
				listView.CancelToPrepareEntry( m_procAsync ) ;
			}
			m_procAsync = listView.AsyncPrepareToDrawEntry( this ) ;
		}
		m_viewArea = area ;
	}
}

// 描画準備
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::Entry::OnPrepareToDraw(const SGLSpriteBasicList& listView )
{
	m_procAsync = SProcedureQueue::NullProcId ;
}



//////////////////////////////////////////////////////////////////////////
// OnPrepareToDraw 呼び出し Procedure
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteBasicList::PrepareToDrawProc, SProcedure )

// スレッド関数
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::PrepareToDrawProc::Run( void )
{
	m_entry.OnPrepareToDraw( m_list ) ;
}



//////////////////////////////////////////////////////////////////////////
// リスト表示基底スプライト
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLSpriteBasicList, SGLSprite, SGLSpriteScrollListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLSpriteBasicList::SGLSpriteBasicList( void )
	: m_nBehaviorFlags( 0 ),
		m_nViewProximity( 500 ),
		m_nBatchEdit( 0 ),
		m_flagSwiping( false ),
		m_msLastClicked( 0 ), m_pLastClicked( nullptr ),
		m_iKeyFocus( -1 ), m_iMouseFocus( -1 ), m_pPressingEntry( nullptr )
{
}

SGLSpriteBasicList::SGLSpriteBasicList( const SGLSpriteBasicList& src )
	: m_nBehaviorFlags( src.m_nBehaviorFlags ),
		m_nViewProximity( src.m_nViewProximity ),
		m_nBatchEdit( 0 ),
		m_flagSwiping( false ),
		m_msLastClicked( 0 ), m_pLastClicked( nullptr ),
		m_iKeyFocus( -1 ), m_iMouseFocus( -1 ), m_pPressingEntry( nullptr )
{
	if ( !src.m_sizeView.IsEmpty() )
	{
		CreateView
			( (uint32_t) src.m_sizeView.w,
				(uint32_t) src.m_sizeView.h, src.IsBuffered() ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLSpriteBasicList::~SGLSpriteBasicList( void )
{
	RemoveAllListEntries() ;
}

// 作成
//////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteBasicList::CreateView
	( uint32_t width, uint32_t height, bool flagBuffered )
{
	ReleaseView() ;
	//
	if ( flagBuffered )
	{
		SGLError	err = SGLSprite::CreateBuffer( width, height ) ;
		if ( err )
		{
			SGLSprite::ReleaseBuffer() ;
			return	err ;
		}
	}
	m_sizeView.w = (int32_t) width ;
	m_sizeView.h = (int32_t) height ;
	//
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::ReleaseView( void )
{
	if ( !m_sizeView.IsEmpty() )
	{
		if ( SGLSprite::IsBuffered() )
		{
			SGLSprite::ReleaseBuffer() ;
		}
		m_sizeView.w = 0 ;
		m_sizeView.h = 0 ;
	}
}

//ビューサイズ
//////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteBasicList::ResizeView( uint32_t width, uint32_t height )
{
	SGLError	err = sglErrSuccess ;
	if ( (m_sizeView.w != (int32_t) width)
		|| (m_sizeView.h = (int32_t) height) )
	{
		if ( SGLSprite::IsBuffered() )
		{
			SGLSprite::CreateBuffer(width, height ) ;
		}
		m_sizeView.w = (int32_t) width ;
		m_sizeView.h = (int32_t) height ;
	}
	return	err ;
}

const SGLSize& SGLSpriteBasicList::GetViewSize( void ) const
{
	return	m_sizeView ;
}

// 動作フラグ
//////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteBasicList::GetListBehaviorFlags( void ) const
{
	return	m_nBehaviorFlags ;
}

void SGLSpriteBasicList::SetListBehaviorFlags( uint32_t nFlags )
{
	m_nBehaviorFlags = nFlags ;
}

// 表示近傍距離
//////////////////////////////////////////////////////////////////////////
int SGLSpriteBasicList::GetViewProximity( void ) const
{
	return	m_nViewProximity ;
}

void SGLSpriteBasicList::SetViewProximity( int nProximity )
{
	m_nViewProximity = nProximity ;
}

// 現在のスクロール位置をスクロールバーへ反映
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::ReflectToScrollBar( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pScrollBar = m_refScrollBar.GetReference() ;
	if ( pScrollBar != nullptr )
	{
		pScrollBar->SetScrollRange( (int) GetTotalListHeight() - m_sizeView.h ) ;
		pScrollBar->SetScrollPos( m_ptScroll.y ) ;
	}
	SGLBasicForm::TrackBar * pTrackBar = m_refTrackBar.GetReference() ;
	if ( pTrackBar != nullptr )
	{
		pTrackBar->SetBarRange( (int) GetTotalListHeight() - m_sizeView.h ) ;
		pTrackBar->SetBarPos( m_ptScroll.y ) ;
	}
	Unlock() ;
}

// スクロール位置変更に伴う表示エリア情報の更新通知
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::NorifyListChangeViewArea( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	iViewFirst = IndexFromPointY( 0 ) ;
	size_t	iViewLast = IndexFromPointY( m_sizeView.h ) ;
	size_t	iViewNearFirst = IndexFromPointY( -m_nViewProximity ) ;
	size_t	iViewNearLast = IndexFromPointY( m_sizeView.h + m_nViewProximity ) ;
	for ( size_t i = 0; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( pEntry != nullptr )
		{
			ViewArea	area = viewOutside ;
			if ( (i >= iViewFirst) && (i <= iViewLast) )
			{
				area = viewInside ;
			}
			else if ( (i >= iViewNearFirst) && (i <= iViewNearLast) )
			{
				area = viewNearInside ;
			}
			if ( pEntry->CurrentViewArea() != area )
			{
				pEntry->ChangeViewArea( *this, area ) ;
			}
		}
	}
	Unlock() ;
}

// リストエントリ数
//////////////////////////////////////////////////////////////////////////
size_t SGLSpriteBasicList::GetListEntryCount( void ) const
{
	return	m_list.GetLength() ;
}

// リストエントリ取得
//////////////////////////////////////////////////////////////////////////
SGLSpriteBasicList::Entry * SGLSpriteBasicList::GetListEntryAt( size_t i ) const
{
	Entry *	pEntry = nullptr ;
	LockTrace( __FILE__, __LINE__ ) ;
	pEntry = m_list.GetAt( i ) ;
	Unlock() ;
	return	pEntry ;
}

// リストエントリ追加
//////////////////////////////////////////////////////////////////////////
size_t SGLSpriteBasicList::AddListEntry( SGLSpriteBasicList::Entry * pEntry )
{
	size_t	i = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	if ( m_nBatchEdit == 0 )
	{
		GetViewListRange( range ) ;
	}
	i = m_list.Add( pEntry ) ;
	//
	if ( m_nBatchEdit == 0 )
	{
		UpdateListHeight( i ) ;
		if ( IsChangedViewListRange( range ) )
		{
			NorifyListChangeViewArea() ;
		}
		PostUpdate() ;
	}
	Unlock() ;
	return	i ;
}

void SGLSpriteBasicList::InsertListEntryAt( size_t i, SGLSpriteBasicList::Entry * pEntry )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	if ( m_nBatchEdit == 0 )
	{
		GetViewListRange( range ) ;
	}
	m_list.InsertAt( i, pEntry ) ;
	//
	if ( m_nBatchEdit == 0 )
	{
		UpdateListHeight( i ) ;
		if ( IsChangedViewListRange( range ) )
		{
			NorifyListChangeViewArea() ;
		}
		PostUpdate() ;
	}
	Unlock() ;
}

// リストエントリ削除
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::RemoveAllListEntries( void )
{
	bool	flagRemoveAny = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	for ( size_t i = 0; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( pEntry != nullptr )
		{
			pEntry->OnDetachEntry( *this ) ;
			m_delayRemove.Add( pEntry ) ;
			flagRemoveAny = true ;
		}
	}
	m_list.DetachAll() ;
	//
	UpdateListHeight() ;
	PostUpdate() ;
	Unlock() ;
	//
	if ( flagRemoveAny && (m_nBehaviorFlags & behaviorAsyncPrepare) )
	{
		WaitForAsyncPrepare() ;
	}
}

void SGLSpriteBasicList::RemoveListEntryAt( size_t i )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	if ( m_nBatchEdit == 0 )
	{
		GetViewListRange( range ) ;
	}
	Entry *	pEntry = m_list.DetachAt( i ) ;
	if ( pEntry != nullptr )
	{
		pEntry->OnDetachEntry( *this ) ;
		m_delayRemove.Add( pEntry ) ;
	}
	if ( m_nBatchEdit == 0 )
	{
		UpdateListHeight( i ) ;
		if ( IsChangedViewListRange( range ) )
		{
			NorifyListChangeViewArea() ;
		}
		PostUpdate() ;
	}
	Unlock() ;
	//
	if ( (pEntry != nullptr) && (m_nBehaviorFlags & behaviorAsyncPrepare) )
	{
		WaitForAsyncPrepare() ;
	}
}

SGLSpriteBasicList::Entry * SGLSpriteBasicList::DetachListEntryAt( size_t i )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	if ( m_nBatchEdit == 0 )
	{
		GetViewListRange( range ) ;
	}
	Entry *	pEntry = m_list.DetachAt( i ) ;
	if ( pEntry != nullptr )
	{
		pEntry->OnDetachEntry( *this ) ;
	}
	if ( m_nBatchEdit == 0 )
	{
		UpdateListHeight( i ) ;
		if ( IsChangedViewListRange( range ) )
		{
			NorifyListChangeViewArea() ;
		}
		PostUpdate() ;
	}
	Unlock() ;
	//
	if ( (pEntry != nullptr) && (m_nBehaviorFlags & behaviorAsyncPrepare) )
	{
		WaitForAsyncPrepare() ;
	}
	return	pEntry ;
}

// リストエントリ検索
//////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteBasicList::FindListEntry( SGLSpriteBasicList::Entry * pEntry ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	ssize_t	iFound = m_list.FindPtr( pEntry ) ;
	Unlock() ;
	return	iFound ;
}

// バッチ編集
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::BeginBatchEdit( void )
{
	ESLVerify( AtomicAdd( &m_nBatchEdit, 1 ) >= 1 ) ;
}

void SGLSpriteBasicList::EndBatchEdit( void )
{
	if ( AtomicSub( &m_nBatchEdit, 1 ) == 0 )
	{
		UpdateListHeight() ;
		NorifyListChangeViewArea() ;
		PostUpdate() ;
	}
}

// リストエントリ描画準備 (OnPrepareToDraw 呼び出し)
//////////////////////////////////////////////////////////////////////////
SProcedureQueue::ProcIdentity
	SGLSpriteBasicList::AsyncPrepareToDrawEntry
			( SGLSpriteBasicList::Entry * pEntry )
{
	ESLAssert( pEntry != nullptr ) ;
	if ( pEntry == nullptr )
	{
		return	SProcedureQueue::NullProcId ;
	}
	if ( m_nBehaviorFlags & behaviorAsyncPrepare )
	{
		SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
		if ( pWindow != nullptr )
		{
			return	pWindow->PostAsyncProcedure
				( new PrepareToDrawProc( *this, *pEntry ), nullptr, true, false ) ;
		}
	}
	pEntry->OnPrepareToDraw( *this ) ;
	return	SProcedureQueue::NullProcId ;
}

void SGLSpriteBasicList::CancelToPrepareEntry( SProcedureQueue::ProcIdentity procId )
{
	if ( procId != SProcedureQueue::NullProcId )
	{
		SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
		if ( pWindow != nullptr )
		{
			pWindow->CancelAsyncProcedure( procId ) ;
		}
	}
}

void SGLSpriteBasicList::WaitForAsyncPrepare( void )
{
	SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
	if ( pWindow != nullptr )
	{
		atomic_int_t	nRelock = pWindow->UnlockAll() ;
		pWindow->WaitForAllAsyncProcedure() ;
		pWindow->Relock( nRelock ) ;
		//
		LockTrace( __FILE__, __LINE__ ) ;
		m_delayRemove.RemoveAll() ;
		Unlock() ;
	}
}

// フォーカス設定（true の場合、それ以外のエントリは false に設定）
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::SetKeyFocusEntryAt( size_t iEntry, bool flagFocus )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( flagFocus )
	{
		SetFocusEntryAt( iEntry, true ) ;
		m_iKeyFocus = (ssize_t) iEntry ;
	}
	else
	{
		if ( m_iMouseFocus >= 0 )
		{
			SetFocusEntryAt( (size_t) m_iMouseFocus, true ) ;
		}
		else
		{
			SetFocusEntryAt( (size_t) m_iKeyFocus, false ) ;
		}
		m_iKeyFocus = -1 ;
	}
	Unlock() ;
}

void SGLSpriteBasicList::SetFocusEntryAt( size_t iEntry, bool flagFocus )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( flagFocus )
	{
		bool	flagChanged = false ;
		for ( size_t i = 0; i < m_list.GetLength(); i ++ )
		{
			Entry *	pEntry = m_list.GetAt( i ) ;
			bool	flagEntryFocus = (i == iEntry) ;
			if ( (pEntry != nullptr)
				&& (pEntry->HasEntryFocus() != flagEntryFocus) )
			{
				pEntry->SetEntryFocus( flagEntryFocus ) ;
				flagChanged = true ;
			}
		}
		if ( flagChanged )
		{
			PostUpdate() ;
		}
	}
	else
	{
		Entry *	pEntry = m_list.GetAt( iEntry ) ;
		if ( (pEntry != nullptr)
			&& pEntry->HasEntryFocus() )
		{
			pEntry->SetEntryFocus( false ) ;
			PostUpdate() ;
		}
	}
	Unlock() ;
}

// フォーカス取得
//////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteBasicList::FindFocusEntry( void ) const
{
	ssize_t	iFocus = -1 ;
	LockTrace( __FILE__, __LINE__ ) ;
	for ( size_t i = 0; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( (pEntry != nullptr)
			&& pEntry->HasEntryFocus() )
		{
			iFocus = (ssize_t) i ;
			break ;
		}
	}
	Unlock() ;
	return	iFocus ;
}

// アイテムの選択状態設定
//（flagFocus&&flagRadio が true の場合、それ以外のエントリは false に設定）
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::SelectEntryAt( size_t iEntry, bool flagFocus, bool flagRadio )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( flagFocus && flagRadio )
	{
		bool	flagChanged = false ;
		for ( size_t i = 0; i < m_list.GetLength(); i ++ )
		{
			Entry *	pEntry = m_list.GetAt( i ) ;
			bool	flagEntrySel = (i == iEntry) ;
			if ( (pEntry != nullptr)
				&& (pEntry->IsEntrySelected() != flagEntrySel) )
			{
				pEntry->SelectEntry( flagEntrySel ) ;
				flagChanged = true ;
			}
		}
		if ( flagChanged )
		{
			PostUpdate() ;
		}
	}
	else
	{
		Entry *	pEntry = m_list.GetAt( iEntry ) ;
		if ( (pEntry != nullptr)
			&& pEntry->IsEntrySelected() )
		{
			pEntry->SetEntryFocus( false ) ;
			PostUpdate() ;
		}
	}
	Unlock() ;
}

// アイテムが選択状態か？
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::IsSelectedEntryAt( size_t iEntry ) const
{
	bool	flagSelected = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	Entry *	pEntry = m_list.GetAt( iEntry ) ;
	if ( pEntry != nullptr )
	{
		flagSelected = pEntry->IsEntrySelected() ;
	}
	Unlock() ;
	return	flagSelected ;
}

// 禁止状態を設定
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::DisableEntryAt( size_t iEntry, bool flagDisable )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Entry *	pEntry = m_list.GetAt( iEntry ) ;
	if ( (pEntry != nullptr)
		&& (pEntry->IsEntryDisabled() != flagDisable) )
	{
		pEntry->DisableEntry( flagDisable ) ;
		PostUpdate() ;
	}
	Unlock() ;
}

// アイテムが禁止状態か？
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::IsDisabledEntryAt( size_t iEntry ) const
{
	bool	flagDisabled = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	Entry *	pEntry = m_list.GetAt( iEntry ) ;
	if ( pEntry != nullptr )
	{
		flagDisabled = pEntry->IsEntryDisabled() ;
	}
	Unlock() ;
	return	flagDisabled ;
}

// 全表示高取得
//////////////////////////////////////////////////////////////////////////
size_t SGLSpriteBasicList::GetTotalListHeight( void ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	yHeight = 0 ;
	if ( m_yList.GetLength() > 0 )
	{
		yHeight = m_yList.LastAt(0) ;
	}
	Unlock() ;
	return	yHeight ;
}

// 表示幅最大取得
//////////////////////////////////////////////////////////////////////////
size_t SGLSpriteBasicList::GetMaxWidthOfList( void ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	xMaxWidth = 0 ;
	for ( size_t i = 0; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( pEntry != nullptr )
		{
			size_t	xWidth = pEntry->GetListViewWidth() ;
			if ( xMaxWidth < xWidth )
			{
				xMaxWidth = xWidth ;
			}
		}
	}
	Unlock() ;
	return	xMaxWidth ;
}

// 表示リスト範囲取得
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::GetViewListRange
		( SGLSpriteBasicList::ViewListRange& range ) const
{
	range.iFirst = IndexFromPointY( 0 ) ;
	range.iLast = IndexFromPointY( m_sizeView.h ) ;
}

// 表示リスト範囲が変わったか？
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::IsChangedViewListRange
	( const SGLSpriteBasicList::ViewListRange& range ) const
{
	return	(range.iFirst != IndexFromPointY( 0 ))
			|| (range.iLast != IndexFromPointY( m_sizeView.h )) ;
}

// ローカル座標→エントリ指標
//////////////////////////////////////////////////////////////////////////
size_t SGLSpriteBasicList::IndexFromPointY( int yView ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_yList.GetLength() == 0)
		|| (yView + m_ptScroll.y <= 0) )
	{
		Unlock() ;
		return	0 ;
	}
	size_t	yPos = (size_t) (yView + m_ptScroll.y) ;
	size_t	iFirst = 0 ;
	size_t	iLast = m_yList.GetLength() - 1 ;
	while ( iFirst < iLast )
	{
		size_t	iMiddle = (iFirst + iLast) >> 1 ;
		size_t	yListBottom = m_yList.At( iMiddle ) ;
		if ( yPos < yListBottom )
		{
			iLast = iMiddle ;
		}
		else
		{
			iFirst = iMiddle + 1 ;
			ESLAssert( iFirst <= iLast ) ;
		}
	}
	Unlock() ;
	return	iFirst ;
}

ssize_t SGLSpriteBasicList::TestHitListEntry( int yView ) const
{
	size_t		iEntry = IndexFromPointY( yView ) ;
	SGLPoint	ptEntry0 = PointFromIndex( iEntry ) ;
	SGLPoint	ptEntry1 = PointFromIndex( iEntry + 1 ) ;
	if ( (yView < ptEntry0.y) || (ptEntry1.y <= yView) )
	{
		return	-1 ;
	}
	return	(ssize_t) iEntry ;
}

// エントリ指標→ローカル座標
//////////////////////////////////////////////////////////////////////////
SGLPoint SGLSpriteBasicList::PointFromIndex( size_t i ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLPoint	ptList = - m_ptScroll ;
	if ( (i > 0) && ((i - 1) < m_yList.GetLength()) )
	{
		ptList.y += (int32_t) m_yList.At( i - 1 ) ;
	}
	Unlock() ;
	return	ptList ;
}

// リスト毎の下辺ｙ座標更新
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::UpdateListHeight( size_t iUpdateFrom )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( iUpdateFrom < m_yList.GetLength() )
	{
		m_yList.SetLength( iUpdateFrom ) ;
	}
	size_t	yLine = 0 ;
	if ( (iUpdateFrom > 0)
		&& (m_yList.GetLength() > iUpdateFrom - 1) )
	{
		yLine = m_yList.At( iUpdateFrom - 1 ) ;
	}
	for ( size_t i = iUpdateFrom; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( pEntry != nullptr )
		{
			yLine += pEntry->GetListViewHeight() ;
		}
		m_yList.Add( yLine ) ;
	}
	Unlock() ;
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::DrawChildren
	( S3DRenderContextInterface& render,
			SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;
	//
	size_t		iFirst = IndexFromPointY( 0 ) ;
	for ( size_t i = iFirst; i < m_list.GetLength(); i ++ )
	{
		Entry *	pEntry = m_list.GetAt( i ) ;
		if ( pEntry == nullptr )
		{
			continue ;
		}
		SGLPoint		ptList = PointFromIndex( i ) ;
		SGLImageRect	rectList ;
		rectList.SetPosition( ptList ) ;
		rectList.SetSize
			( SGLSize( m_sizeView.w - esl_min( ptList.x, 0 ),
							(int) pEntry->GetListViewHeight() ) ) ;
		pEntry->DrawListEntry( *this, render, rectList ) ;
	}
}

// 表示状態の子スプライトに対し BeforeDraw を呼び出し
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::BeforeDrawChildren( SGLSprite::Stereo3DView s3dView )
{
	SGLSprite::BeforeDrawChildren( s3dView ) ;

	UpdateListHeight() ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::GetRectangle( SGLRect& rectExt ) const
{
	if ( SGLSprite::GetRectangle( rectExt ) )
	{
		if ( !m_sizeView.IsEmpty() )
		{
			rectExt |= SGLRect( 0, 0, m_sizeView.w - 1, m_sizeView.h - 1 ) ;
		}
		return	true ;
	}
	else
	{
		if ( !m_sizeView.IsEmpty() )
		{
			rectExt.left = 0 ;
			rectExt.top = 0 ;
			rectExt.SetSize( m_sizeView ) ;
			return	true ;
		}
	}
	return	false ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::IsHitSprite( double x, double y ) const
{
	if ( !m_sizeView.IsEmpty() )
	{
		if ( (x >= 0) && (x < m_sizeView.w)
			&& (y >= 0) && (y < m_sizeView.h) )
		{
			return	true ;
		}
	}
	return	SGLSprite::IsHitSprite( x, y ) ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////
int SGLSpriteBasicList::GetScrollPos( ScrollDirection scrlDir )
{
	if ( scrlDir == scrollHorz )
	{
		return	m_ptScroll.x ;
	}
	else
	{
		return	m_ptScroll.y ;
	}
}

void SGLSpriteBasicList::SetScrollPos( int nPos, ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	GetViewListRange( range ) ;
	//
	if ( scrlDir == scrollHorz )
	{
		m_ptScroll.x = (int32_t) nPos ;
	}
	else
	{
		m_ptScroll.y = (int32_t) nPos ;
	}
	//
	if ( IsChangedViewListRange( range ) )
	{
		NorifyListChangeViewArea() ;
	}
	PostUpdate() ;
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////
int SGLSpriteBasicList::GetScrollRange( ScrollDirection scrlDir )
{
	int	nRange = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	if ( scrlDir == scrollHorz )
	{
	}
	else
	{
		nRange = (int) GetTotalListHeight() ;
	}
	Unlock() ;
	return	nRange ;
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::AttachScrollBar
	( SGLSprite * pScrollBar, ScrollDirection scrlDir )
{
	if ( (scrlDir == scrollVert)
		|| (scrlDir == scrollDefault) )
	{
		m_refScrollBar = pScrollBar ;
		ReflectToScrollBar() ;
	}
}

void SGLSpriteBasicList::AttachTrackBar( SGLBasicForm::TrackBar * pScrollBar )
{
	m_refTrackBar = pScrollBar ;
	ReflectToScrollBar() ;
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////
void SGLSpriteBasicList::DetachScrollBar
	( SGLSprite * pScrollBar, ScrollDirection scrlDir )
{
	if ( (scrlDir == scrollVert)
		|| (scrlDir == scrollDefault) )
	{
		if ( m_refScrollBar.GetReference() == pScrollBar )
		{
			m_refScrollBar = nullptr ;
		}
	}
}

void SGLSpriteBasicList::DetachTrackBar( SGLBasicForm::TrackBar * pScrollBar )
{
	if ( m_refTrackBar.GetReference() == pScrollBar )
	{
		m_refTrackBar = nullptr ;
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagSwiping && (m_nBehaviorFlags & behaviorSwipable) )
	{
		int	dy = (int) eslRoundR64ToLInt( yPos - m_vLastSwipe.y ) ;
		if ( dy != 0 )
		{
			ViewListRange	range ;
			GetViewListRange( range ) ;
			//
			m_ptScroll.y -= dy ;
			//
			int	nRange = (int) GetTotalListHeight() ;
			if ( m_ptScroll.y + m_sizeView.h > nRange )
			{
				m_ptScroll.y = nRange - m_sizeView.h ;
			}
			if ( m_ptScroll.y < 0 )
			{
				m_ptScroll.y = 0 ;
			}
			//
			if ( IsChangedViewListRange( range ) )
			{
				NorifyListChangeViewArea() ;
			}
			ReflectToScrollBar() ;
			PostUpdate() ;
			//
			m_fpSwipeMoved += fabs( dy ) ;
			m_vLastSwipe.x = xPos ;
			m_vLastSwipe.y = yPos ;
		}
	}
	else
	{
		ssize_t	iEntry = TestHitListEntry( (int) yPos ) ;
		if ( (iEntry != m_iMouseFocus)
			&& !IsDisabledEntryAt( (size_t) iEntry ) )
		{
			SetFocusEntryAt( (size_t) iEntry, true ) ;
			m_iMouseFocus = iEntry ;
		}
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteBasicList::OnMouseLeave( int64_t nFlags )
{
	if ( m_iMouseFocus >= 0 )
	{
		if ( m_iKeyFocus >= 0 )
		{
			SetFocusEntryAt( (size_t) m_iKeyFocus, true ) ;
		}
		else
		{
			SetFocusEntryAt( (size_t) m_iMouseFocus, false ) ;
		}
		m_iMouseFocus = -1 ;
	}
	if ( (m_pPressingEntry != nullptr)
		&& (FindListEntry( m_pPressingEntry ) >= 0) )
	{
		m_pPressingEntry->OnReleasePressEntry( *this ) ;
		m_pPressingEntry = nullptr ;
	}
	if ( m_flagSwiping )
	{
		ReleaseMouseCapture() ;
		m_flagSwiping = false ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	if ( m_nBehaviorFlags & behaviorScrollable )
	{
		size_t	iViewFirst = IndexFromPointY(0) ;
		int	nLinePitch = PointFromIndex(iViewFirst + 1).y
							- PointFromIndex(iViewFirst).y ;
		if ( nLinePitch <= 0 )
		{
			nLinePitch = 16 ;
		}
		m_ptScroll.y -= zDelta * nLinePitch / WheelDeltaUnit ;
		//
		int	nRange = (int) GetTotalListHeight() ;
		if ( m_ptScroll.y + m_sizeView.h > nRange )
		{
			m_ptScroll.y = nRange - m_sizeView.h ;
		}
		if ( m_ptScroll.y < 0 )
		{
			m_ptScroll.y = 0 ;
		}
		//
		NorifyListChangeViewArea() ;
		ReflectToScrollBar() ;
		PostUpdate() ;
		return	true ;
	}
	else
	{
		return	SGLSprite::OnMouseWheel( zDelta, xPos, yPos, nFlags ) ;
	}
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
	//
	m_flagSwiping = true ;
	m_msStartSwiping = CurrentMilliSec() ;
	m_fpSwipeMoved = 0.0 ;
	m_vLastSwipe.x = xPos ;
	m_vLastSwipe.y = yPos ;
	SetMouseCapture() ;
	//
	ssize_t	iList = TestHitListEntry( (int) yPos ) ;
	m_pPressingEntry = GetListEntryAt( (size_t) iList ) ;
	if ( m_pPressingEntry != nullptr )
	{
		m_pPressingEntry->OnPressEntry( *this ) ;
	}
	//
	return	true ;
}

bool SGLSpriteBasicList::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
	//
	if ( (m_pPressingEntry != nullptr)
		&& (FindListEntry( m_pPressingEntry ) >= 0) )
	{
		m_pPressingEntry->OnReleasePressEntry( *this ) ;
		m_pPressingEntry = nullptr ;
	}
	if ( m_flagSwiping )
	{
		ReleaseMouseCapture() ;
		m_flagSwiping = false ;
		//
		if ( (m_fpSwipeMoved < 10.0)
			&& (CurrentMilliSec() - m_msStartSwiping < 500) )
		{
			ssize_t	iList = TestHitListEntry( (int) yPos ) ;
			Entry *	pEntry = GetListEntryAt( (size_t) iList ) ;
			if ( (pEntry != nullptr)
				&& !(pEntry->IsEntryDisabled()) )
			{
				if ( (pEntry == m_pLastClicked)
					&& (CurrentMilliSec() - m_msLastClicked < 600) )
				{
					pEntry->OnDoubleClickEntry( *this ) ;
					m_pLastClicked = nullptr ;
				}
				else
				{
					pEntry->OnClickEntry( *this ) ;
					m_pLastClicked = pEntry ;
					m_msLastClicked = CurrentMilliSec() ;
				}
			}
		}
	}
	return	true ;
}

// 位置が移動した
//////////////////////////////////////////////////////////////////////////
bool SGLSpriteBasicList::OnScroll
	( SGLSpriteScrollBar& scroll, int64_t codeNotify )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ViewListRange	range ;
	GetViewListRange( range ) ;
	//
	int	nPos = scroll.GetScrollPos() ;
	m_ptScroll.y = nPos ;
	//
	if ( IsChangedViewListRange( range ) )
	{
		NorifyListChangeViewArea() ;
	}
	Unlock() ;
	return	false ;
}

// 複製
//////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteBasicList::DuplicateObject( void )
{
	return	new SGLSpriteBasicList( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteBasicList::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	int32_t		nViewProximity = (int32_t) m_nViewProximity ;
	if ( SGLSprite::IsBuffered() )
	{
		nFlags |= 0x01 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &m_nBehaviorFlags, sizeof(uint32_t) ) ;
	file.Write( &m_sizeView, sizeof(SGLSize) ) ;
	file.Write( &nViewProximity, sizeof(int32_t) ) ;
	file.Write( &m_ptScroll, sizeof(SGLPoint) ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteBasicList::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	int32_t		nViewProximity = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	file.Read( &m_nBehaviorFlags, sizeof(uint32_t) ) ;
	file.Read( &m_sizeView, sizeof(SGLSize) ) ;
	file.Read( &nViewProximity, sizeof(int32_t) ) ;
	file.Read( &m_ptScroll, sizeof(SGLPoint) ) ;
	m_nViewProximity = nViewProximity ;
	//
	if ( m_sizeView.IsEmpty() )
	{
		ReleaseView() ;
	}
	else
	{
		CreateView
			( (uint32_t) m_sizeView.w,
				(uint32_t) m_sizeView.h, ((nFlags & 0x01) != 0) ) ;
	}
	return	sglErrSuccess ;
}




//////////////////////////////////////////////////////////////////////////////
// メニュー表示スタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
SGLSpriteMenuList::MenuStyle::MenuStyle( void )
	: menuType(menuButton),
		ptTextOffset(0,0), rectTextMargin(0,0,0,0), nMenuHeight(32)
{
	for ( int i = 0; i < statusCount; i ++ )
	{
		argbBackColor[i] = 0 ;
	}
}

// 構築関数（複製）
SGLSpriteMenuList::MenuStyle::MenuStyle( const MenuStyle& style )
	: SGLSpriteMessage::RichTextStyle( style ),
		menuType( style.menuType ),
		ptTextOffset( style.ptTextOffset ),
		rectTextMargin( style.rectTextMargin ),
		nMenuHeight( style.nMenuHeight )
{
	for ( int i = 0; i < statusCount; i ++ )
	{
		argbBackColor[i] = style.argbBackColor[i] ;
	}
}

// 代入
const SGLSpriteMenuList::MenuStyle&
	SGLSpriteMenuList::MenuStyle::operator =
		( const SGLSpriteMenuList::MenuStyle& style )
{
	SGLSpriteMessage::RichTextStyle::operator = ( style ) ;
	menuType = style.menuType ;
	ptTextOffset = style.ptTextOffset ;
	rectTextMargin = style.rectTextMargin ;
	nMenuHeight = style.nMenuHeight ;
	for ( int i = 0; i < statusCount; i ++ )
	{
		argbBackColor[i] = style.argbBackColor[i] ;
	}
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// メニューリストアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMenuList::MenuEntry, Entry )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMenuList::MenuEntry::MenuEntry( const wchar_t * pwszID, int64_t nCmdParam )
	: m_strID( pwszID ), m_nCmdParam( nCmdParam ),
		m_nHeight( 32 ), m_rectMargin( 0, 0, 0, 0 ),
		m_ptText( 0, 0 ), m_sizeAlign( 0, 0 )
{
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::MenuEntry::SetText
	( SGLSpriteMenuList& menu, const wchar_t * pwszText )
{
	m_nHeight = menu.GetMenuStyle().nMenuHeight ;
	m_rectMargin = menu.GetMenuStyle().rectTextMargin ;
	m_strText = pwszText ;
	m_imgText = nullptr ;

	if ( !m_strText.IsEmpty() && (m_viewArea != viewOutside) )
	{
		menu.AsyncPrepareToDrawEntry( this ) ;
	}
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& SGLSpriteMenuList::MenuEntry::GetText( void ) const
{
	return	m_strText ;
}

// クリック通知処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::MenuEntry::NotifyMenuClick
	( SGLSpriteBasicList& listView, MenuNotificationCode ncode )
{
	if ( !m_strID.IsEmpty() )
	{
		listView.OnCommand( m_strID, m_nCmdParam, ncode ) ;
	}
	else if ( !listView.GetID().IsEmpty() )
	{
		listView.OnCommand( listView.GetID(), m_nCmdParam, ncode ) ;
	}
}

// クリック時
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::MenuEntry::OnPressEntry( SGLSpriteBasicList& listView )
{
	Entry::OnPressEntry( listView ) ;
	listView.PostUpdate() ;
}

void SGLSpriteMenuList::MenuEntry::OnReleasePressEntry( SGLSpriteBasicList& listView )
{
	Entry::OnReleasePressEntry( listView ) ;
	listView.PostUpdate() ;
}

void SGLSpriteMenuList::MenuEntry::OnClickEntry( SGLSpriteBasicList& listView )
{
	SGLSpriteMenuList *	pMenuList = ESLTypeCast<SGLSpriteMenuList>( &listView ) ;
	const MenuType	menuType =
		(pMenuList != nullptr) ? pMenuList->GetMenuStyle().menuType : menuButton ;
	//
	switch ( menuType )
	{
	default:
	case	menuButton:
		NotifyMenuClick( listView, ncodeClicked ) ;
		break ;

	case	menuCheck:
		SelectEntry( !IsEntrySelected() ) ;
		listView.PostUpdate() ;
		NotifyMenuClick
			( listView, (IsEntrySelected() ? ncodeChecked : ncodeUnchecked) ) ;
		break ;

	case	menuRadio:
		if ( !IsEntrySelected() )
		{
			listView.SelectEntryAt
				( (size_t) listView.FindListEntry( this ), true, true ) ;
			NotifyMenuClick( listView, ncodeChecked ) ;
		}
		break ;
	}
}

void SGLSpriteMenuList::MenuEntry::OnDoubleClickEntry( SGLSpriteBasicList& listView )
{
	MenuEntry::OnClickEntry( listView ) ;
}

// 表示サイズ計算
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMenuList::MenuEntry::GetListViewWidth( void ) const
{
	if ( m_imgText != nullptr )
	{
		return	(int) m_imgText->GetImageWidth()
					+ esl_max( m_ptText.x, 0 )
					+ m_rectMargin.left + m_rectMargin.right ;
	}
	return	0 ;
}

size_t SGLSpriteMenuList::MenuEntry::GetListViewHeight( void ) const
{
	return	m_nHeight ;
}

// 描画準備
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::MenuEntry::OnPrepareToDraw( const SGLSpriteBasicList& listView )
{
	const SGLSpriteMenuList *
			pMenuList = ESLConstTypeCast<SGLSpriteMenuList>( &listView ) ;
	if ( !m_strText.IsEmpty() && (pMenuList != nullptr) )
	{
		SSmartPointer<SGLImageObject>	imgText = new SGLImage ;
		if ( !pMenuList->RasterizeImageOfTextXML
			( *imgText, m_ptText, m_sizeAlign, m_strText ) )
		{
			if ( !m_flagDetached )
			{
				pMenuList->Lock() ;
				m_imgText = imgText.Detach() ;
				pMenuList->Unlock() ;
			}
		}
	}
	Entry::OnPrepareToDraw( listView ) ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::MenuEntry::DrawListEntry
	( const SGLSpriteBasicList& listView,
		S3DRenderContextInterface& render,
		const SGLImageRect& rectList )
{
	MenuStatus	status = statusNormal ;
	if ( !IsEntryDisabled() )
	{
		if ( IsPressingEntry() )
		{
			status = statusPressed ;
		}
		else if ( HasEntryFocus() )
		{
			status = IsEntrySelected() ? statusFocusSelected : statusFocus ;
		}
		else
		{
			status = IsEntrySelected() ? statusSelected : statusNormal ;
		}
	}
	else
	{
		status = IsEntrySelected() ? statusSelectDisabled : statusDisabled ;
	}
	const SGLSpriteMenuList *
			pMenuList = ESLConstTypeCast<SGLSpriteMenuList>( &listView ) ;
	if ( (pMenuList != nullptr)
		&& (pMenuList->GetMenuStyle().argbBackColor[status].ui32 != 0) )
	{
		render.FillRectangle
			( rectList.x, rectList.y,
				rectList.w, rectList.h,
				pMenuList->GetMenuStyle().argbBackColor[status],
				0.0, paintDelayable ) ;
	}
	if ( m_imgText != nullptr )
	{
		SGLPaintParam	ppPaint ;
		ppPaint.nFlags = paintDelayable ;
		ppPaint.ptPaint.x = rectList.x + m_rectMargin.left ;
		ppPaint.ptPaint.y = rectList.y + m_rectMargin.top ;
		if ( pMenuList != nullptr )
		{
			ppPaint.ptPaint +=
				pMenuList->CalcDrawOffsetOfImageText( m_ptText, m_sizeAlign ) ;
		}
		if ( IsEntryDisabled() )
		{
			ppPaint.nTransparency = 0x80 ;
		}
		render.DrawImage( ppPaint, m_imgText ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// メニュースプライト
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	SGLSpriteMenuList::s_aiMenuType[SGLSpriteMenuList::menuTypeCount+1] =
{
	{ L"button", menuButton },
	{ L"check", menuCheck },
	{ L"radio", menuRadio },
	{ nullptr, menuButton },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMenuList, SGLSpriteBasicList )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMenuList::SGLSpriteMenuList( void )
{
}

SGLSpriteMenuList::SGLSpriteMenuList( const SGLSpriteMenuList& src )
	: SGLSpriteBasicList( src )
{
	SetMenuStyle( src.GetMenuStyle() ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMenuList::~SGLSpriteMenuList( void )
{
}

// メニュースタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteMenuList::MenuStyle&
	SGLSpriteMenuList::GetMenuStyle( void ) const
{
	return	m_styleMenu ;
}

// メニュースタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::SetMenuStyle( const MenuStyle& style )
{
	m_styleMenu = style ;
	//
	m_strFontFace = m_styleMenu.font.pszFace ;
	m_strRubyFont = m_styleMenu.fontRuby.pszFace ;
	m_strProhibition = m_styleMenu.context.pwszProhibition ;
	//
	m_styleMenu.font.pszFace = m_strFontFace ;
	m_styleMenu.fontRuby.pszFace = m_strRubyFont ;
	m_styleMenu.context.pwszProhibition = m_strProhibition ;
}

void SGLSpriteMenuList::SetMenuStyleXML( const SSystem::SXMLDocument& xmlStyle )
{
	MenuStyle	style ;
	SString		strFontFace, strRubyFont ;
	ParseMenuStyle( style, strFontFace, strRubyFont, xmlStyle ) ;
	//
	SetMenuStyle( style ) ;
}

// スタイル解釈
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMenuList::ParseMenuStyle
	( MenuStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strRubyFont,
		const SSystem::SXMLDocument& xmlStyle )
{
	SGLSpriteMessage::ParseRichTextStyle
		( style, strFontFace, strRubyFont, xmlStyle ) ;
	//
	// <menu>
	SXMLDocument *	pxmlMenu = xmlStyle.GetElementTagAs( L"menu" ) ;
	if ( pxmlMenu != nullptr )
	{
		style.menuType =
			(MenuType) pxmlMenu->GetAttrSymbolizedIntegerAs
						( L"menu_type", s_aiMenuType, style.menuType ) ;
		//
		style.ptTextOffset.x =
			(int32_t) pxmlMenu->GetAttrIntegerAs( L"text_x", style.ptTextOffset.x ) ;
		style.ptTextOffset.y =
			(int32_t) pxmlMenu->GetAttrIntegerAs( L"text_y", style.ptTextOffset.y ) ;
		//
		style.rectTextMargin.left =
			(int32_t) pxmlMenu->GetAttrIntegerAs
						( L"margin_left", style.rectTextMargin.left ) ;
		style.rectTextMargin.top =
			(int32_t) pxmlMenu->GetAttrIntegerAs
						( L"margin_top", style.rectTextMargin.top ) ;
		style.rectTextMargin.right =
			(int32_t) pxmlMenu->GetAttrIntegerAs
						( L"margin_right", style.rectTextMargin.right ) ;
		style.rectTextMargin.bottom =
			(int32_t) pxmlMenu->GetAttrIntegerAs
						( L"margin_bottom", style.rectTextMargin.bottom ) ;
		//
		style.nMenuHeight =
			(size_t) pxmlMenu->GetAttrIntegerAs( L"height", style.nMenuHeight ) ;
		//
		SXMLDocument *	pxmlBackColor = pxmlMenu->GetElementTagAs( L"back_color" ) ;
		if ( pxmlBackColor != nullptr )
		{
			static const wchar_t *	s_pwszAttrNames[statusCount] =
			{
				L"normal_hex",
				L"focus_hex",
				L"selected_hex",
				L"sel_focus_hex",
				L"pressed_hex",
				L"disabled_hex",
				L"sel_disabled_hex",
			} ;
			for ( int i = 0; i < statusCount; i ++ )
			{
				style.argbBackColor[i].ui32 =
					(uint32_t) pxmlBackColor->GetAttrHexIntegerAs
						( s_pwszAttrNames[i], style.argbBackColor[i].ui32 ) ;
			}
		}
	}
}

// 文字画像生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMenuList::RasterizeImageOfTextXML
	( SGLImageObject& imgText,
		SGLPoint& ptOffset,
		SGLSize& sizeAlign, const wchar_t * pwszTextXML ) const
{
	SGLSpriteMessage	msg ;
	SGLSpriteMessage::RichTextStyle
						style = m_styleMenu ;
	style.context.rectWritable.SetPosition( SGLPoint( 0, 0 ) ) ;
	style.context.rectWritable.SetSize( CalcSizeOfMenuTextArea() ) ;
	sizeAlign = style.context.rectWritable.GetSize() ;
	//
	msg.SetRichTextStyle( style ) ;
	msg.AddMessageXML( pwszTextXML ) ;
	return	msg.RasterizeTextImage( imgText, ptOffset ) ;
}

// 描画位置アライメント調整
//////////////////////////////////////////////////////////////////////////////
SGLPoint SGLSpriteMenuList::CalcDrawOffsetOfImageText
	( const SGLPoint& ptOffset, const SGLSize& sizeAlign ) const
{
	SGLPoint		ptDrawOffset = ptOffset ;
	const SGLSize	sizeView = CalcSizeOfMenuTextArea() ;

	switch ( m_styleMenu.context.typeAlignment )
	{
	case	SGLLetteringContext::alignRight:
		ptDrawOffset.x += (sizeView.w - sizeAlign.w) ;
		break ;
	case	SGLLetteringContext::alignCenter:
		ptDrawOffset.x += (sizeView.w - sizeAlign.w) / 2 ;
		break ;
	default:
		break ;
	}
	switch ( m_styleMenu.boxAlign & SGLSpriteText::alignBoxVertMask )
	{
	case	SGLSpriteText::alignBoxBottom:
		ptDrawOffset.y += (sizeView.h - sizeAlign.h) ;
		break ;
	case	SGLSpriteText::alignBoxVCenter:
		ptDrawOffset.y += (sizeView.h - sizeAlign.h) / 2 ;
		break ;
	default:
		break ;
	}
	return	ptDrawOffset ;
}

// 現在のスタイルでのメニュー文字表示領域サイズを計算
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLSpriteMenuList::CalcSizeOfMenuTextArea( void ) const
{
	SGLSize	sizeView( m_sizeView.w - m_styleMenu.rectTextMargin.left
									- m_styleMenu.rectTextMargin.right,
					(int) m_styleMenu.nMenuHeight
								- m_styleMenu.rectTextMargin.top
								- m_styleMenu.rectTextMargin.bottom ) ;
	if ( sizeView.w < (int32_t) m_styleMenu.font.nSize )
	{
		sizeView.w = (int32_t) m_styleMenu.font.nSize ;
	}
	if ( sizeView.h < (int32_t) m_styleMenu.font.nSize )
	{
		sizeView.h = (int32_t) m_styleMenu.font.nSize ;
	}
	return	sizeView ;
}

// メニューエントリ追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMenuList::AddMenuEntry
	( const wchar_t * pwszTextXML, const wchar_t * pwszID, int64_t nCmdParam )
{
	MenuEntry *	pEntry = new MenuEntry( pwszID, nCmdParam ) ;
	pEntry->SetText( *this, pwszTextXML ) ;
	return	AddListEntry( pEntry ) ;
}

