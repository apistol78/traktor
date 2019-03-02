#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class Edit;
class ListView;
class PreviewList;
class TreeView;

	}

	namespace editor
	{

class NewInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	NewInstanceDialog(PropertyGroup* settings);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	const TypeInfo* getType() const;

	const std::wstring& getInstanceName() const;

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::PreviewList > m_typeList;
	Ref< ui::Edit > m_editInstanceName;
	const TypeInfo* m_type;
	std::wstring m_instanceName;

	void updatePreviewList();

	void eventDialogClick(ui::ButtonClickEvent* event);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);
};

	}
}

