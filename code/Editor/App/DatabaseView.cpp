#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/Asset.h"
#include "Editor/Assets.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/IWizardTool.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/InstanceClipboardData.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/PipelineDependency.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/DatabaseView.h"
#include "Resources/Types.h"
#include "Resources/TypesHidden.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView", DatabaseView, ui::Container)

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView.Filter", DatabaseView::Filter, Object)

		namespace
		{

class DefaultFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS;

public:
	virtual bool acceptInstance(const db::Instance* instance) const
	{
		return is_type_of< ISerializable >(*instance->getPrimaryType());
	}

	virtual bool acceptEmptyGroups() const
	{
		return true;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView.DefaultFilter", DefaultFilter, DatabaseView::Filter)

class TextFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS;

public:
	TextFilter(const std::wstring& filter)
	:	m_filter(filter)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const
	{
		return m_filter.match(instance->getName());
	}

	virtual bool acceptEmptyGroups() const
	{
		return false;
	}

private:
	WildCompare m_filter;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView.TextFilter", TextFilter, DatabaseView::Filter)

class TypeSetFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS;

public:
	TypeSetFilter(const TypeInfoSet& typeSet)
	:	m_typeSet(typeSet)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const
	{
		return m_typeSet.find(instance->getPrimaryType()) != m_typeSet.end();
	}

	virtual bool acceptEmptyGroups() const
	{
		return false;
	}

private:
	mutable TypeInfoSet m_typeSet;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView.TypeSetFilter", TypeSetFilter, DatabaseView::Filter)

class GuidSetFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS;

public:
	GuidSetFilter(const std::set< Guid >& guidSet)
	:	m_guidSet(guidSet)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const
	{
		return m_guidSet.find(instance->getGuid()) != m_guidSet.end();
	}

	virtual bool acceptEmptyGroups() const
	{
		return false;
	}

private:
	mutable std::set< Guid > m_guidSet;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView.GuidSetFilter", GuidSetFilter, DatabaseView::Filter)

class CollectInstanceTypes
{
public:
	CollectInstanceTypes(TypeInfoSet& outInstanceTypes)
	:	m_outInstanceTypes(outInstanceTypes)
	{
	}

	bool operator () (const db::Instance* instance) const
	{
		const TypeInfo* instanceType = instance->getPrimaryType();
		if (instanceType)
				m_outInstanceTypes.insert(instanceType);
		return false;
	}

private:
	TypeInfoSet& m_outInstanceTypes;
};

ui::TreeViewItem* findTreeItem(ui::TreeViewItem* item, const db::Instance* instance)
{
	if (!item)
		return 0;

	if (item->getData< db::Instance >(L"INSTANCE") == instance)
		return item;

	RefArray< ui::TreeViewItem > children;
	item->getChildren(children);

	for (RefArray< ui::TreeViewItem >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if ((item = findTreeItem(*i, instance)) != 0)
			return item;
	}

	return 0;
}

bool isInstanceInPrivate(const db::Instance* instance)
{
	db::Group* group = instance->getParent();
	while (group)
	{
		if (group->getName() == L"System")
			return true;
		group = group->getParent();
	}
	return false;
}

std::wstring getCategoryText(const TypeInfo* categoryType)
{
	std::wstring id = L"DATABASE_CATEGORY_" + replaceAll< std::wstring >(toUpper(std::wstring(categoryType->getName())), L".", L"_");
	return i18n::Text(id, categoryType->getName());
}

		}

DatabaseView::DatabaseView(IEditor* editor)
:	m_editor(editor)
,	m_filter(new DefaultFilter())
{
}

bool DatabaseView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolSelection = new ui::custom::ToolBar();
	if (!m_toolSelection->create(this))
		return false;
	m_toolSelection->addImage(ui::Bitmap::load(c_ResourceDatabaseView, sizeof(c_ResourceDatabaseView), L"png"), 4);

	m_toolFilterType = new ui::custom::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER"),
		0,
		ui::Command(L"Database.Filter"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterType);

	m_toolFilterAssets = new ui::custom::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_ASSETS"),
		1,
		ui::Command(L"Database.FilterAssets"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterAssets);

	m_toolFilterShow = new ui::custom::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_SHOW_FILTERED"),
		2,
		ui::Command(L"Database.ShowFiltered"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterShow);

	m_toolFavoritesShow = new ui::custom::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_SHOW_FAVORITES"),
		3,
		ui::Command(L"Database.ShowFavorites"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFavoritesShow);

	m_toolSelection->addItem(new ui::custom::ToolBarSeparator());

	m_editFilter = new ui::Edit();
	m_editFilter->create(m_toolSelection, L"", ui::WsNone);
	m_editFilter->addEventHandler< ui::KeyUpEvent >(this, &DatabaseView::eventFilterKey);
	m_toolSelection->addItem(new ui::custom::ToolBarEmbed(m_editFilter, 100));

	m_toolSelection->addItem(new ui::custom::ToolBarSeparator());

	m_toolViewMode = new ui::custom::ToolBarDropDown(ui::Command(L"Editor.ViewModes"), 80, i18n::Text(L"DATABASE_VIEW_MODE"));
	m_toolViewMode->add(i18n::Text(L"DATABASE_VIEW_MODE_HIERARCHY"));
	m_toolViewMode->add(i18n::Text(L"DATABASE_VIEW_MODE_CATEGORY"));
	m_toolViewMode->select(0);
	m_toolSelection->addItem(m_toolViewMode);

	m_toolSelection->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &DatabaseView::eventToolSelectionClicked);

	m_treeDatabase = new ui::TreeView();
	if (!m_treeDatabase->create(this, (ui::TreeView::WsDefault | ui::TreeView::WsDrag) & ~ui::WsClientBorder))
		return false;
	m_treeDatabase->addImage(ui::Bitmap::load(c_ResourceTypes, sizeof(c_ResourceTypes), L"png"), 23);
	m_treeDatabase->addImage(ui::Bitmap::load(c_ResourceTypesHidden, sizeof(c_ResourceTypesHidden), L"png"), 23);
	m_treeDatabase->addEventHandler< ui::TreeViewItemActivateEvent >(this, &DatabaseView::eventInstanceActivate);
	m_treeDatabase->addEventHandler< ui::MouseButtonDownEvent >(this, &DatabaseView::eventInstanceButtonDown);
	m_treeDatabase->addEventHandler< ui::TreeViewContentChangeEvent >(this, &DatabaseView::eventInstanceRenamed);
	m_treeDatabase->addEventHandler< ui::TreeViewDragEvent >(this, &DatabaseView::eventInstanceDrag);
	m_treeDatabase->setEnable(false);
		
	m_menuGroup[0] = new ui::PopupMenu();
	m_menuGroup[1] = new ui::PopupMenu();
	if (!m_menuGroup[0]->create() || !m_menuGroup[1]->create())
		return false;
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewInstance"), i18n::Text(L"DATABASE_NEW_INSTANCE")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewGroup"), i18n::Text(L"DATABASE_NEW_GROUP")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuGroup[0]->add(new ui::MenuItem(L"-"));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.FavoriteEntireGroup"), i18n::Text(L"DATABASE_FAVORITE_ENTIRE_GROUP")));
	m_menuGroup[0]->add(new ui::MenuItem(L"-"));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"DATABASE_PASTE")));

	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewInstance"), i18n::Text(L"DATABASE_NEW_INSTANCE")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewGroup"), i18n::Text(L"DATABASE_NEW_GROUP")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuGroup[1]->add(new ui::MenuItem(L"-"));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.UnFavoriteEntireGroup"), i18n::Text(L"DATABASE_UNFAVORITE_ENTIRE_GROUP")));
	m_menuGroup[1]->add(new ui::MenuItem(L"-"));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"DATABASE_PASTE")));

	m_menuInstance = new ui::PopupMenu();
	if (!m_menuInstance->create())
		return false;
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ReplaceInstance"), i18n::Text(L"DATABASE_REPLACE_INSTANCE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.DefaultEditInstance"), i18n::Text(L"DATABASE_DEFAULT_EDIT_INSTANCE")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"DATABASE_COPY")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleRoot"), i18n::Text(L"DATABASE_TOGGLE_AS_ROOT")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleFavorite"), i18n::Text(L"DATABASE_TOGGLE_AS_FAVORITE")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	m_menuInstanceAsset = new ui::PopupMenu();
	if (!m_menuInstanceAsset->create())
		return false;
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Edit"), i18n::Text(L"DATABASE_EDIT")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Explore"), i18n::Text(L"DATABASE_EXPLORE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ReplaceInstance"), i18n::Text(L"DATABASE_REPLACE_INSTANCE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.DefaultEditInstance"), i18n::Text(L"DATABASE_DEFAULT_EDIT_INSTANCE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"DATABASE_COPY")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleRoot"), i18n::Text(L"DATABASE_TOGGLE_AS_ROOT")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleFavorite"), i18n::Text(L"DATABASE_TOGGLE_AS_FAVORITE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	TypeInfoSet wizardToolTypes;
	type_of< IWizardTool >().findAllOf(wizardToolTypes);

	if (!wizardToolTypes.empty())
	{
		Ref< ui::MenuItem > menuGroupWizards = new ui::MenuItem(i18n::Text(L"DATABASE_WIZARDS"));
		Ref< ui::MenuItem > menuInstanceWizards = new ui::MenuItem(i18n::Text(L"DATABASE_WIZARDS"));

		int32_t nextWizardId = 0;
		for (TypeInfoSet::iterator i = wizardToolTypes.begin(); i != wizardToolTypes.end(); ++i)
		{
			Ref< IWizardTool > wizard = dynamic_type_cast< IWizardTool* >((*i)->createInstance());
			if (!wizard)
				continue;

			std::wstring wizardDescription = wizard->getDescription();
			T_ASSERT (!wizardDescription.empty());

			int32_t wizardId = nextWizardId++;

			if ((wizard->getFlags() & IWizardTool::WfGroup) != 0)
				menuGroupWizards->add(new ui::MenuItem(ui::Command(wizardId, L"Editor.Database.Wizard"), wizardDescription));
			if ((wizard->getFlags() & IWizardTool::WfInstance) != 0)
				menuInstanceWizards->add(new ui::MenuItem(ui::Command(wizardId, L"Editor.Database.Wizard"), wizardDescription));

			m_wizardTools.push_back(wizard);
		}

		m_menuGroup[0]->add(menuGroupWizards);
		m_menuGroup[1]->add(menuGroupWizards);
		m_menuInstance->add(menuInstanceWizards);
		m_menuInstanceAsset->add(menuInstanceWizards);
	}

	addEventHandler< ui::TimerEvent >(this, &DatabaseView::eventTimer);

	setEnable(false);
	return true;
}

