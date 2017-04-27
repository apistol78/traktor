/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_BrowseTypeDialog_H
#define traktor_editor_BrowseTypeDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{
		namespace custom
		{

class PreviewList;
class TreeView;
class TreeViewItem;

		}
	}

	namespace editor
	{

class BrowseTypeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	BrowseTypeDialog(PropertyGroup* settings);

	bool create(ui::Widget* parent, const TypeInfoSet* base, bool onlyEditable, bool onlyInstantiable);

	virtual void destroy() T_OVERRIDE T_FINAL;

	const TypeInfo* getSelectedType() const;

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::custom::TreeView > m_categoryTree;
	Ref< ui::custom::PreviewList > m_typeList;
	const TypeInfo* m_type;

	void updatePreviewList();

	void eventDialogClick(ui::ButtonClickEvent* event);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);

	void eventListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

#endif	// traktor_editor_BrowseTypeDialog_H
