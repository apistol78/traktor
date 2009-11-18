#ifndef traktor_editor_NewInstanceDialog_H
#define traktor_editor_NewInstanceDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class TreeView;
class ListView;
class Edit;

		namespace custom
		{

class MiniButton;

		}
	}

	namespace editor
	{

class Settings;

class NewInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	NewInstanceDialog(Settings* settings);

	bool create(ui::Widget* parent);

	const std::wstring& getTypeName() const;

	const std::wstring& getInstanceName() const;

private:
	Ref< Settings > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::ListView > m_typeList;
	Ref< ui::custom::MiniButton > m_buttonIcon;
	Ref< ui::custom::MiniButton > m_buttonSmall;
	Ref< ui::Edit > m_editInstanceName;
	std::wstring m_typeName;
	std::wstring m_instanceName;

	void eventDialogClick(ui::Event* event);

	void eventTreeItemSelected(ui::Event* event);

	void eventButtonClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_NewInstanceDialog_H
