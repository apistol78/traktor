#include "Editor/App/NewInstanceDialog.h"
#include "Editor/Settings.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Static.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItem.h"
#include "Ui/ListViewItems.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/MiniButton.h"
#include "I18N/Text.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/Split.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.NewInstanceDialog", NewInstanceDialog, ui::ConfigDialog)

NewInstanceDialog::NewInstanceDialog(Settings* settings)
:	m_settings(settings)
{
}

bool NewInstanceDialog::create(ui::Widget* parent)
{
	std::vector< const Type* > types;
	type_of< Serializable >().findAllOf(types);
	if (types.empty())
		return false;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"NEW_INSTANCE_TITLE"),
		640,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		gc_new< ui::TableLayout >(L"100%", L"100%,*", 4, 4)
	))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventDialogClick));

	Ref< ui::custom::Splitter > splitter = gc_new< ui::custom::Splitter >();
	splitter->create(this, true, 200);

	Ref< ui::Container > left = gc_new< ui::Container >();
	left->create(splitter, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"22,100%", 0, 0));

	Ref< ui::Static > treeLabel = gc_new< ui::Static >();
	treeLabel->create(left, i18n::Text(L"NEW_INSTANCE_CATEGORY"));

	m_categoryTree = gc_new< ui::TreeView >();
	m_categoryTree->create(left);
	m_categoryTree->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_categoryTree->addSelectEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventTreeItemSelected));

	Ref< ui::Container > right = gc_new< ui::Container >();
	right->create(splitter, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"22,100%", 0, 0));

	Ref< ui::Container > rightTop = gc_new< ui::Container >();
	rightTop->create(right, ui::WsNone, gc_new< ui::TableLayout >(L"100%,*,*", L"100%", 0, 0));

	Ref< ui::Static > listLabel = gc_new< ui::Static >();
	listLabel->create(rightTop, i18n::Text(L"NEW_INSTANCE_TYPES"));

	m_buttonIcon = gc_new< ui::custom::MiniButton >();
	m_buttonIcon->create(rightTop, ui::Bitmap::load(c_ResourceBigIcons, sizeof(c_ResourceBigIcons), L"png"));
	m_buttonIcon->addClickEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventButtonClick));

	m_buttonSmall = gc_new< ui::custom::MiniButton >();
	m_buttonSmall->create(rightTop, ui::Bitmap::load(c_ResourceSmallIcons, sizeof(c_ResourceSmallIcons), L"png"));
	m_buttonSmall->addClickEventHandler(ui::createMethodHandler(this, &NewInstanceDialog::eventButtonClick));

	int32_t iconSize = m_settings->getProperty< PropertyInteger >(L"Editor.NewInstance.IconSize", 0);

	m_typeList = gc_new< ui::ListView >();
	m_typeList->create(right, ui::WsClientBorder | (iconSize == 0 ? ui::ListView::WsIconNormal : ui::ListView::WsList));
	m_typeList->addImage(ui::Bitmap::load(c_ResourceNew, sizeof(c_ResourceNew), L"png"), 1);

	Ref< ui::Container > bottom = gc_new< ui::Container >();
	bottom->create(this, ui::WsNone, gc_new< ui::TableLayout >(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticInstanceName = gc_new< ui::Static >();
	staticInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_NAME"));

	m_editInstanceName = gc_new< ui::Edit >();
	m_editInstanceName->create(bottom, i18n::Text(L"NEW_INSTANCE_DEFAULT_NAME"));

	Ref< ui::TreeViewItem > groupRoot = m_categoryTree->createItem(0, i18n::Text(L"NEW_INSTANCE_GLOBAL"), 2, 3);
	for (std::vector< const Type* >::iterator i = types.begin(); i != types.end(); ++i)
	{
		const Type* type = *i;
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

		Ref< ui::ListViewItems > items = group->getData< ui::ListViewItems >(L"ITEMS");
		if (!items)
		{
			items = gc_new< ui::ListViewItems >();
			group->setData(L"ITEMS", items);
		}

		Ref< ui::ListViewItem > item = gc_new< ui::ListViewItem >();
		item->setImage(0, 0);
		item->setText(0, className);
		item->setText(1, type->getName());
		
		items->add(item);
	}

	groupRoot->expand();
	m_categoryTree->update();

	return true;
}

const std::wstring& NewInstanceDialog::getTypeName() const
{
	return m_typeName;
}

const std::wstring& NewInstanceDialog::getInstanceName() const
{
	return m_instanceName;
}

void NewInstanceDialog::eventDialogClick(ui::Event* event)
{
	Ref< ui::ListViewItem > item = m_typeList->getSelectedItem();
	if (!item)
		return;

	m_typeName = item->getText(1);
	m_instanceName = m_editInstanceName->getText();
}

void NewInstanceDialog::eventTreeItemSelected(ui::Event* event)
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

void NewInstanceDialog::eventButtonClick(ui::Event* event)
{
	if (event->getSender() == m_buttonIcon)
	{
		m_typeList->setStyle(ui::ListView::WsIconNormal | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.NewInstance.IconSize", 0);
	}
	else
	{
		m_typeList->setStyle(ui::ListView::WsList | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.NewInstance.IconSize", 1);
	}
}

	}
}
