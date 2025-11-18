/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/ShaderExperimentEditorPage.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "Render/Buffer.h"
#include "Render/Editor/Shader/Experiment/ShaderExperiment.h"
#include "Render/Editor/Shader/StructDeclaration.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Shader.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Splitter.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor::render
{
namespace
{

// std430
uint32_t getDeclarationSize(const StructDeclaration* decl)
{
	uint32_t offset = 0;
	for (const auto& elm : decl->getElements())
	{
		switch (elm.type)
		{
		case DtFloat1:
			{
				const uint32_t pad = alignUp(offset, 4) - offset;
				offset += pad + 4;
			}
			break;

		case DtFloat2:
			{
				const uint32_t pad = alignUp(offset, 8) - offset;
				offset += pad + 8;
			}
			break;

		case DtFloat3:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 12;
			}
			break;

		case DtFloat4:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 16;
			}
			break;

		case DtInteger1:
			{
				const uint32_t pad = alignUp(offset, 4) - offset;
				offset += pad + 4;
			}
			break;

		case DtInteger2:
			{
				const uint32_t pad = alignUp(offset, 8) - offset;
				offset += pad + 8;
			}
			break;

		case DtInteger3:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 12;
			}
			break;

		case DtInteger4:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 16;
			}
			break;

		default:
			return 0;
		}
	}

	const uint32_t pad = alignUp(offset, 16) - offset;
	return offset + pad;
}

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

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(parent, true, 150_ut);

	m_buffersGrid = new ui::GridView();
	m_buffersGrid->create(splitter, ui::GridView::WsColumnHeader);
	m_buffersGrid->addColumn(new ui::GridColumn(L"Name", 100_ut));
	m_buffersGrid->addColumn(new ui::GridColumn(L"", 20_ut));

	// Create result grid.
	m_resultGrid = new ui::GridView();
	m_resultGrid->create(splitter, ui::GridView::WsColumnHeader);

	const auto& data = m_experiment->getData();
	for (int32_t i = 0; i < data.size(); ++i)
		m_resultGrid->addColumn(new ui::GridColumn(data[i].name, 50_ut));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &ShaderExperimentEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	// Expose experiment to properties view.
	m_propertiesView->setPropertyObject(m_experiment);

	// Create render view.
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.allowHDR = false;
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	// Bind shader.
	if (!m_resourceManager->bind(
			resource::Id< Shader >(m_experiment->getShader()),
			m_shader))
		return false;

	// Initial resize and reset.
	m_renderWidget->setRect(ui::Rect(0, 0, 64, 64));
	m_renderView->reset(64, 64);

	// Add buffers to tree.
	for (const auto& data : m_experiment->getData())
	{
		Ref< const StructDeclaration > decl = m_editor->getSourceDatabase()->getObjectReadOnly< StructDeclaration >(data.structDeclaration);
		if (!decl)
			continue;

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(data.name);

		for (const auto& elm : decl->getElements())
		{
			Ref< ui::GridRow > childRow = new ui::GridRow();
			childRow->add(elm.name);
			row->addChild(childRow);
		}

		m_buffersGrid->addRow(row);
	}

	// Execute experiment to show initial results.
	executeExperiment();
	return true;
}

void ShaderExperimentEditorPage::destroy()
{
	T_FATAL_ASSERT(m_site != nullptr);

	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	m_shader.clear();

	safeClose(m_renderView);
	safeDestroy(m_resourceManager);
	safeDestroy(m_propertiesView);

	m_site = nullptr;
}

