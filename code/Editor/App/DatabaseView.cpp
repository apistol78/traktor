#include "Editor/App/DatabaseView.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/IProject.h"
#include "Editor/IWizardTool.h"
#include "Editor/Settings.h"
#include "Editor/Asset.h"
#include "Editor/PipelineManager.h"
#include "Editor/PipelineHash.h"
#include "Editor/IPipeline.h"
#include "Ui/Bitmap.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/FloodLayout.h"
#include "Ui/MessageBox.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/TreeViewDragEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "I18N/Text.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/DatabaseView.h"
#include "Resources/Types.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView", DatabaseView, ui::Container)

T_IMPLEMENT_RTTI_COMPOSITE_CLASS(L"traktor.editor.DatabaseView.Filter", DatabaseView, Filter, Object)

		namespace
		{

class DefaultFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS(DefaultFilter)

public:
	virtual bool acceptInstance(const db::Instance* instance) const
	{
		return is_type_of< Serializable >(*instance->getPrimaryType());
	}

	virtual bool acceptEmptyGroups() const
	{
		return true;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"DefaultFilter", DefaultFilter, DatabaseView::Filter)

class TypeSetFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS(TypeSetFilter)

public:
	TypeSetFilter(const TypeSet& typeSet)
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
	mutable TypeSet m_typeSet;
};

T_IMPLEMENT_RTTI_CLASS(L"TypeSetFilter", TypeSetFilter, DatabaseView::Filter)

class GuidSetFilter : public DatabaseView::Filter
{
	T_RTTI_CLASS(DefaultFilter)

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

T_IMPLEMENT_RTTI_CLASS(L"GuidSetFilter", GuidSetFilter, DatabaseView::Filter)

		}

DatabaseView::DatabaseView(IEditor* editor)
:	m_editor(editor)
,	m_filter(gc_new< DefaultFilter >())
{
}

bool DatabaseView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolFilter = gc_new< ui::custom::ToolBarButton >(
		i18n::Text(L"DATABASE_FILTER"),
		ui::Command(L"Database.Filter"),
		0,
		ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolSelection = gc_new< ui::custom::ToolBar >();
	if (!m_toolSelection->create(this))
		return false;
	m_toolSelection->addImage(ui::Bitmap::load(c_ResourceDatabaseView, sizeof(c_ResourceDatabaseView), L"png"), 1);
	m_toolSelection->addItem(m_toolFilter);
	m_toolSelection->addClickEventHandler(ui::createMethodHandler(this, &DatabaseView::eventToolSelectionClicked));

	m_treeDatabase = gc_new< ui::TreeView >();
	if (!m_treeDatabase->create(this, (ui::TreeView::WsDefault | ui::TreeView::WsDrag) & ~ui::WsClientBorder))
		return false;
	m_treeDatabase->addImage(ui::Bitmap::load(c_ResourceTypes, sizeof(c_ResourceTypes), L"png"), 15);
	m_treeDatabase->addActivateEventHandler(ui::createMethodHandler(this, &DatabaseView::eventInstanceActivate));
	m_treeDatabase->addButtonDownEventHandler(ui::createMethodHandler(this, &DatabaseView::eventInstanceButtonDown));
	m_treeDatabase->addEditedEventHandler(ui::createMethodHandler(this, &DatabaseView::eventInstanceRenamed));
	m_treeDatabase->addDragEventHandler(ui::createMethodHandler(this, &DatabaseView::eventInstanceDrag));
	m_treeDatabase->setEnable(false);
		
	m_menuGroup = gc_new< ui::PopupMenu >();
	if (!m_menuGroup->create())
		return false;
	m_menuGroup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.NewInstance"), i18n::Text(L"DATABASE_NEW_INSTANCE")));
	m_menuGroup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.NewGroup"), i18n::Text(L"DATABASE_NEW_GROUP")));
	m_menuGroup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuGroup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Delete"), i18n::Text(L"DATABASE_DELETE")));
	
	std::vector< const Type* > wizardToolTypes;
	type_of< IWizardTool >().findAllOf(wizardToolTypes);

	if (!wizardToolTypes.empty())
	{
		Ref< ui::MenuItem > menuWizard = gc_new< ui::MenuItem >(i18n::Text(L"DATABASE_WIZARDS"));

		int wizardId = 0;
		for (std::vector< const Type* >::iterator i = wizardToolTypes.begin(); i != wizardToolTypes.end(); ++i)
		{
			Ref< IWizardTool > wizard = dynamic_type_cast< IWizardTool* >((*i)->newInstance());
			if (!wizard)
				continue;

			std::wstring wizardDescription = wizard->getDescription();
			T_ASSERT (!wizardDescription.empty());

			menuWizard->add(gc_new< ui::MenuItem >(ui::Command(wizardId++, L"Editor.Database.Wizard"), wizardDescription));
			m_wizardTools.push_back(wizard);
		}

		m_menuGroup->add(menuWizard);
	}

	m_menuInstance = gc_new< ui::PopupMenu >();
	if (!m_menuInstance->create())
		return false;
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstance->add(gc_new< ui::MenuItem >(L"-"));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstance->add(gc_new< ui::MenuItem >(L"-"));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstance->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	m_menuInstanceAsset = gc_new< ui::PopupMenu >();
	if (!m_menuInstanceAsset->create())
		return false;
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Edit"), i18n::Text(L"DATABASE_EDIT")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Explore"), i18n::Text(L"DATABASE_EXPLORE")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(L"-"));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(L"-"));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(L"-"));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstanceAsset->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	return true;
}

