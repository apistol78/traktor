/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/TracerEditorPlugin.h"
#include "Shape/Editor/Bake/TracerPanel.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Ui/Application.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerEditorPlugin", TracerEditorPlugin, editor::IEditorPlugin)

TracerEditorPlugin::TracerEditorPlugin(editor::IEditor* editor)
:   m_editor(editor)
,	m_site(nullptr)
{
}

bool TracerEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_site = site;

	m_tracerPanel = new TracerPanel(m_editor);
	m_tracerPanel->create(parent);

	m_site->createAdditionalPanel(m_tracerPanel, 100, false);
    return true;
}

void TracerEditorPlugin::destroy()
{
	if (m_tracerPanel)
	{
		m_site->destroyAdditionalPanel(m_tracerPanel);
		m_site = nullptr;
		m_tracerPanel = nullptr;
	}
}

bool TracerEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
    return false;
}

void TracerEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void TracerEditorPlugin::handleWorkspaceOpened()
{
	std::wstring tracerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree");
	if (tracerTypeName.empty())
		return;

	auto tracerType = TypeInfo::find(tracerTypeName.c_str());
	if (!tracerType)
		return;

	std::wstring compressionMethod = L""; // m_editor->getSettings()->getProperty< std::wstring >(L"TexturePipeline.CompressionMethod", L"DXTn");

    BakePipelineOperator::setTracerProcessor(new TracerProcessor(
		tracerType,
		compressionMethod,
		true
	));
}

void TracerEditorPlugin::handleWorkspaceClosed()
{
	BakePipelineOperator::setTracerProcessor(nullptr);
}

void TracerEditorPlugin::handleEditorClosed()
{
}

    }
}