bool ShaderExperimentEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ShaderExperimentEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;

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
	const auto& data = m_experiment->getData();

	struct BufferInfo
	{
		std::wstring name;
		Ref< const StructDeclaration > decl;
		uint32_t declSize;
		uint32_t count;
		Ref< Buffer > buffer;
		uint8_t* bufferPtr;
	};

	AlignedVector< BufferInfo > buffers;

	// Create buffers.
	buffers.resize(data.size());
	for (int32_t i = 0; i < data.size(); ++i)
	{
		BufferInfo& bi = buffers[i];

		bi.name = data[i].name;
		if (bi.name.empty())
		{
			log::error << L"Unable to execute experiment; unnamed buffer." << Endl;
			return;
		}

		bi.decl = m_editor->getSourceDatabase()->getObjectReadOnly< StructDeclaration >(data[i].structDeclaration);
		if (!bi.decl)
		{
			log::error << L"Unable to execute experiment; missing struct declaration for buffer \"" << data[i].name << L"\"." << Endl;
			return;
		}

		bi.declSize = getDeclarationSize(bi.decl);
		if (!bi.declSize)
		{
			log::error << L"Unable to execute experiment; unsupported struct declaration for buffer \"" << data[i].name << L"\"." << Endl;
			return;
		}

		bi.count = data[i].count;
		if (!bi.count)
		{
			log::error << L"Unable to execute experiment; buffer needs atleast one item." << Endl;
			return;
		}

		bi.buffer = m_renderSystem->createBuffer(BufferUsage::BuStructured, bi.count * bi.declSize, true);
		if (!bi.buffer)
		{
			log::error << L"Unable to execute experiment; failed to create buffer." << Endl;
			return;
		}

		if ((bi.bufferPtr = (uint8_t*)bi.buffer->lock()) == nullptr)
		{
			log::error << L"Unable to execute experiment; failed to lock buffer." << Endl;
			return;
		}

		if (data[i].initial != ShaderExperiment::Initial::Undefined)
		{
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

		for (int32_t i = 0; i < buffers.size(); ++i)
			program.program->setBufferViewParameter(
				getParameterHandle(data[i].name),
				buffers[i].buffer->getBufferView());

		const int32_t workSize[] = { pass.workSize, 1, 1 };
		m_renderView->compute(program.program, workSize, false);
		m_renderView->barrier(render::Stage::Compute, render::Stage::Compute, nullptr, 0);

		m_renderView->endFrame();
		m_renderView->present();
	}

	// Update buffer views in grid views.
	m_resultGrid->removeAllColumns();
	m_resultGrid->removeAllRows();

	RefArray< ui::GridRow > outputRows;
	for (const auto& r : m_buffersGrid->getRows())
	{
		const std::wstring& bufferName = r->get(0)->getText();

		const auto it = std::find_if(
			buffers.begin(),
			buffers.end(),
			[&](const BufferInfo& it) {
			return bufferName == it.name;
		});

		for (const auto& elm : it->decl->getElements())
			m_resultGrid->addColumn(new ui::GridColumn(str(L"%s.%s", bufferName.c_str(), elm.name.c_str()), 100_ut));

		outputRows.resize(std::max((uint32_t)outputRows.size(), it->count));

		const uint8_t* rptr = it->bufferPtr;
		for (uint32_t i = 0; i < it->count; ++i)
		{
			if (!outputRows[i])
				outputRows[i] = new ui::GridRow();

			uint32_t offset = 0;
			for (const auto& elm : it->decl->getElements())
			{
				switch (elm.type)
				{
				case DtFloat1:
					{
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 0]));
						const uint32_t pad = alignUp(offset, 4) - offset;
						offset += pad + 4;
					}
					break;

				case DtFloat2:
					{
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 4]));
						const uint32_t pad = alignUp(offset, 8) - offset;
						offset += pad + 8;
					}
					break;

				case DtFloat3:
					{
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 4]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 8]));
						const uint32_t pad = alignUp(offset, 16) - offset;
						offset += pad + 12;
					}
					break;

				case DtFloat4:
					{
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 4]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 8]));
						outputRows[i]->add(str(L"%f", *(float*)&rptr[offset + 12]));
						const uint32_t pad = alignUp(offset, 16) - offset;
						offset += pad + 16;
					}
					break;

				case DtInteger1:
					{
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 0]));
						const uint32_t pad = alignUp(offset, 4) - offset;
						offset += pad + 4;
					}
					break;

				case DtInteger2:
					{
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 4]));
						const uint32_t pad = alignUp(offset, 8) - offset;
						offset += pad + 8;
					}
					break;

				case DtInteger3:
					{
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 4]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 8]));
						const uint32_t pad = alignUp(offset, 16) - offset;
						offset += pad + 12;
					}
					break;

				case DtInteger4:
					{
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 0]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 4]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 8]));
						outputRows[i]->add(str(L"%d", *(int32_t*)&rptr[offset + 12]));
						const uint32_t pad = alignUp(offset, 16) - offset;
						offset += pad + 16;
					}
					break;

				default:
					break;
				}
			}

			rptr += it->declSize;
		}
	}
	m_resultGrid->setRows(outputRows);

	// Destroy all buffers.
	for (auto& buffer : buffers)
	{
		buffer.buffer->unlock();
		buffer.buffer->destroy();
	}
}

void ShaderExperimentEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	executeExperiment();
}

void ShaderExperimentEditorPage::eventRenderSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	const ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

}