void DatabaseView::destroy()
{
	m_menuGroup->destroy();
	m_menuInstance->destroy();
	m_menuInstanceAsset->destroy();

	ui::Container::destroy();
}

void DatabaseView::setDatabase(db::Database* db)
{
	m_db = db;
	updateView();
}

void DatabaseView::updateView()
{
	Ref< ui::TreeViewState > treeState = m_treeDatabase->captureState();

	m_treeDatabase->removeAllItems();

	if (m_db)
	{
		Ref< ui::TreeViewItem > rootItem = buildTreeItem(m_treeDatabase, 0, m_db->getRootGroup());
		if (rootItem)
			rootItem->expand();

		m_treeDatabase->setEnable(true);
	}
	else
	{
		m_treeDatabase->setData(L"DOMAIN", 0);
		m_treeDatabase->setEnable(false);
	}

	m_treeDatabase->applyState(treeState);
	m_treeDatabase->update();
}

void DatabaseView::setEnable(bool enable)
{
	m_toolSelection->setEnable(enable);
	m_treeDatabase->setEnable(enable);
	ui::Container::setEnable(enable);
}

ui::TreeViewItem* DatabaseView::buildTreeItem(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group)
{
	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parentItem, group->getName(), 0, 1);
	groupItem->setData(L"GROUP", group);

	for (Ref< db::Group > groupIter = group->getFirstChildGroup(); groupIter; groupIter = group->getNextChildGroup(groupIter))
		buildTreeItem(treeView, groupItem, groupIter);

	std::map< std::wstring, Ref< PropertyValue > > icons = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Icons");

	for (Ref< db::Instance > instanceIter = group->getFirstChildInstance(); instanceIter; instanceIter = group->getNextChildInstance(instanceIter))
	{
		const Type* primaryType = instanceIter->getPrimaryType();
		if (!primaryType)
			continue;

		if (!m_filter->acceptInstance(instanceIter))
			continue;

		int iconIndex = 2;
		for (std::map< std::wstring, Ref< PropertyValue > >::const_iterator i = icons.begin(); i != icons.end(); ++i)
		{
			const Type* iconType = Type::find(i->first);
			if (iconType && is_type_of(*iconType, *primaryType))
			{
				iconIndex = PropertyInteger::get(i->second);
				break;
			}
		}

		Ref< ui::TreeViewItem > instanceItem = treeView->createItem(
			groupItem,
			instanceIter->getName(),
			iconIndex
		);
		
		instanceItem->setData(L"GROUP", group);
		instanceItem->setData(L"INSTANCE", instanceIter);
	}

	// Remove group if it's empty.
	if (!m_filter->acceptEmptyGroups() && !groupItem->hasChildren())
	{
		treeView->removeItem(groupItem);
		groupItem = 0;
	}

	return groupItem;
}

void DatabaseView::filterType(db::Instance* instance)
{
	TypeSet typeSet;
	typeSet.insert(instance->getPrimaryType());
	m_filter = gc_new< TypeSetFilter >(cref(typeSet));
	m_toolFilter->setToggled(true);
	updateView();
}

