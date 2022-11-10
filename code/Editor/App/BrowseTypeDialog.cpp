/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Split.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Splitter.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

// Resources
#include "Resources/Folders.h"

#pragma warning(disable: 4344)

namespace traktor
{
	namespace editor
	{
		namespace
		{

class TypeInfoWrapper : public Object
{
public:
	const TypeInfo& m_typeInfo;

	TypeInfoWrapper(const TypeInfo& typeInfo)
	:	m_typeInfo(typeInfo)
	{
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseTypeDialog", BrowseTypeDialog, ui::ConfigDialog)

BrowseTypeDialog::BrowseTypeDialog(PropertyGroup* settings)
:	m_settings(settings)
,	m_type(0)
{
}

bool BrowseTypeDialog::create(ui::Widget* parent, const TypeInfoSet* base, bool onlyEditable, bool onlyInstantiable)
{
	TypeInfoSet types;
	if (base)
	{
		for (auto baseType : *base)
		{
			auto derivedTypes = baseType->findAllOf();
			types.insert(derivedTypes.begin(), derivedTypes.end());
		}
	}
	else
		types = type_of< Object >().findAllOf(false);

	if (types.empty())
		return false;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"BROWSE_TYPE_TITLE"),
		ui::dpi96(640),
		ui::dpi96(500),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &BrowseTypeDialog::eventDialogClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	if (!splitter->create(this, true, ui::dpi96(200)))
		return false;

	Ref< ui::Container > left = new ui::Container();
	if (!left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Static > treeLabel = new ui::Static();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_TYPE_CATEGORY")))
		return false;

	m_categoryTree = new ui::TreeView();
	if (!m_categoryTree->create(left, ui::WsDoubleBuffer))
		return false;
	m_categoryTree->addImage(new ui::StyleBitmap(L"Editor.Database.Folders"), 2);
	m_categoryTree->addEventHandler< ui::SelectionChangeEvent >(this, &BrowseTypeDialog::eventTreeItemSelected);

	Ref< ui::Container > right = new ui::Container();
	if (!right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(right, i18n::Text(L"BROWSE_TYPE_TYPES")))
		return false;

	m_typeList = new ui::PreviewList();
	if (!m_typeList->create(right, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_typeList->addEventHandler< ui::MouseDoubleClickEvent >(this, &BrowseTypeDialog::eventListDoubleClick);

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"BROWSE_TYPE_GLOBAL"), 1);
	groupRoot->setImage(0, 0, 1);

	for (TypeInfoSet::iterator i = types.begin(); i != types.end(); ++i)
	{
		const TypeInfo* type = *i;

		if (onlyEditable && !type->isEditable())
			continue;
		if (onlyInstantiable && !type->isInstantiable())
			continue;

		std::vector< std::wstring > parts;
		if (!Split< std::wstring >::any(type->getName(), L".", parts))
			continue;

		std::wstring className = parts.back(); parts.pop_back();

		Ref< ui::TreeViewItem > group = groupRoot;
		for (std::vector< std::wstring >::iterator j = parts.begin(); j != parts.end(); ++j)
		{
			Ref< ui::TreeViewItem > child = group->findChild(*j);
			if (!child)
			{
				child = m_categoryTree->createItem(group, *j, 1);
				child->setImage(0, 0, 1);
				child->expand();
			}
			group = child;
		}

		Ref< ui::PreviewItems > items = group->getData< ui::PreviewItems >(L"ITEMS");
		if (!items)
		{
			items = new ui::PreviewItems();
			group->setData(L"ITEMS", items);
		}

		Ref< ui::PreviewItem > item = new ui::PreviewItem(className);
		item->setData(L"TYPE", new TypeInfoWrapper(*type));

		items->add(item);
	}

	groupRoot->sort(true);

	// Expand all groups until a group with multiple children is found.
	ui::TreeViewItem* expandGroup = groupRoot;
	while (expandGroup)
	{
		expandGroup->expand();

		const RefArray< ui::TreeViewItem >& children = expandGroup->getChildren();
		if (children.size() == 1)
			expandGroup = children[0];
		else
			break;
	}

	m_categoryTree->update();

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< Ref< ISerializable > >(L"Editor.BrowseTypeTreeState"));
	if (state)
		m_categoryTree->applyState(state);

	updatePreviewList();
	return true;
}

void BrowseTypeDialog::destroy()
{
	if (m_settings)
	{
		Ref< ui::HierarchicalState > state = m_categoryTree->captureState();
		m_settings->setProperty< PropertyObject >(L"Editor.BrowseTypeTreeState", state);
	}

	ui::ConfigDialog::destroy();
}

const TypeInfo* BrowseTypeDialog::getSelectedType() const
{
	return m_type;
}

void BrowseTypeDialog::updatePreviewList()
{
	RefArray< ui::TreeViewItem > items;
	m_categoryTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (!items.empty())
	{
		Ref< ui::PreviewItems > previewItems = items[0]->getData< ui::PreviewItems >(L"ITEMS");
		m_typeList->setItems(previewItems);
	}
	else
		m_typeList->setItems(0);
}

void BrowseTypeDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	Ref< ui::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = 0;
}

void BrowseTypeDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	updatePreviewList();
}

void BrowseTypeDialog::eventListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = 0;

	endModal(ui::DialogResult::Ok);
}

	}
}