void DatabaseView::destroy()
{
	safeDestroy(m_menuGroup[1]);
	safeDestroy(m_menuGroup[0]);
	safeDestroy(m_menuInstance);
	safeDestroy(m_menuInstanceAsset);
	ui::Container::destroy();
}

void DatabaseView::setDatabase(db::Database* db)
{
	m_db = db;
	updateView();
}

void DatabaseView::updateView()
{
	Ref< ui::HierarchicalState > treeState = m_treeDatabase->captureState();
	m_treeState = m_treeState ? m_treeState->merge(treeState) : treeState;

	m_treeDatabase->removeAllItems();

	if (m_db)
	{
		m_rootInstances.clear();
		m_favoriteInstances.clear();

		std::vector< std::wstring > rootInstances = m_editor->getSettings()->getProperty< PropertyStringArray >(L"Editor.RootInstances");
		for (std::vector< std::wstring >::const_iterator i = rootInstances.begin(); i != rootInstances.end(); ++i)
			m_rootInstances.insert(Guid(*i));

		std::vector< std::wstring > favoriteInstances = m_editor->getSettings()->getProperty< PropertyStringArray >(L"Editor.FavoriteInstances");
		for (std::vector< std::wstring >::const_iterator i = favoriteInstances.begin(); i != favoriteInstances.end(); ++i)
			m_favoriteInstances.insert(Guid(*i));

		int32_t viewMode = m_toolViewMode->getSelected();

		if (viewMode == 0)	// Hierarchy
			buildTreeItem(m_treeDatabase, 0, m_db->getRootGroup());
		else if (viewMode == 1)	// Category
		{
			bool showFiltered = m_toolFilterShow->isToggled();
			bool showFavorites = m_toolFavoritesShow->isToggled();
			bool showPrivate = false;

			TypeInfoSet instanceTypes;
			db::recursiveFindChildInstance(m_db->getRootGroup(), CollectInstanceTypes(instanceTypes));

			for (TypeInfoSet::const_iterator i = instanceTypes.begin(); i != instanceTypes.end(); ++i)
			{
				const TypeInfo* instanceType = *i;
				T_ASSERT (instanceType);

				Ref< ui::TreeViewItem > instanceTypeItem = m_treeDatabase->createItem(0, getCategoryText(instanceType), 0, 1);

				RefArray< db::Instance > instances;
				db::recursiveFindChildInstances(m_db->getRootGroup(), db::FindInstanceByType(*instanceType), instances);

				for (RefArray< db::Instance >::const_iterator j = instances.begin(); j != instances.end(); ++j)
				{
					const TypeInfo* primaryType = (*j)->getPrimaryType();
					if (!primaryType)
						continue;

					if (showFavorites)
					{
						if (m_favoriteInstances.find((*j)->getGuid()) == m_favoriteInstances.end())
							continue;
					}

					if (!showPrivate)
					{
						if (isInstanceInPrivate(*j))
							continue;
					}

					int32_t iconIndex = getIconIndex(primaryType);

					if (!showFiltered)
					{
						if (!m_filter->acceptInstance((*j)))
							continue;
					}
					else
					{
						if (!m_filter->acceptInstance((*j)))
							iconIndex += 23;
					}

					Ref< ui::TreeViewItem > instanceItem = m_treeDatabase->createItem(
						instanceTypeItem,
						(*j)->getName(),
						iconIndex
					);

					if (m_rootInstances.find((*j)->getGuid()) != m_rootInstances.end())
						instanceItem->setBold(true);

					instanceItem->setData(L"GROUP", (*j)->getParent());
					instanceItem->setData(L"INSTANCE", (*j));
				}

				if (!instanceTypeItem->hasChildren())
					m_treeDatabase->removeItem(instanceTypeItem);
			}
		}

		setEnable(true);
	}
	else
		setEnable(false);

	m_treeDatabase->applyState(m_treeState);
	m_treeDatabase->update();
}

