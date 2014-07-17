#include "Core/Log/Log.h"
#include "Ui/TreeView.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/TreeViewItemWin32.h"
#include "Ui/Win32/TreeViewWin32.h"

namespace traktor
{
	namespace ui
	{

TreeViewWin32::TreeViewWin32(EventSubject* owner)
:	WidgetWin32Impl< ITreeView >(owner)
,	m_hImageList(NULL)
{
}

bool TreeViewWin32::create(IWidget* parent, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & TreeView::WsAutoEdit)
		nativeStyle |= TVS_EDITLABELS;
	if (style & TreeView::WsTreeButtons)
		nativeStyle |= TVS_HASBUTTONS;
	if (style & TreeView::WsTreeLines)
		nativeStyle |= TVS_HASLINES | TVS_LINESATROOT;
	if (!(style & TreeView::WsDrag))
		nativeStyle |= TVS_DISABLEDRAGDROP;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		WC_TREEVIEW,
		_T(""),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TVS_SHOWSELALWAYS | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_LBUTTONUP, new MethodMessageHandler< TreeViewWin32 >(this, &TreeViewWin32::eventButtonUp));
	m_hWnd.registerMessageHandler(WM_REFLECTED_NOTIFY, new MethodMessageHandler< TreeViewWin32 >(this, &TreeViewWin32::eventNotify));

	return true;
}

void TreeViewWin32::destroy()
{
	removeAllItems();
	WidgetWin32Impl< ITreeView >::destroy();
}

int TreeViewWin32::addImage(IBitmap* image, int imageCount)
{
	if (!m_hImageList)
	{
		m_hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 0);
		m_hWnd.sendMessage(TVM_SETIMAGELIST, 0, (LPARAM)m_hImageList);
	}

	HBITMAP hImageBitmap = reinterpret_cast< BitmapWin32* >(image)->createClone();
	if (!hImageBitmap)
		return -1;

	ImageList_AddMasked(m_hImageList, hImageBitmap, reinterpret_cast< BitmapWin32* >(image)->getMask());

	DeleteObject(hImageBitmap);
	return 0;
}

Ref< TreeViewItem > TreeViewWin32::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	if (expandedImage < 0)
		expandedImage = image;

	Ref< TreeViewItemWin32 > item = new TreeViewItemWin32(this, parent, image, expandedImage, (HWND)m_hWnd);

	tstring ttext = wstots(text);

	TV_INSERTSTRUCT tvis;
	std::memset(&tvis, 0, sizeof(tvis));
	tvis.hParent = parent ? checked_type_cast< TreeViewItemWin32* >(parent)->getHandle() : NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText = (LPTSTR)ttext.c_str();
	tvis.item.iImage = image;
	tvis.item.iSelectedImage = tvis.item.iImage;
	tvis.item.lParam = (LPARAM)item.ptr();

	HTREEITEM hItem = TreeView_InsertItem((HWND)m_hWnd, &tvis);
	if (!hItem)
		return 0;

	item->setHandle(hItem);

	m_items[hItem] = item;

	return item;
}

void TreeViewWin32::removeItem(TreeViewItem* item)
{
	Ref< TreeViewItemWin32 > item_ = checked_type_cast< TreeViewItemWin32* >(item);
	T_ASSERT_M (item_->getOwner() == this, L"Incorrect owner of tree item");

	RefArray< TreeViewItem > children;
	item->getChildren(children);

	for (RefArray< TreeViewItem >::iterator i = children.begin(); i != children.end(); ++i)
		removeItem(*i);

	std::map< HTREEITEM, Ref< TreeViewItemWin32 > >::iterator i = m_items.find(item_->getHandle());
	T_ASSERT_M (i != m_items.end(), L"Item not in tree, already removed?");
	T_ASSERT (i->second == item_);

	m_items.erase(i);

	TreeView_DeleteItem(m_hWnd, item_->getHandle());
	item_->setHandle(NULL);
}

void TreeViewWin32::removeAllItems()
{
	TreeView_DeleteAllItems(m_hWnd);

	for (std::map< HTREEITEM, Ref< TreeViewItemWin32 > >::iterator i = m_items.begin(); i != m_items.end(); ++i)
		i->second->setHandle(NULL);

	m_items.clear();
}

Ref< TreeViewItem > TreeViewWin32::getRootItem() const
{
	HTREEITEM hRootItem = TreeView_GetRoot(m_hWnd);
	if (!hRootItem)
		return 0;

	return getFromHandle(hRootItem);
}

Ref< TreeViewItem > TreeViewWin32::getSelectedItem() const
{
	HTREEITEM hSelectedItem = (HTREEITEM)m_hWnd.sendMessage(TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)TVI_ROOT);
	if (!hSelectedItem)
		return 0;

	return getFromHandle(hSelectedItem);
}