void DatabaseView::filterDependencies(db::Instance* instance)
{
	RefArray< IPipeline > pipelines;

	std::vector< const Type* > pipelineTypes;
	type_of< IPipeline >().findAllOf(pipelineTypes);

	for (std::vector< const Type* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< IPipeline > pipeline = dynamic_type_cast< IPipeline* >((*i)->newInstance());
		if (pipeline)
		{
			if (!pipeline->create(m_editor->getSettings()))
			{
				log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"" << Endl;
				continue;
			}
			pipelines.push_back(pipeline);
		}
	}

	Ref< editor::IProject > project = m_editor->getProject();
	T_ASSERT (project);

	Ref< PipelineHash > pipelineHash = gc_new< PipelineHash >();
	Ref< PipelineManager > pipelineManager = gc_new< PipelineManager >(
		project->getSourceDatabase(),
		project->getOutputDatabase(),
		pipelines,
		pipelineHash
	);

	pipelineManager->addDependency(instance, false);

	std::set< Guid > guidSet;

	const RefArray< PipelineManager::Dependency >& dependencies = pipelineManager->getDependencies();
	for (RefArray< PipelineManager::Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		guidSet.insert((*i)->outputGuid);

	m_filter = gc_new< GuidSetFilter >(cref(guidSet));
	m_toolFilter->setToggled(true);
	updateView();
}

void DatabaseView::eventToolSelectionClicked(ui::Event* event)
{
	const ui::CommandEvent* commandEvent = checked_type_cast< const ui::CommandEvent* >(event);

	ui::custom::ToolBarButton* toolButton = checked_type_cast< ui::custom::ToolBarButton* >(commandEvent->getItem());
	if (toolButton->isToggled())
	{
		const Type* filterType = m_editor->browseType(&type_of< Serializable >());
		if (filterType)
		{
			TypeSet typeSet;
			typeSet.insert(filterType);
			m_filter = gc_new< TypeSetFilter >(cref(typeSet));
		}
		else
			toolButton->setToggled(false);
	}
	if (!toolButton->isToggled())
		m_filter = gc_new< DefaultFilter >();

	updateView();
}

void DatabaseView::eventInstanceActivate(ui::Event* event)
{
	Ref< ui::TreeViewItem > item = checked_type_cast< ui::TreeViewItem* >(
		checked_type_cast< ui::CommandEvent* >(event)->getItem()
	);

	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	if (!instance)
		return;

	m_editor->openEditor(instance);
}

void DatabaseView::eventInstanceButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::TreeView > treeDomain = checked_type_cast< ui::TreeView* >(mouseEvent->getSender());
	Ref< ui::TreeViewItem > treeItem = treeDomain->getSelectedItem();
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

		Ref< ui::MenuItem > selected = menuInstance->show(treeDomain, mouseEvent->getPosition());
		if (!selected)
			return;

		if (selected->getCommand() == L"Editor.Database.Edit")	// Edit
		{
			Ref< Asset > editAsset = instance->getObject< Asset >();
			if (editAsset)
				OS::getInstance().editFile(editAsset->getFileName());
		}
		else if (selected->getCommand() == L"Editor.Database.Explore")	// Explore
		{
			Ref< Asset > exploreAsset = instance->getObject< Asset >();
			if (exploreAsset)
				OS::getInstance().exploreFile(exploreAsset->getFileName());
		}
		else if (selected->getCommand() == L"Editor.Database.Rename")	// Rename
		{
			treeItem->edit();
		}
		else if (selected->getCommand() == L"Editor.Database.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_INSTANCE"), ui::MbYesNo | ui::MbIconQuestion) != 1)
				return;

			if (!instance->checkout())
				return;

			if (!instance->remove())
				return;

			if (!instance->commit())
				return;

			treeDomain->removeItem(treeItem);
			treeDomain->update();
		}
		else if (selected->getCommand() == L"Editor.Database.Clone")	// Clone
		{
			Ref< Serializable > object = instance->getObject< Serializable >();
			if (!object)
			{
				log::error << L"Unable to checkout instance" << Endl;
				return;
			}

			object = DeepClone(object).create();
			if (!object)
			{
				log::error << L"Unable to create clone" << Endl;
				return;
			}

			Ref< db::Instance > instanceClone = group->createInstance(instance->getName() + L" (clone)");
			if (!instanceClone)
			{
				log::error << L"Unable to create clone instance" << Endl;
				return;
			}

			instanceClone->setObject(object);

			if (!instanceClone->commit())
			{
				log::error << L"Unable to commit clone instance" << Endl;
				return;
			}

			Ref< ui::TreeViewItem > treeCloneItem = treeDomain->createItem(treeItem->getParent(), instanceClone->getName(), treeItem->getImage());
			treeCloneItem->setData(L"GROUP", group);
			treeCloneItem->setData(L"INSTANCE", instanceClone);
			
			treeDomain->update();
		}
		else if (selected->getCommand() == L"Editor.Database.FilterInstanceType")	// Filter on type.
		{
			filterType(instance);
		}
		else if (selected->getCommand() == L"Editor.Database.FilterInstanceDepends")	// Filter on dependencies
		{
			filterDependencies(instance);
		}
		else if (selected->getCommand() == L"Editor.Database.Build")	// Build asset
		{
			m_editor->buildAsset(instance->getGuid(), false);
		}
		else if (selected->getCommand() == L"Editor.Database.Rebuild")	// Rebuild asset
		{
			m_editor->buildAsset(instance->getGuid(), true);
		}
	}
	else if (group)
	{
		Ref< ui::MenuItem > selected = m_menuGroup->show(treeDomain, mouseEvent->getPosition());
		if (!selected)
			return;

		const ui::Command& command = selected->getCommand();

		if (command == L"Editor.Database.NewInstance")	// New instance...
		{
			NewInstanceDialog newInstanceDlg;
			newInstanceDlg.create(this);

			if (newInstanceDlg.showModal() == ui::DrOk)
			{
				std::wstring typeName = newInstanceDlg.getTypeName();
				T_ASSERT (!typeName.empty());

				std::wstring instanceName = newInstanceDlg.getInstanceName();
				T_ASSERT (!instanceName.empty());

				const Type* type = Type::find(typeName);
				T_ASSERT (type);

				Ref< Serializable > data = dynamic_type_cast< Serializable* >(type->newInstance());
				T_ASSERT (data);

				Ref< db::Instance > instance = group->createInstance(instanceName);
				if (instance)
				{
					instance->setObject(data);
					if (instance->commit())
					{
						const PropertyGroup* iconGroup = checked_type_cast< const PropertyGroup* >(m_editor->getSettings()->getProperty(L"Editor.Icons"));
						int iconIndex = iconGroup->getProperty< PropertyInteger >(typeName, 2);

						Ref< ui::TreeViewItem > instanceItem = treeDomain->createItem(treeItem, instanceName, iconIndex);
						instanceItem->setData(L"GROUP", group);
						instanceItem->setData(L"INSTANCE", instance);
						
						treeDomain->update();
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
				Ref< ui::TreeViewItem > groupItem = treeDomain->createItem(treeItem, i18n::Text(L"DATABASE_NEW_GROUP_UNNAMED"), 0, 1);
				groupItem->setData(L"GROUP", newGroup);

				treeDomain->update();

				// Enter edit mode directly as user probably don't want to call
				// the group "Unnamed".
				groupItem->edit();
			}
		}
		else if (command == L"Editor.Database.Rename")	// Rename
		{
			treeItem->edit();
		}
		else if (command == L"Editor.Database.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_GROUP"), ui::MbYesNo | ui::MbIconQuestion) != 1)
				return;

			if (!group->remove())
				return;

			treeDomain->removeItem(treeItem);
			treeDomain->update();
		}
		else if (command == L"Editor.Database.Wizard")
		{
			Ref< IWizardTool > wizard = m_wizardTools[command.getId()];

			if (wizard->launch(this, m_editor, group))
				updateView();
		}
	}

	event->consume();
}