bool DatabaseView::highlight(const db::Instance* instance)
{
	ui::TreeViewItem* item = findTreeItem(m_treeDatabase->getRootItem(), instance);
	if (!item)
		return false;

	item->show();
	item->select();

	return true;
}

bool DatabaseView::handleCommand(const ui::Command& command)
{
	Ref< ui::TreeViewItem > treeItem = m_treeDatabase->getSelectedItem();
	if (!treeItem)
		return false;

	Ref< db::Group > group = treeItem->getData< db::Group >(L"GROUP");
	Ref< db::Instance > instance = treeItem->getData< db::Instance >(L"INSTANCE");

	if (group && instance)
	{
		if (command == L"Editor.Database.Edit")	// Edit
		{
			Ref< Asset > editAsset = instance->getObject< Asset >();
			if (editAsset)
				OS::getInstance().editFile(editAsset->getFileName().getPathName());
		}
		else if (command == L"Editor.Database.Explore")	// Explore
		{
			Ref< Asset > exploreAsset = instance->getObject< Asset >();
			if (exploreAsset)
				OS::getInstance().exploreFile(exploreAsset->getFileName().getPathName());
		}
		else if (command == L"Editor.Database.ReplaceInstance")	// Replace instance
		{
			BrowseTypeDialog browseTypeDlg(m_editor->checkoutGlobalSettings());
			browseTypeDlg.create(this, &type_of< ISerializable >(), true, true);

			if (browseTypeDlg.showModal() == ui::DrOk)
			{
				const TypeInfo* type = browseTypeDlg.getSelectedType();
				T_ASSERT (type);

				Ref< ISerializable > data = dynamic_type_cast< ISerializable* >(type->createInstance());
				T_ASSERT (data);

				if (instance->checkout())
				{
					instance->setObject(data);
					if (instance->commit())
					{
						// Type might have changed; ensure icon is updated.
						int32_t iconIndex = getIconIndex(type);
						treeItem->setImage(iconIndex);
						m_treeDatabase->update();
					}
					else
						log::error << L"Unable to commit instance" << Endl;
				}
				else
					log::error << L"Unable to checkout instance" << Endl;
			}

			browseTypeDlg.destroy();
		}
		else if (command == L"Editor.Database.Rename")	// Rename
		{
			treeItem->edit();
		}
		else if (command == L"Editor.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_INSTANCE"), ui::MbYesNo | ui::MbIconQuestion) != 1)
				return false;

			if (!instance->checkout())
				return false;

			if (!instance->remove())
				return false;

			if (!instance->commit())
				return false;

			m_treeDatabase->removeItem(treeItem);
			m_treeDatabase->update();
		}
		else if (command == L"Editor.Database.Clone")	// Clone
		{
			Ref< ISerializable > object = instance->getObject< ISerializable >();
			if (!object)
			{
				log::error << L"Unable to checkout instance" << Endl;
				return false;
			}

			object = DeepClone(object).create();
			if (!object)
			{
				log::error << L"Unable to create clone" << Endl;
				return false;
			}

			Ref< db::Instance > instanceClone = group->createInstance(instance->getName() + L" (clone)");
			if (!instanceClone)
			{
				log::error << L"Unable to create clone instance" << Endl;
				return false;
			}

			instanceClone->setObject(object);

			if (!instanceClone->commit())
			{
				log::error << L"Unable to commit clone instance" << Endl;
				return false;
			}

			Ref< ui::TreeViewItem > treeCloneItem = m_treeDatabase->createItem(treeItem->getParent(), instanceClone->getName(), treeItem->getImage());
			treeCloneItem->setData(L"GROUP", group);
			treeCloneItem->setData(L"INSTANCE", instanceClone);

			m_treeDatabase->update();
		}
		else if (command == L"Editor.Database.DefaultEditInstance")	// Default edit instance
		{
			m_editor->openDefaultEditor(instance);
		}
		else if (command == L"Editor.Copy")		// Copy instance
		{
			Ref< ISerializable > object = instance->getObject< ISerializable >();
			if (!object)
			{
				log::error << L"Unable to checkout instance" << Endl;
				return false;
			}

			object = DeepClone(object).create();
			if (!object)
			{
				log::error << L"Unable to create clone" << Endl;
				return false;
			}

			Ref< InstanceClipboardData > instanceClipboardData = new InstanceClipboardData(instance->getName(), object);
			ui::Application::getInstance()->getClipboard()->setObject(instanceClipboardData);
		}
		else if (command == L"Editor.Database.FilterInstanceType")	// Filter on type.
		{
			filterType(instance);
		}
		else if (command == L"Editor.Database.FilterInstanceDepends")	// Filter on dependencies
		{
			filterDependencies(instance);
		}
		else if (command == L"Editor.Database.ToggleRoot")	// Toggle root flag.
		{
			Guid instanceGuid = instance->getGuid();

			std::set< Guid >::iterator i = m_rootInstances.find(instanceGuid);
			if (i == m_rootInstances.end())
				m_rootInstances.insert(instanceGuid);
			else
				m_rootInstances.erase(i);

			std::vector< std::wstring > rootInstances;
			for (std::set< Guid >::iterator i = m_rootInstances.begin(); i != m_rootInstances.end(); ++i)
				rootInstances.push_back(i->format());

			Ref< PropertyGroup > workspaceSettings = m_editor->checkoutWorkspaceSettings();
			workspaceSettings->setProperty< PropertyStringArray >(L"Editor.RootInstances", rootInstances);
			m_editor->commitWorkspaceSettings();

			updateView();
		}
		else if (command == L"Editor.Database.ToggleFavorite")	// Toggle favorite flag.
		{
			Guid instanceGuid = instance->getGuid();

			std::set< Guid >::iterator i = m_favoriteInstances.find(instanceGuid);
			if (i == m_favoriteInstances.end())
				m_favoriteInstances.insert(instanceGuid);
			else
				m_favoriteInstances.erase(i);

			std::vector< std::wstring > favoriteInstances;
			for (std::set< Guid >::iterator i = m_favoriteInstances.begin(); i != m_favoriteInstances.end(); ++i)
				favoriteInstances.push_back(i->format());

			Ref< PropertyGroup > globalSettings = m_editor->checkoutGlobalSettings();
			globalSettings->setProperty< PropertyStringArray >(L"Editor.FavoriteInstances", favoriteInstances);
			m_editor->commitGlobalSettings();

			updateView();
		}
		else if (command == L"Editor.Database.Build")	// Build asset
		{
			m_editor->buildAsset(instance->getGuid(), false);
		}
		else if (command == L"Editor.Database.Rebuild")	// Rebuild asset
		{
			m_editor->buildAsset(instance->getGuid(), true);
		}
		else if (command == L"Editor.Database.Wizard")
		{
			Ref< IWizardTool > wizard = m_wizardTools[command.getId()];
			if (wizard->launch(this, m_editor, group, instance))
				updateView();
		}
		else
			return false;
	}
	else if (group)
	{
		if (command == L"Editor.Database.NewInstance")	// New instance...
		{
			NewInstanceDialog newInstanceDlg(m_editor->checkoutGlobalSettings());
			newInstanceDlg.create(this);

			if (newInstanceDlg.showModal() == ui::DrOk)
			{
				const TypeInfo* type = newInstanceDlg.getType();
				T_ASSERT (type);

				std::wstring instanceName = newInstanceDlg.getInstanceName();
				T_ASSERT (!instanceName.empty());

				Ref< ISerializable > data = dynamic_type_cast< ISerializable* >(type->createInstance());
				T_ASSERT (data);

				Ref< db::Instance > instance = group->createInstance(instanceName);
				if (instance)
				{
					instance->setObject(data);
					if (instance->commit())
					{
						int32_t iconIndex = getIconIndex(type);

						Ref< ui::TreeViewItem > instanceItem = m_treeDatabase->createItem(treeItem, instanceName, iconIndex);
						instanceItem->setData(L"GROUP", group);
						instanceItem->setData(L"INSTANCE", instance);

						m_treeDatabase->update();
					}
				}
			}

			newInstanceDlg.destroy();
		}
		else if (command == L"Editor.Database.NewGroup")	// New group...
		{
			Ref< db::Group > newGroup = group->createGroup(i18n::Text(L"DATABASE_NEW_GROUP_UNNAMED"));
			if (newGroup)
			{
				Ref< ui::TreeViewItem > groupItem = m_treeDatabase->createItem(treeItem, i18n::Text(L"DATABASE_NEW_GROUP_UNNAMED"), 0, 1);
				groupItem->setData(L"GROUP", newGroup);

				m_treeDatabase->update();

				// Enter edit mode directly as user probably don't want to call
				// the group "Unnamed".
				groupItem->edit();
			}
		}
		else if (command == L"Editor.Database.Rename")	// Rename
		{
			treeItem->edit();
		}
		else if (command == L"Editor.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_GROUP"), ui::MbYesNo | ui::MbIconQuestion) != 1)
				return false;

			if (!group->remove())
				return false;

			m_treeDatabase->removeItem(treeItem);
			m_treeDatabase->update();
		}
		else if (command == L"Editor.Paste")	// Paste instance into group
		{
			Ref< InstanceClipboardData > instanceClipboardData = dynamic_type_cast< InstanceClipboardData* >(
				ui::Application::getInstance()->getClipboard()->getObject()
			);
			if (!instanceClipboardData || !instanceClipboardData->getObject())
				return false;

			if (group->getInstance(instanceClipboardData->getName()) != 0)
			{
				log::error << L"Instance named \"" << instanceClipboardData->getName() << L"\" already exist in selected group" << Endl;
				return false;
			}

			Ref< db::Instance > instanceCopy = group->createInstance(instanceClipboardData->getName());
			if (!instanceCopy)
			{
				log::error << L"Unable to create instance copy" << Endl;
				return false;
			}

			instanceCopy->setObject(instanceClipboardData->getObject());

			if (!instanceCopy->commit())
			{
				log::error << L"Unable to commit instance copy" << Endl;
				return false;
			}

			int32_t iconIndex = getIconIndex(&type_of(instanceClipboardData->getObject()));

			Ref< ui::TreeViewItem > treeCloneItem = m_treeDatabase->createItem(treeItem, instanceCopy->getName(), iconIndex);
			treeCloneItem->setData(L"GROUP", group);
			treeCloneItem->setData(L"INSTANCE", instanceCopy);

			m_treeDatabase->update();
		}
		else if (command == L"Editor.Database.FavoriteEntireGroup" || command == L"Editor.Database.UnFavoriteEntireGroup")
		{
			bool addToFavorites = bool(command == L"Editor.Database.FavoriteEntireGroup");

			RefArray< db::Instance > instances;
			db::recursiveFindChildInstances(group, db::FindInstanceAll(), instances);

			for (RefArray< db::Instance >::iterator i = instances.begin(); i != instances.end(); ++i)
			{
				Guid instanceGuid = (*i)->getGuid();
				if (addToFavorites)
					m_favoriteInstances.insert(instanceGuid);
				else
				{
					std::set< Guid >::iterator i = m_favoriteInstances.find(instanceGuid);
					if (i != m_favoriteInstances.end())
						m_favoriteInstances.erase(i);
				}
			}

			std::vector< std::wstring > favoriteInstances;
			for (std::set< Guid >::iterator i = m_favoriteInstances.begin(); i != m_favoriteInstances.end(); ++i)
				favoriteInstances.push_back(i->format());

			Ref< PropertyGroup > globalSettings = m_editor->checkoutGlobalSettings();
			globalSettings->setProperty< PropertyStringArray >(L"Editor.FavoriteInstances", favoriteInstances);
			m_editor->commitGlobalSettings();

			updateView();
		}
		else if (command == L"Editor.Database.Wizard")
		{
			Ref< IWizardTool > wizard = m_wizardTools[command.getId()];
			if (wizard->launch(this, m_editor, group, 0))
				updateView();
		}
		else
			return false;
	}

	return true;
}

