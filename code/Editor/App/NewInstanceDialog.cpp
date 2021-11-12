#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Editor/App/NewInstanceDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.NewInstanceDialog", NewInstanceDialog, ui::ConfigDialog)

NewInstanceDialog::NewInstanceDialog(PropertyGroup* settings)
:	m_settings(settings)
,	m_type(nullptr)
{
}

bool NewInstanceDialog::create(ui::Widget* parent, const std::wstring& initialGroup)
{
	TypeInfoSet types;
	type_of< ISerializable >().findAllOf(types);
	if (types.empty())
		return false;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"NEW_INSTANCE_TITLE"),
		ui::dpi96(640),
		ui::dpi96(500),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &NewInstanceDialog::eventDialogClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, ui::dpi96(200));

	Ref< ui::Container > left = new ui::Container();
	left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::Static > treeLabel = new ui::Static();
	treeLabel->create(left, i18n::Text(L"NEW_INSTANCE_CATEGORY"));

	m_categoryTree = new ui::TreeView();
	m_categoryTree->create(left, ui::WsDoubleBuffer);
	m_categoryTree->addImage(new ui::StyleBitmap(L"Editor.Database.Folders"), 2);
	m_categoryTree->addEventHandler< ui::SelectionChangeEvent >(this, &NewInstanceDialog::eventTreeItemSelected);

	Ref< ui::Container > right = new ui::Container();
	right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::Static > listLabel = new ui::Static();
	listLabel->create(right, i18n::Text(L"NEW_INSTANCE_TYPES"));

	m_typeList = new ui::PreviewList();
	if (!m_typeList->create(right, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;

	Ref< ui::Container > bottom = new ui::Container();
	bottom->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticInstanceName = new ui::Static();
	staticInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_NAME"));

	m_editInstanceName = new ui::Edit();
	m_editInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_DEFAULT_NAME"));

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"NEW_INSTANCE_GLOBAL"), 1);
	groupRoot->setImage(0, 0, 1);

	Ref< ui::TreeViewItem > selectGroup;

	for (auto type : types)
	{
		if (!type->isEditable())
			continue;

		std::vector< std::wstring > parts;
		if (!Split< std::wstring >::any(type->getName(), L".", parts))
			continue;

		std::wstring className = parts.back();
		parts.pop_back();

		Ref< ui::TreeViewItem > group = groupRoot;
		for (auto part : parts)
		{
			Ref< ui::TreeViewItem > child = group->findChild(part);
			if (!child)
			{
				child = m_categoryTree->createItem(group, part, 1);
				child->setImage(0, 0, 1);
				child->expand();
			}
			group = child;
		}

		if (!initialGroup.empty())
		{
			StringOutputStream ss;
			for (size_t i = 0; i < parts.size(); ++i)
			{
				if (i > 0)
					ss << L".";
				ss << parts[i];
			}
			if (initialGroup == ss.str())
				selectGroup = group;
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
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< Ref< ISerializable > >(L"Editor.NewInstanceTreeState"));
	if (state)
		m_categoryTree->applyState(state);

	// Select initial group if found.
	if (selectGroup)
	{
		selectGroup->select();
		selectGroup->show();
	}

	updatePreviewList();
	return true;
}

void NewInstanceDialog::destroy()
{
	if (m_settings)
	{
		Ref< ui::HierarchicalState > state = m_categoryTree->captureState();
		m_settings->setProperty< PropertyObject >(L"Editor.NewInstanceTreeState", state);
	}

	ui::ConfigDialog::destroy();
}

const TypeInfo* NewInstanceDialog::getType() const
{
	return m_type;
}

const std::wstring& NewInstanceDialog::getInstanceName() const
{
	return m_instanceName;
}

void NewInstanceDialog::updatePreviewList()
{
	RefArray< ui::TreeViewItem > items;
	m_categoryTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (!items.empty())
	{
		Ref< ui::PreviewItems > previewItems = items[0]->getData< ui::PreviewItems >(L"ITEMS");
		m_typeList->setItems(previewItems);
	}
	else
		m_typeList->setItems(nullptr);
}

void NewInstanceDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	Ref< ui::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = nullptr;

	m_instanceName = m_editInstanceName->getText();
}

void NewInstanceDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	updatePreviewList();
}

	}
}
