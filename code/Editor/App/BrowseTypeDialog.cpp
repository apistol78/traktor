#include "Core/Misc/Split.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/Settings.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "I18N/Text.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Static.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItems.h"
#include "Ui/ListViewItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/MiniButton.h"

// Resources
#include "Resources/Files.h"
#include "Resources/New.h"
#include "Resources/BigIcons.h"
#include "Resources/SmallIcons.h"

#pragma warning(disable: 4344)

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseTypeDialog", BrowseTypeDialog, ui::ConfigDialog)

BrowseTypeDialog::BrowseTypeDialog(Settings* settings)
:	m_settings(settings)
{
}

bool BrowseTypeDialog::create(ui::Widget* parent, const TypeInfo* base)
{
	std::vector< const TypeInfo* > types;
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

	Ref< ui::Container > rightTop = new ui::Container();
	if (!rightTop->create(right, ui::WsNone, new ui::TableLayout(L"100%,*,*", L"100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(rightTop, i18n::Text(L"BROWSE_TYPE_TYPES")))
		return false;

	m_buttonIcon = new ui::custom::MiniButton();
	if (!m_buttonIcon->create(rightTop, ui::Bitmap::load(c_ResourceBigIcons, sizeof(c_ResourceBigIcons), L"png")))
		return false;
	m_buttonIcon->addClickEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventButtonClick));

	m_buttonSmall = new ui::custom::MiniButton();
	if (!m_buttonSmall->create(rightTop, ui::Bitmap::load(c_ResourceSmallIcons, sizeof(c_ResourceSmallIcons), L"png")))
		return false;
	m_buttonSmall->addClickEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventButtonClick));

	int32_t iconSize = m_settings->getProperty< PropertyInteger >(L"Editor.BrowseType.IconSize", 0);

	m_typeList = new ui::ListView();
	if (!m_typeList->create(right, ui::WsClientBorder | (iconSize == 0 ? ui::ListView::WsIconNormal : ui::ListView::WsList)))
		return false;
	m_typeList->addDoubleClickEventHandler(ui::createMethodHandler(this, &BrowseTypeDialog::eventListDoubleClick));
	m_typeList->addImage(ui::Bitmap::load(c_ResourceNew, sizeof(c_ResourceNew), L"png"), 1);

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"BROWSE_TYPE_GLOBAL"), 2, 3);
	for (std::vector< const TypeInfo* >::iterator i = types.begin(); i != types.end(); ++i)
	{
		const TypeInfo* type = *i;

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

		Ref< ui::ListViewItems > items = group->getData< ui::ListViewItems >(L"ITEMS");
		if (!items)
		{
			items = new ui::ListViewItems();
			group->setData(L"ITEMS", items);
		}

		Ref< ui::ListViewItem > item = new ui::ListViewItem();
		item->setImage(0, 0);
		item->setText(0, className);
		item->setText(1, type->getName());
		
		items->add(item);
	}

	groupRoot->expand();
	m_categoryTree->update();

	return true;
}

const TypeInfo* BrowseTypeDialog::getSelectedType() const
{
	return TypeInfo::find(m_typeName);
}

void BrowseTypeDialog::eventDialogClick(ui::Event* event)
{
	Ref< ui::ListViewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	m_typeName = item->getText(1);
}

void BrowseTypeDialog::eventTreeItemSelected(ui::Event* event)
{
	Ref< ui::TreeViewItem > item = dynamic_type_cast< ui::TreeViewItem* >(
		static_cast< ui::CommandEvent* >(event)->getItem()
	);
	if (item)
	{
		Ref< ui::ListViewItems > items = item->getData< ui::ListViewItems >(L"ITEMS");
		m_typeList->setItems(items);
	}
	else
		m_typeList->setItems(0);
}

void BrowseTypeDialog::eventListDoubleClick(ui::Event* event)
{
	if (m_typeList->getSelectedItem())
	{
		m_typeName = m_typeList->getSelectedItem()->getText(1);
		endModal(ui::DrOk);
	}
}

void BrowseTypeDialog::eventButtonClick(ui::Event* event)
{
	if (event->getSender() == m_buttonIcon)
	{
		m_typeList->setStyle(ui::ListView::WsIconNormal | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.BrowseType.IconSize", 0);
	}
	else
	{
		m_typeList->setStyle(ui::ListView::WsList | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.BrowseType.IconSize", 1);
	}
}

	}
}