void DatabaseView::setEnable(bool enable)
{
	m_toolSelection->setEnable(enable);
	m_toolSelection->update();

	m_treeDatabase->setEnable(enable);
	m_treeDatabase->update();

	ui::Container::setEnable(enable);
}

int32_t DatabaseView::getIconIndex(const TypeInfo* instanceType) const
{
	Ref< PropertyGroup > iconsGroup = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Icons");
	T_ASSERT (iconsGroup);

	const std::map< std::wstring, Ref< IPropertyValue > >& icons = iconsGroup->getValues();

	int iconIndex = 2;
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = icons.begin(); i != icons.end(); ++i)
	{
		const TypeInfo* iconType = TypeInfo::find(i->first);
		if (iconType && is_type_of(*iconType, *instanceType))
		{
			iconIndex = PropertyInteger::get(i->second);
			break;
		}
	}

	return iconIndex;
}

Ref< ui::TreeViewItem > DatabaseView::buildTreeItem(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group)
{
	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parentItem, group->getName(), 0, 1);
	groupItem->setData(L"GROUP", group);

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
		buildTreeItem(treeView, groupItem, *i);

	bool showFiltered = m_toolFilterShow->isToggled();
	bool showFavorites = m_toolFavoritesShow->isToggled();
	bool showPrivate = true;

	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		const TypeInfo* primaryType = (*i)->getPrimaryType();
		if (!primaryType)
			continue;

		if (showFavorites)
		{
			if (m_favoriteInstances.find((*i)->getGuid()) == m_favoriteInstances.end())
				continue;
		}

		if (!showPrivate)
		{
			if (isInstanceInPrivate(*i))
				continue;
		}

		int32_t iconIndex = getIconIndex(primaryType);

		if (!showFiltered)
		{
			if (!m_filter->acceptInstance((*i)))
				continue;
		}
		else
		{
			if (!m_filter->acceptInstance((*i)))
				iconIndex += 23;
		}

		Ref< ui::TreeViewItem > instanceItem = treeView->createItem(
			groupItem,
			(*i)->getName(),
			iconIndex
		);

		if (m_rootInstances.find((*i)->getGuid()) != m_rootInstances.end())
			instanceItem->setBold(true);
		
		instanceItem->setData(L"GROUP", group);
		instanceItem->setData(L"INSTANCE", (*i));
	}

	// Remove group if it's empty.
	if ((showFavorites || !m_filter->acceptEmptyGroups()) && !groupItem->hasChildren())
	{
		treeView->removeItem(groupItem);
		groupItem = 0;
	}

	return groupItem;
}

