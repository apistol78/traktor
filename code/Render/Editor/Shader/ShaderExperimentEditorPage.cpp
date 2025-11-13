/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderExperimentEditorPage.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IBrowseFilter.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Render/Buffer.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Group.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphEvaluator.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Render/Editor/Shader/Facades/DefaultNodeFacade.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Render/Editor/Shader/Facades/IndexedUniformNodeFacade.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Render/Editor/Shader/Facades/ScalarNodeFacade.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwizzleNodeFacade.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Render/Editor/Shader/Facades/UniformNodeFacade.h"
#include "Render/Editor/Shader/Facades/VariableNodeFacade.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/INodeFacade.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/QuickMenuTool.h"
#include "Render/Editor/Shader/ShaderDependencyPane.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Render/Editor/Shader/ShaderExperiment.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"
#include "Render/Editor/Shader/ShaderViewer.h"
#include "Render/Editor/Shader/UniformDeclaration.h"
#include "Render/Editor/Shader/UniformLinker.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/GroupMovedEvent.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::render
{
	namespace
	{

const int32_t numBuffers = 16;
const int32_t numBufferElements = 64;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderExperimentEditorPage", ShaderExperimentEditorPage, editor::IEditorPage)

ShaderExperimentEditorPage::ShaderExperimentEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
	: m_editor(editor)
	, m_site(site)
	, m_document(document)
{
}

bool ShaderExperimentEditorPage::create(ui::Container* parent)
{
	Ref< db::Database > database = m_editor->getOutputDatabase();
	if (!database)
		return false;

	m_experiment = m_document->getObject< ShaderExperiment >(0);
	if (!m_experiment)
		return false;

	m_renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!m_renderSystem)
		return false;

	// Create resource manager.
	m_resourceManager = new resource::ResourceManager(database, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new render::TextureFactory(m_renderSystem, 0));
	m_resourceManager->addFactory(new render::ShaderFactory(m_renderSystem));

	// Create dummy render widget.
	m_renderWidget = new ui::Widget();
	m_renderWidget->create(parent, ui::WsNoCanvas);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &ShaderExperimentEditorPage::eventRenderSize);
	m_renderWidget->setVisible(false);




	// Create result grid.
	m_resultGrid = new ui::GridView();
	m_resultGrid->create(parent, ui::GridView::WsColumnHeader);
	m_resultGrid->addColumn(new ui::GridColumn(L"", 30_ut));
	for (int32_t i = 0; i < numBuffers; ++i)
		m_resultGrid->addColumn(new ui::GridColumn(str(L"%c", L'A' + i), 60_ut));



	// Create render view.
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.allowHDR = false;
	desc.waitVBlanks = 1;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	// Bind shader.
	if (!m_resourceManager->bind(
		resource::Id< Shader >(m_experiment->getShader()),
		m_shader
	))
		return false;

	// Initial resize and reset.
	m_renderWidget->setRect(ui::Rect(0, 0, 64, 64));
	m_renderView->reset(64, 64);

	// Execute experiment to show initial results.
	executeExperiment();
	return true;
}

void ShaderExperimentEditorPage::destroy()
{
	T_FATAL_ASSERT(m_site != nullptr);

	m_shader.clear();

	safeClose(m_renderView);
	safeDestroy(m_resourceManager);

	m_site = nullptr;
}

bool ShaderExperimentEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ShaderExperimentEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void ShaderExperimentEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager && database == m_editor->getOutputDatabase())
	{
		if (m_resourceManager->reload(eventId, false))
		{
			// Resource reloaded; re-run experiment.
			executeExperiment();
		}
	}	
}

void ShaderExperimentEditorPage::executeExperiment()
{
	Ref< Buffer > buffers[numBuffers];
	uint32_t* bufferPtrs[numBuffers];

	// Create buffers.
	for (int32_t i = 0; i < numBuffers; ++i)
	{
		buffers[i] = m_renderSystem->createBuffer(BufferUsage::BuStructured, numBufferElements * sizeof(uint32_t), true);
		if (!buffers[i])
		{
			log::error << L"Unable to execute experiment; failed to create buffer." << Endl;
			return;
		}

		if ((bufferPtrs[i] = (uint32_t*)buffers[i]->lock()) == nullptr)
		{
			log::error << L"Unable to execute experiment; failed to lock buffer." << Endl;
			return;
		}
	}

	// Upload initial data into buffers.
	{
		Random rnd;
		for (int32_t i = 0; i < numBufferElements; ++i)
		{
			bufferPtrs[0][i] = rnd.next() & 3;
			for (int32_t j = 1; j < numBuffers; ++j)
				bufferPtrs[j][i] = 0;
		}
	}

	// Execute expermient passes.
	for (const auto& pass : m_experiment->getPasses())
	{
		const Shader::Program program = m_shader->getProgram(Shader::Permutation(getParameterHandle(pass.technique)));
		if (!program)
		{
			log::warning << L"No technique \"" << pass.technique << L"\" in experiment found; pass ignored." << Endl;
			continue;
		}

		m_renderView->beginFrame();

		for (int32_t i = 0; i < numBuffers; ++i)
		{
			const std::wstring bufferName = str(L"Buffer_%c", L'A' + i);
			program.program->setBufferViewParameter(
				getParameterHandle(bufferName),
				buffers[i]->getBufferView()
			);
		}

		const int32_t workSize[] = { pass.workSize, 1, 1 };
		m_renderView->compute(program.program, workSize, false);
		m_renderView->barrier(render::Stage::Compute, render::Stage::Compute, nullptr, 0);

		m_renderView->endFrame();
		m_renderView->present();
	}

	// Update buffer views in grid views.
	m_resultGrid->removeAllRows();
	for (int32_t i = 0; i < numBufferElements; ++i)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->set(0, new ui::GridItem(toString(i)));
		for (int32_t j = 0; j < numBuffers; ++j)
			row->set(j + 1, new ui::GridItem(toString(bufferPtrs[j][i])));
		m_resultGrid->addRow(row);
	}

	// Destroy all buffers.
	for (int32_t i = 0; i < numBuffers; ++i)
	{
		buffers[i]->unlock();
		safeDestroy(buffers[i]);
	}
}

void ShaderExperimentEditorPage::eventRenderSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

}
