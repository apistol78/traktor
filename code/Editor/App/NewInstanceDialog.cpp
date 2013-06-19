#include "Core/Misc/Split.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Editor/App/NewInstanceDialog.h"
#include "I18N/Text.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/HierarchicalState.h"
#include "Ui/MethodHandler.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Events/CommandEvent.h"

// Resources
#include "Resources/Files.h"

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
		640,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventDialogClick));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 200);

	Ref< ui::Container > left = new ui::Container();
	left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0));

	Ref< ui::Static > treeLabel = new ui::Static();
	treeLabel->create(left, i18n::Text(L"NEW_INSTANCE_CATEGORY"));

	m_categoryTree = new ui::TreeView();
	m_categoryTree->create(left);
	m_categoryTree->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_categoryTree->addSelectEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventTreeItemSelected));

	Ref< ui::Container > right = new ui::Container();
	right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0));

	Ref< ui::Static > listLabel = new ui::Static();
	listLabel->create(right, i18n::Text(L"NEW_INSTANCE_TYPES"));

	m_typeList = new ui::custom::PreviewList();
	if (!m_typeList->create(right, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;

	Ref< ui::Container > bottom = new ui::Container();
	bottom->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticInstanceName = new ui::Static();
	staticInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_NAME"));

	m_editInstanceName = new ui::Edit();
	m_editInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_DEFAULT_NAME"));

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"NEW_INSTANCE_GLOBAL"), 2, 3);
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

		Ref< ui::TreeViewItem > group = groupRoot;
		for (std::vector< std::wstring >::iterator j = parts.begin(); j != parts.end(); ++j)
		{
			Ref< ui::TreeViewItem > child = group->findChild(*j);
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

	groupRoot->expand();
	m_categoryTree->update();

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< PropertyObject >(L"Editor.NewInstanceTreeState"));
	if (state)
		m_categoryTree->applyState(state);

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

void NewInstanceDialog::eventDialogClick(ui::Event* event)
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

void NewInstanceDialog::eventTreeItemSelected(ui::Event* event)
{
	Ref< ui::TreeViewItem > item = dynamic_type_cast< ui::TreeViewItem* >(
		static_cast< ui::CommandEvent* >(event)->getItem()
	);
	if (item)
	{
		Ref< ui::custom::PreviewItems > items = item->getData< ui::custom::PreviewItems >(L"ITEMS");
		m_typeList->setItems(items);
	}
	else
		m_typeList->setItems(0);
}

	}
}