void DatabaseView::filterType(db::Instance* instance)
{
	TypeInfoSet typeSet;
	typeSet.insert(instance->getPrimaryType());
	m_editFilter->setText(L"");
	m_filter = new TypeSetFilter(typeSet);
	m_toolFilterType->setToggled(true);
	m_toolFilterAssets->setToggled(false);
	updateView();
}

void DatabaseView::filterDependencies(db::Instance* instance)
{
	if (!instance)
		return;

	Ref< IPipelineDependencySet > dependencySet = m_editor->buildAssetDependencies(instance->getObject(), ~0UL);
	if (!dependencySet)
		return;

	std::set< Guid > guidSet;
	guidSet.insert(instance->getGuid());

	for (uint32_t i = 0; i < dependencySet->size(); ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT (dependency);

		if (dependency->outputGuid.isNotNull())
			guidSet.insert(dependency->outputGuid);
	}

	m_editFilter->setText(L"");
	m_filter = new GuidSetFilter(guidSet);
	m_toolFilterType->setToggled(true);
	m_toolFilterAssets->setToggled(false);

	updateView();
}

void DatabaseView::eventToolSelectionClicked(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Database.Filter")
	{
		if (m_toolFilterType->isToggled())
		{
			const TypeInfo* filterType = m_editor->browseType(&type_of< ISerializable >());
			if (filterType)
			{
				TypeInfoSet typeSet;
				typeSet.insert(filterType);
				m_editFilter->setText(L"");
				m_filter = new TypeSetFilter(typeSet);
				m_toolFilterAssets->setToggled(false);
			}
			else
				m_toolFilterType->setToggled(false);
		}
		if (!m_toolFilterType->isToggled())
			m_filter = new DefaultFilter();
	}
	else if (cmd == L"Database.FilterAssets")
	{
		if (m_toolFilterAssets->isToggled())
		{
			RefArray< db::Instance > assetsInstances;
			db::recursiveFindChildInstances(
				m_db->getRootGroup(),
				db::FindInstanceByType(type_of< Assets >()),
				assetsInstances
			);

			std::set< Guid > guidSet;
			for (RefArray< db::Instance >::iterator i = assetsInstances.begin(); i != assetsInstances.end(); ++i)
			{
				guidSet.insert((*i)->getGuid());

				Ref< IPipelineDependencySet > dependencySet = m_editor->buildAssetDependencies((*i)->getObject(), ~0UL);
				if (!dependencySet)
					continue;

				for (uint32_t j = 0; j < dependencySet->size(); ++j)
				{
					const PipelineDependency* dependency = dependencySet->get(j);
					T_ASSERT (dependency);

					if (dependency->outputGuid.isNotNull())
						guidSet.insert(dependency->outputGuid);
				}
			}

			m_editFilter->setText(L"");
			m_filter = new GuidSetFilter(guidSet);
			m_toolFilterType->setToggled(false);
		}
		if (!m_toolFilterAssets->isToggled())
			m_filter = new DefaultFilter();
	}
	
	updateView();
}

