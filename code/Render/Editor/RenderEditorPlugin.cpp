/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/Vrfy/RenderSystemVrfy.h"
#include "Render/Editor/RenderEditorPlugin.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Ui/MessageBox.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderEditorPlugin", 0, RenderEditorPlugin, editor::IEditorPlugin)

bool RenderEditorPlugin::create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_editor = editor;

	if (!createRenderSystem())
		return false;

	return true;
}

void RenderEditorPlugin::destroy()
{
	Ref< IRenderSystem > renderSystem = m_editor->getObjectStore()->get< IRenderSystem >();
	if (renderSystem)
	{
		m_editor->getObjectStore()->unset(renderSystem);
		safeDestroy(renderSystem);
	}
}

int32_t RenderEditorPlugin::getOrdinal() const
{
	return 0;
}

void RenderEditorPlugin::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Render.PrintMemoryUsage"));
}

bool RenderEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	if (command == L"Editor.SettingsChanged")
	{
		createRenderSystem();
		return true;
	}
	else if (command == L"Render.PrintMemoryUsage")
	{
		Ref< IRenderSystem > renderSystem = m_editor->getObjectStore()->get< IRenderSystem >();
		if (renderSystem)
		{
			RenderSystemStatistics rss;
			renderSystem->getStatistics(rss);

			log::info << L"Render system statistics ==================" << Endl;
			log::info << IncreaseIndent;
			log::info << L"Memory available: " << formatByteSize(rss.memoryAvailable) << Endl;
			log::info << L"Memory usage: " << formatByteSize(rss.memoryUsage) << Endl;
			log::info << L"Allocation count: " << rss.allocationCount << Endl;
			log::info << L"Buffers: " << rss.buffers << Endl;
			log::info << L"Simple textures: " << rss.simpleTextures << Endl;
			log::info << L"Cube textures: " << rss.cubeTextures << Endl;
			log::info << L"Volume textures: " << rss.volumeTextures << Endl;
			log::info << L"Render target sets: " << rss.renderTargetSets << Endl;
			log::info << L"Programs: " << rss.programs << Endl;
			log::info << DecreaseIndent;
		}
		else
			log::info << L"No render system created." << Endl;

		return true;
	}
	else
		return false;
}

void RenderEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_tracker && database == m_editor->getSourceDatabase())
		m_tracker->scan(database, eventId);
}

void RenderEditorPlugin::handleWorkspaceOpened()
{
	m_tracker = new ShaderDependencyTracker();
	m_tracker->scan(m_editor->getSourceDatabase());
	m_editor->getObjectStore()->set(m_tracker);
}

void RenderEditorPlugin::handleWorkspaceClosed()
{
	m_editor->getObjectStore()->unset(m_tracker);
	safeDestroy(m_tracker);
}

void RenderEditorPlugin::handleEditorClosed()
{
}

bool RenderEditorPlugin::createRenderSystem()
{
	auto settings = m_editor->getSettings();

	// Create render system.
	const std::wstring renderSystemTypeName = settings->getProperty< std::wstring >(L"Editor.RenderSystem");

	// Check if render system is already instantiated.
	if (m_renderSystem != nullptr)
	{
		if (type_name(m_renderSystem) == renderSystemTypeName)
			return true;
	}

	const TypeInfo* renderSystemType = TypeInfo::find(renderSystemTypeName.c_str());
	if (!renderSystemType)
	{
		ui::MessageBox::show(str(L"Unable to instantiate render system \"%ls\",\nNo such type.", renderSystemTypeName.c_str()), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	m_renderSystem = dynamic_type_cast< IRenderSystem* >(renderSystemType->createInstance());
	T_ASSERT(m_renderSystem);

	Ref< RenderSystemVrfy > renderSystemVrfy = new RenderSystemVrfy(settings->getProperty< bool >(L"Editor.UseRenderDoc", false));

	RenderSystemDesc desc;
	desc.capture = m_renderSystem;
	desc.mipBias = settings->getProperty< float >(L"Editor.MipBias", 0.0f);
	desc.maxAnisotropy = settings->getProperty< int32_t >(L"Editor.MaxAnisotropy", 1);
	desc.maxAnisotropy = std::max(desc.maxAnisotropy, 1);
	desc.validation = settings->getProperty< bool >(L"Editor.RenderValidation", true);
	desc.programCache = settings->getProperty< bool >(L"Editor.UseProgramCache", false);
	if (!renderSystemVrfy->create(desc))
	{
		ui::MessageBox::show(str(L"Unable to instantiate render system \"%ls\",\nFailed to initialize render system.", renderSystemTypeName.c_str()), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	m_editor->getObjectStore()->set(renderSystemVrfy);
	return true;
}

}
