#ifndef traktor_editor_BrowseTypeDialog_H
#define traktor_editor_BrowseTypeDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class TreeView;
class TreeViewItem;

		namespace custom
		{

class PreviewList;

		}
	}

	namespace editor
	{

class BrowseTypeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	BrowseTypeDialog(PropertyGroup* settings);

	bool create(ui::Widget* parent, const TypeInfo* base, bool onlyEditable, bool onlyInstantiable);

	virtual void destroy();

	const TypeInfo* getSelectedType() const;

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::custom::PreviewList > m_typeList;
	const TypeInfo* m_type;

	void eventDialogClick(ui::ButtonClickEvent* event);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);

	void eventListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

#endif	// traktor_editor_BrowseTypeDialog_H
