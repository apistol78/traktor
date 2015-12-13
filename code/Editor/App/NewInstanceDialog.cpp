#include "Core/Misc/Split.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Editor/App/NewInstanceDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

// Resources
//#include "Resources/Files.h"

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
,	m_type(0)
{
}

bool NewInstanceDialog::create(ui::Widget* parent)
{
	TypeInfoSet types;
	type_of< ISerializable >().findAllOf(types);
	if (types.empty())
		return false;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"NEW_INSTANCE_TITLE"),
		ui::scaleBySystemDPI(640),
		ui::scaleBySystemDPI(500),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &NewInstanceDialog::eventDialogClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, ui::scaleBySystemDPI(200));

	Ref< ui::Container > left = new ui::Container();
	left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::Static > treeLabel = new ui::Static();
	treeLabel->create(left, i18n::Text(L"NEW_INSTANCE_CATEGORY"));

	m_categoryTree = new ui::custom::TreeView();
	m_categoryTree->create(left, ui::WsDoubleBuffer);
	//m_categoryTree->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_categoryTree->addEventHandler< ui::SelectionChangeEvent >(this, &NewInstanceDialog::eventTreeItemSelected);

	Ref< ui::Container > right = new ui::Container();
	right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::Static > listLabel = new ui::Static();
	listLabel->create(right, i18n::Text(L"NEW_INSTANCE_TYPES"));

	m_typeList = new ui::custom::PreviewList();
	if (!m_typeList->create(right, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;

	Ref< ui::Container > bottom = new ui::Container();
	bottom->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticInstanceName = new ui::Static();
	staticInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_NAME"));

	m_editInstanceName = new ui::Edit();
	m_editInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_DEFAULT_NAME"));

	Ref< ui::custom::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"NEW_INSTANCE_GLOBAL"), 2, 3);
	for (TypeInfoSet::iterator i = types.begin(); i != types.end(); ++i)
	{
		const TypeInfo* type = *i;
		T_ASSERT (type);

		if (!type->isEditable())
			continue;

		std::vector< std::wstring > parts;
		if (!Split< std::wstring >::any(type->getName(), L".", parts))
			continue;

		std::wstring className = parts.back(); parts.pop_back();

		Ref< ui::custom::TreeViewItem > group = groupRoot;
		for (std::vector< std::wstring >::iterator j = parts.begin(); j != parts.end(); ++j)
		{
			Ref< ui::custom::TreeViewItem > child = group->findChild(*j);
			if (!child)
			{
				child = m_categoryTree->createItem(group, *j, 2, 3);
				child->expand();
			}
			group = child;
		}

		Ref< ui::custom::PreviewItems > items = group->getData< ui::custom::PreviewItems >(L"ITEMS");
		if (!items)
		{
			items = new ui::custom::PreviewItems();
			group->setData(L"ITEMS", items);
		}

		Ref< ui::custom::PreviewItem > item = new ui::custom::PreviewItem(className);
		item->setData(L"TYPE", new TypeInfoWrapper(*type));
		
		items->add(item);
	}

	groupRoot->sort(true);

	// Expand all groups until a group with multiple children is found.
	ui::custom::TreeViewItem* expandGroup = groupRoot;
	while (expandGroup)
	{
		expandGroup->expand();

		const RefArray< ui::custom::TreeViewItem >& children = expandGroup->getChildren();
		if (children.size() == 1)
			expandGroup = children[0];
		else
			break;
	}

	m_categoryTree->update();

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< PropertyObject >(L"Editor.NewInstanceTreeState"));
	if (state)
		m_categoryTree->applyState(state);

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
	RefArray< ui::custom::TreeViewItem > items;
	
	m_categoryTree->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);
	if (!items.empty())
	{
		Ref< ui::custom::PreviewItems > previewItems = items[0]->getData< ui::custom::PreviewItems >(L"ITEMS");
		m_typeList->setItems(previewItems);
	}
	else
		m_typeList->setItems(0);
}

void NewInstanceDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	Ref< ui::custom::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = 0;

	m_instanceName = m_editInstanceName->getText();
}

void NewInstanceDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	updatePreviewList();
}

	}
}