void DatabaseView::eventInstanceRenamed(ui::Event* event)
{
	Ref< ui::TreeViewItem > treeItem = checked_type_cast< ui::TreeViewItem* >(
		checked_type_cast< ui::CommandEvent* >(event)->getItem()
	);
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

void DatabaseView::eventInstanceDrag(ui::Event* event)
{
	ui::TreeViewDragEvent* dragEvent = checked_type_cast< ui::TreeViewDragEvent* >(event);
	ui::TreeViewItem* dragItem = checked_type_cast< ui::TreeViewItem* >(dragEvent->getItem());

	if (dragEvent->getMoment() == ui::TreeViewDragEvent::DmDrag)
	{
		// Only instance nodes are allowed to be dragged.
		if (!dragItem->getData< db::Instance >(L"INSTANCE"))
			dragEvent->cancel();
	}
	else if (dragEvent->getMoment() == ui::TreeViewDragEvent::DmDrop)
	{
		// @fixme Ensure drop target are active editor.

		Ref< db::Instance > instance = dragItem->getData< db::Instance >(L"INSTANCE");
		T_ASSERT (instance);

		Ref< IEditorPage > editorPage = m_editor->getActiveEditorPage();
		if (editorPage)
			editorPage->dropInstance(instance, dragEvent->getPosition());
	}

	dragEvent->consume();
}

	}
}