void DatabaseView::eventFilterKey(ui::KeyUpEvent* event)
{
	m_filterText = m_editFilter->getText();
	stopTimer();
	startTimer(500);
}

void DatabaseView::eventTimer(ui::TimerEvent* event)
{
	stopTimer();

	if (!m_filterText.empty())
		m_filter = new TextFilter(m_filterText);
	else
		m_filter = new DefaultFilter();

	m_toolFilterType->setToggled(false);
	m_toolFilterAssets->setToggled(false);

	updateView();
}

void DatabaseView::eventInstanceActivate(ui::TreeViewItemActivateEvent* event)
{
	Ref< ui::TreeViewItem > item = event->getItem();

	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	if (!instance)
		return;

	m_editor->openEditor(instance);
}

void DatabaseView::eventInstanceButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	Ref< ui::TreeViewItem > treeItem = m_treeDatabase->getSelectedItem();
	if (!treeItem)
		return;

	Ref< db::Group > group = treeItem->getData< db::Group >(L"GROUP");
	Ref< db::Instance > instance = treeItem->getData< db::Instance >(L"INSTANCE");

	if (group && instance)
	{
		Ref< ui::PopupMenu > menuInstance;

		if (is_type_of< Asset >(*instance->getPrimaryType()))
			menuInstance = m_menuInstanceAsset;
		else
			menuInstance = m_menuInstance;

		Ref< ui::MenuItem > selected = menuInstance->show(m_treeDatabase, event->getPosition());
		if (selected)
			handleCommand(selected->getCommand());
	}
	else if (group)
	{
		bool showFavorites = m_toolFavoritesShow->isToggled();
		Ref< ui::MenuItem > selected = m_menuGroup[showFavorites ? 1 : 0]->show(m_treeDatabase, event->getPosition());
		if (selected)
			handleCommand(selected->getCommand());
	}

	event->consume();
}

