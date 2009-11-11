#include "Ui/Win32/TreeViewItemWin32.h"
#include "Ui/Win32/TreeViewWin32.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemWin32", TreeViewItemWin32, TreeViewItem)

TreeViewItemWin32::TreeViewItemWin32(TreeViewWin32* owner, TreeViewItem* parent, int image, int expandedImage, HWND hWndTree)
:	m_owner(owner)
,	m_image(image)
,	m_expandedImage(expandedImage)
,	m_hWndTree(hWndTree)
,	m_hItem(NULL)
{
}

TreeViewItemWin32::~TreeViewItemWin32()
{
	T_ASSERT (!m_hItem);
}

void TreeViewItemWin32::setText(const std::wstring& text)
{
	tstring ttext = wstots(text);

	TV_ITEM tvi;
	memset(&tvi, 0, sizeof(tvi));
	tvi.mask = TVIF_TEXT;
	tvi.hItem = m_hItem;
	tvi.pszText = (LPTSTR)ttext.c_str();
	TreeView_SetItem(m_hWndTree, &tvi);
}

std::wstring TreeViewItemWin32::getText() const
{
	TCHAR buf[_MAX_PATH];
	
	TV_ITEM tvi;
	memset(&tvi, 0, sizeof(tvi));
	tvi.mask = TVIF_TEXT;
	tvi.hItem = m_hItem;
	tvi.cchTextMax = sizeof_array(buf);
	tvi.pszText = buf;
	TreeView_GetItem(m_hWndTree, &tvi);

	return tstows(buf);
}

void TreeViewItemWin32::setImage(int image)
{
	m_image = image;
	updateImage();
}

int TreeViewItemWin32::getImage() const
{
	return m_image;
}

void TreeViewItemWin32::setExpandedImage(int expandedImage)
{
	m_expandedImage = expandedImage;
	updateImage();
}

int TreeViewItemWin32::getExpandedImage() const
{
	return m_expandedImage;
}

bool TreeViewItemWin32::isExpanded() const
{
#if !defined(WINCE)
	BOOL expanded = TreeView_GetItemState(m_hWndTree, m_hItem, TVIS_EXPANDED) & TVIS_EXPANDED;
#else
	BOOL expanded = FALSE;
#endif
	return bool(expanded != 0);
}

void TreeViewItemWin32::expand()
{
	TreeView_Expand(m_hWndTree, m_hItem, TVE_EXPAND);
}

bool TreeViewItemWin32::isCollapsed() const
{
	return !isExpanded();
}

void TreeViewItemWin32::collapse()
{
	TreeView_Expand(m_hWndTree, m_hItem, TVE_COLLAPSE);
}

bool TreeViewItemWin32::isSelected() const
{
#if !defined(WINCE)
	BOOL selected = TreeView_GetItemState(m_hWndTree, m_hItem, TVIS_SELECTED);
#else
	BOOL selected = FALSE;
#endif
	return bool(selected != 0);
}

void TreeViewItemWin32::select()
{
	TreeView_SelectItem(m_hWndTree, m_hItem);
}

bool TreeViewItemWin32::edit()
{
	SetFocus(m_hWndTree);
	TreeView_SelectItem(m_hWndTree, m_hItem);
	return TreeView_EditLabel(m_hWndTree, m_hItem) != NULL;
}

Ref< TreeViewItem > TreeViewItemWin32::getParent() const
{
	HTREEITEM hParentItem = TreeView_GetParent(m_hWndTree, m_hItem);
	if (!hParentItem)
		return 0;

	return m_owner->getFromHandle(hParentItem);
}

bool TreeViewItemWin32::hasChildren() const
{
	return TreeView_GetChild(m_hWndTree, m_hItem) != NULL;
}

int TreeViewItemWin32::getChildren(RefArray< TreeViewItem >& outChildren) const
{
	HTREEITEM hChildItem = TreeView_GetChild(m_hWndTree, m_hItem);
	while (hChildItem)
	{
		outChildren.push_back(m_owner->getFromHandle(hChildItem));
		hChildItem = TreeView_GetNextSibling(m_hWndTree, hChildItem);
	}
	return int(outChildren.size());
}

TreeViewWin32* TreeViewItemWin32::getOwner() const
{
	return m_owner;
}

void TreeViewItemWin32::setHandle(HTREEITEM hItem)
{
	m_hItem = hItem;
}

HTREEITEM TreeViewItemWin32::getHandle() const
{
	return m_hItem;
}

void TreeViewItemWin32::updateImage()
{
	TV_ITEM tvi;
	memset(&tvi, 0, sizeof(tvi));
	tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.hItem = m_hItem;
	tvi.iImage = isExpanded() ? m_expandedImage : m_image;
	tvi.iSelectedImage = tvi.iImage;
	TreeView_SetItem(m_hWndTree, &tvi);
}

	}
}