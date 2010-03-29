#ifndef traktor_editor_BrowseTypeDialog_H
#define traktor_editor_BrowseTypeDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class Settings;

	namespace ui
	{

class TreeView;
class TreeViewItem;
class ListView;

		namespace custom
		{

class MiniButton;

		}
	}

	namespace editor
	{

class BrowseTypeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	BrowseTypeDialog(Settings* settings);

	bool create(ui::Widget* parent, const TypeInfo* base);

	const TypeInfo* getSelectedType() const;

private:
	Ref< Settings > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::ListView > m_typeList;
	Ref< ui::custom::MiniButton > m_buttonIcon;
	Ref< ui::custom::MiniButton > m_buttonSmall;
	std::wstring m_typeName;

	void eventDialogClick(ui::Event* event);

	void eventTreeItemSelected(ui::Event* event);

	void eventListDoubleClick(ui::Event* event);

	void eventButtonClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_BrowseTypeDialog_H
