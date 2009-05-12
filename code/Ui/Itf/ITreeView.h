#ifndef traktor_ui_ITreeView_H
#define traktor_ui_ITreeView_H

#include "Ui/Itf/IWidget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IBitmap;

class TreeViewItem;

/*! \brief TreeView interface.
 * \ingroup UI
 */
class T_DLLCLASS ITreeView : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual int addImage(IBitmap* image, int imageCount) = 0;

	virtual TreeViewItem* createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage) = 0;

	virtual void removeItem(TreeViewItem* item) = 0;

	virtual void removeAllItems() = 0;

	virtual TreeViewItem* getRootItem() const = 0;

	virtual TreeViewItem* getSelectedItem() const = 0;
};

	}
}

#endif	// traktor_ui_ITreeView_H
