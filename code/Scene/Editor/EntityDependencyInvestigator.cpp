#include "Core/Guid.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

std::wstring getCategoryText(const TypeInfo* categoryType)
{
	std::wstring id = replaceAll(toUpper(std::wstring(categoryType->getName())), L".", L"_");
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
	m_currentEntityAdapter = nullptr;
	m_context = nullptr;
	ui::Container::destroy();
}

bool EntityDependencyInvestigator::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"));

	m_dependencyTree = new ui::TreeView();
	m_dependencyTree->create(this, (ui::TreeView::WsDefault & ~(ui::TreeView::WsAutoEdit | ui::WsClientBorder)) | ui::WsDoubleBuffer);
	m_dependencyTree->addImage(new ui::StyleBitmap(L"Editor.Folders"), 2);
	m_dependencyTree->addImage(new ui::StyleBitmap(L"Editor.Types"), 23);
	m_dependencyTree->addEventHandler< ui::TreeViewItemActivateEvent >(this, &EntityDependencyInvestigator::eventDependencyActivate);

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
		editor::PipelineDependencySet dependencySet;
		Ref< editor::IPipelineDepends > depends = m_context->getEditor()->createPipelineDepends(&dependencySet, 1);
		if (!depends)
			return;

		depends->addDependency(entityAdapter->getEntityData());
		depends->waitUntilFinished();

		std::map< const TypeInfo*, Ref< ui::TreeViewItem > > typeGroups;
		std::set< Path > externalFiles;

		for (uint32_t i = 0; i < dependencySet.size(); ++i)
		{
			editor::PipelineDependency* dependency = dependencySet.get(i);
			T_ASSERT(dependency);

			const TypeInfo* assetType = &type_of(dependency->sourceAsset);
			T_ASSERT(assetType);

			Ref< ui::TreeViewItem > typeGroup = typeGroups[assetType];
			if (!typeGroup)
			{
				typeGroup = m_dependencyTree->createItem(0, getCategoryText(assetType), 1);
				typeGroup->setImage(0, 0, 1);
				typeGroups[assetType] = typeGroup;
			}

			Ref< ui::TreeViewItem > dependencyItem = m_dependencyTree->createItem(typeGroup, dependency->outputPath, 1);
			dependencyItem->setImage(0, 2);
			dependencyItem->setData(L"DEPENDENCY", dependency);

			for (const auto& file : dependency->files)
				externalFiles.insert(file.filePath);
		}

		if (!externalFiles.empty())
		{
			Ref< ui::TreeViewItem > filesGroup = m_dependencyTree->createItem(0, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_FILES"), 1);
			filesGroup->setImage(0, 0, 1);
			for (const auto& file : externalFiles)
			{
				Ref< ui::TreeViewItem > fileItem = m_dependencyTree->createItem(filesGroup, file.getFileName(), 1);
				fileItem->setImage(0, 2);
				fileItem->setData(L"FILE", new Path(file));
			}
		}
	}
}

void EntityDependencyInvestigator::eventDependencyActivate(ui::TreeViewItemActivateEvent* event)
{
	Ref< ui::TreeViewItem > item = event->getItem();

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
		setEntityAdapter(nullptr);
}

	}
}