void DatabaseView::eventInstanceRenamed(ui::TreeViewContentChangeEvent* event)
{
	Ref< ui::TreeViewItem > treeItem = event->getItem();
	if (!treeItem)
		return;

	Ref< db::Instance > instance = treeItem->getData< db::Instance >(L"INSTANCE");
	Ref< db::Group > group = treeItem->getData< db::Group >(L"GROUP");

	bool result = false;

	if (instance && group)
	{
		if (instance->checkout())
		{
			result = instance->setName(treeItem->getText());
			result &= instance->commit();
		}
	}
	else if (group)
		result = group->rename(treeItem->getText());

	if (result)
		event->consume();
}

void DatabaseView::eventInstanceDrag(ui::TreeViewDragEvent* event)
{
	ui::TreeViewItem* dragItem = event->getItem();

	if (event->getMoment() == ui::TreeViewDragEvent::DmDrag)
	{
		// Only instance nodes are allowed to be dragged.
		if (!dragItem->getData< db::Instance >(L"INSTANCE"))
			event->cancel();
	}
	else if (event->getMoment() == ui::TreeViewDragEvent::DmDrop)
	{
		// @fixme Ensure drop target are active editor.

		Ref< db::Instance > instance = dragItem->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		Ref< IEditorPage > editorPage = m_editor->getActiveEditorPage();
		if (editorPage)
			editorPage->dropInstance(instance, event->getPosition());
	}

	event->consume();
}

	}
}