Ref< TreeViewItemWin32 > TreeViewWin32::getFromHandle(HTREEITEM hItem) const
{
	std::map< HTREEITEM, Ref< TreeViewItemWin32 > >::const_iterator i = m_items.find(hItem);
	T_ASSERT_M (i != m_items.end(), L"No such item");

	return i->second;
}

LRESULT TreeViewWin32::eventButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	if (!m_dragItem)
		return 0;

	// Notify position in screen coordinates as receiver doesn't necessarily know
	// from which tree-view the event originated.
	Point position = clientToScreen(Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
	TreeViewDragEvent dragEvent(m_owner, m_dragItem, TreeViewDragEvent::DmDrop, position);
	m_owner->raiseEvent(&dragEvent);

	m_dragItem = 0;
	releaseCapture();
	setCursor(CrArrow);

	return 0;
}

LRESULT TreeViewWin32::eventNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
	LRESULT result = FALSE;

	switch (nmhdr->code)
	{
	case TVN_SELCHANGED:
		{
			LPNMTREEVIEW nmtv = reinterpret_cast< LPNMTREEVIEW >(nmhdr);
			if (nmtv->action == TVC_BYKEYBOARD || nmtv->action == TVC_BYMOUSE)
			{
				SelectionChangeEvent selectionChangeEvent(m_owner);
				m_owner->raiseEvent(&selectionChangeEvent);
			}
		}
		break;

	case TVN_ITEMEXPANDED:
		{
			LPNMTREEVIEW nmtv = reinterpret_cast< LPNMTREEVIEW >(nmhdr);

			Ref< TreeViewItemWin32 > item = getFromHandle(nmtv->itemNew.hItem);
			T_ASSERT (item);

			item->updateImage();
		}
		break;

	case TVN_BEGINLABELEDIT:
		{
			LPNMTVDISPINFO nmtvdi = reinterpret_cast< LPNMTVDISPINFO >(nmhdr);
			if (nmtvdi->item.hItem)
			{
				Ref< TreeViewItemWin32 > item = getFromHandle(nmtvdi->item.hItem);
				T_ASSERT (item);

				if (!item->isEditable())
					return TRUE;

				TreeViewEditEvent editEvent(m_owner, item);
				m_owner->raiseEvent(&editEvent);

				if (editEvent.consumed() && editEvent.cancelled())
					return TRUE;

				// In case the item's value has changed in the event handler we need
				// to ensure the edit control has the proper value too.
				HWND hEditCtrl = TreeView_GetEditControl(hWnd);
				if (hEditCtrl != NULL)
				{
					std::wstring labelText = item->getText();
					SetWindowText(hEditCtrl, wstots(labelText).c_str());
				}
			}
			else
				return TRUE;
		}
		break;

	case TVN_ENDLABELEDIT:
		{
			LPNMTVDISPINFO nmtvdi = reinterpret_cast< LPNMTVDISPINFO >(nmhdr);
			if (nmtvdi->item.pszText && nmtvdi->item.hItem)
			{
				Ref< TreeViewItemWin32 > item = getFromHandle(nmtvdi->item.hItem);
				T_ASSERT (item);

				std::wstring newText = nmtvdi->item.pszText;
				std::wstring originalText = item->getText();

				item->setText(newText);

				TreeViewContentChangeEvent contentChangeEvent(m_owner, item);
				m_owner->raiseEvent(&contentChangeEvent);

				if (!contentChangeEvent.consumed())
				{
					// Rename event not consumed; revert item text.
					item->setText(originalText);
				}
				else
				{
					// Ensure content is set to the correct value.
					_tcscpy(m_editBuffer, item->getText().c_str());
					nmtvdi->item.pszText = m_editBuffer;
					result = TRUE;
				}
			}
		}
		break;

	case TVN_BEGINDRAG:
		{
			LPNMTREEVIEW nmtv = reinterpret_cast< LPNMTREEVIEW >(nmhdr);
			Ref< TreeViewItemWin32 > item = getFromHandle(nmtv->itemNew.hItem);
			if (item)
			{
				// Notify listener we're about to begin drag item.
				TreeViewDragEvent dragEvent(m_owner, item, TreeViewDragEvent::DmDrag);
				m_owner->raiseEvent(&dragEvent);
				if (!(dragEvent.consumed() && dragEvent.cancelled()))
				{
					m_dragItem = item;
					setCursor(CrHand);
					setCapture();
				}
			}
		}
		break;

	case NM_DBLCLK:
		{
			TV_HITTESTINFO tvhti;
			GetCursorPos(&tvhti.pt);
			ScreenToClient(m_hWnd, &tvhti.pt);
			if (m_hWnd.sendMessage(TVM_HITTEST, 0, (LPARAM)&tvhti) && tvhti.flags & TVHT_ONITEM)
			{
				TreeViewItemActivateEvent activateEvent(m_owner, getSelectedItem());
				m_owner->raiseEvent(&activateEvent);
			}
		}
		break;

	default:
		pass = true;
		break;
	}

	return result;
}

	}
}
