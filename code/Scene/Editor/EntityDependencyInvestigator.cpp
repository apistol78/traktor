#include "Core/Guid.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewItemActivateEvent.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

std::wstring getCategoryText(const TypeInfo* categoryType)
{
	std::wstring id = L"DATABASE_CATEGORY_" + replaceAll< std::wstring >(toUpper(std::wstring(categoryType->getName())), L".", L"_");
	return i18n::Text(id, categoryType->getName());
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityDependencyInvestigator", EntityDependencyInvestigator, ui::Container)

EntityDependencyInvestigator::EntityDependencyInvestigator(SceneEditorContext* context)
:	m_context(context)
{
}

void EntityDependencyInvestigator::destroy()
{
	safeDestroy(m_dependencyTree);
	m_currentEntityAdapter = 0;
	m_context = 0;
	ui::Container::destroy();
}

bool EntityDependencyInvestigator::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"));

	m_dependencyTree = new ui::custom::TreeView();
	m_dependencyTree->create(this, (ui::custom::TreeView::WsDefault & ~ui::WsClientBorder) | ui::WsDoubleBuffer);
	m_dependencyTree->addImage(new ui::StyleBitmap(L"Scene.Folders"), 2);
	m_dependencyTree->addImage(new ui::StyleBitmap(L"Scene.Types"), 23);
	m_dependencyTree->addEventHandler< ui::custom::TreeViewItemActivateEvent >(this, &EntityDependencyInvestigator::eventDependencyActivate);

	m_context->addEventHandler< ui::SelectionChangeEvent >(this, &EntityDependencyInvestigator::eventContextSelect);
	return true;
}

void EntityDependencyInvestigator::setEntityAdapter(EntityAdapter* entityAdapter)
{
	if (!isVisible(false))
		return;

	m_dependencyTree->removeAllItems();

	if (entityAdapter)
	{
		Ref< editor::IPipelineDependencySet > dependencySet = m_context->getEditor()->buildAssetDependencies(entityAdapter->getEntityData(), 1);
		T_ASSERT (dependencySet);

		std::map< const TypeInfo*, Ref< ui::custom::TreeViewItem > > typeGroups;
		std::set< Path > externalFiles;

		for (uint32_t i = 0; i < dependencySet->size(); ++i)
		{
			editor::PipelineDependency* dependency = dependencySet->get(i);
			T_ASSERT (dependency);

			const TypeInfo* assetType = &type_of(dependency->sourceAsset);
			T_ASSERT (assetType);

			Ref< ui::custom::TreeViewItem > typeGroup = typeGroups[assetType];
			if (!typeGroup)
			{
				typeGroup = m_dependencyTree->createItem(0, getCategoryText(assetType), 0, 1);
				typeGroups[assetType] = typeGroup;
			}

			Ref< ui::custom::TreeViewItem > dependencyItem = m_dependencyTree->createItem(typeGroup, dependency->outputPath, 2);
			dependencyItem->setData(L"DEPENDENCY", dependency);

			for (AlignedVector< editor::PipelineDependency::ExternalFile >::const_iterator j = dependency->files.begin(); j != dependency->files.end(); ++j)
				externalFiles.insert(j->filePath);
		}

		if (!externalFiles.empty())
		{
			Ref< ui::custom::TreeViewItem > filesGroup = m_dependencyTree->createItem(0, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_FILES"), 0, 1);
			for (std::set< Path >::iterator i = externalFiles.begin(); i != externalFiles.end(); ++i)
			{
				Ref< ui::custom::TreeViewItem > fileItem = m_dependencyTree->createItem(filesGroup, i->getFileName(), 2);
				fileItem->setData(L"FILE", new Path(*i));
			}
		}
	}
}

void EntityDependencyInvestigator::eventDependencyActivate(ui::custom::TreeViewItemActivateEvent* event)
{
	Ref< ui::custom::TreeViewItem > item = event->getItem();

	Ref< editor::PipelineDependency > dependency = item->getData< editor::PipelineDependency >(L"DEPENDENCY");
	if (dependency)
	{
		Ref< db::Instance > instance = m_context->getEditor()->getSourceDatabase()->getInstance(dependency->outputGuid);
		if (instance)
			m_context->getEditor()->openEditor(instance);
	}

	Ref< Path > file = item->getData< Path >(L"FILE");
	if (file)
	{
		if (!OS::getInstance().editFile(file->getPathName()))
			log::error << L"Unable to edit file \"" << file->getFileName() << L"\"" << Endl;
	}
}

void EntityDependencyInvestigator::eventContextSelect(ui::SelectionChangeEvent* event)
{
	RefArray< EntityAdapter > selectedEntityAdapters;
	if (m_context->getEntities(selectedEntityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly) == 1)
		setEntityAdapter(selectedEntityAdapters[0]);
	else
		setEntityAdapter(0);
}

	}
}
