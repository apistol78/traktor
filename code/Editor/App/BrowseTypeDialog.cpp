#include "Core/Misc/Split.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "I18N/Text.h"
#include "Ui/Bitmap.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseTypeDialog", BrowseTypeDialog, ui::ConfigDialog)

BrowseTypeDialog::BrowseTypeDialog(PropertyGroup* settings)
:	m_settings(settings)
,	m_type(0)
{
}

bool BrowseTypeDialog::create(ui::Widget* parent, const TypeInfo* base, bool onlyEditable, bool onlyInstantiable)
{
	TypeInfoSet types;
	if (base)
		base->findAllOf(types);
	else
		type_of< Object >().findAllOf(types, false);

	if (types.empty())
		return false;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"BROWSE_TYPE_TITLE"),
		640,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventDialogClick));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	if (!splitter->create(this, true, 200))
		return false;

	Ref< ui::Container > left = new ui::Container();
	if (!left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Static > treeLabel = new ui::Static();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_TYPE_CATEGORY")))
		return false;

	m_categoryTree = new ui::TreeView();
	if (!m_categoryTree->create(left))
		return false;
	m_categoryTree->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_categoryTree->addSelectEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventTreeItemSelected));

	Ref< ui::Container > right = new ui::Container();
	if (!right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(right, i18n::Text(L"BROWSE_TYPE_TYPES")))
		return false;

	m_typeList = new ui::custom::PreviewList();
	if (!m_typeList->create(right, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;
	m_typeList->addDoubleClickEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventListDoubleClick));

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"BROWSE_TYPE_GLOBAL"), 2, 3);
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
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< PropertyObject >(L"Editor.BrowseTypeTreeState"));
	if (state)
		m_categoryTree->applyState(state);

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

void BrowseTypeDialog::eventDialogClick(ui::Event* event)
{
	Ref< ui::custom::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;
		
	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = 0;
}

void BrowseTypeDialog::eventTreeItemSelected(ui::Event* event)
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

void BrowseTypeDialog::eventListDoubleClick(ui::Event* event)
{
	Ref< ui::custom::PreviewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;
		
	TypeInfoWrapper* typeInfoWrapper = item->getData< TypeInfoWrapper >(L"TYPE");
	if (typeInfoWrapper)
		m_type = &typeInfoWrapper->m_typeInfo;
	else
		m_type = 0;

	endModal(ui::DrOk);
}

	}
}
