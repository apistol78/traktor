#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Editor/App/DiscoveryPlugin.h"
#include "Net/Discovery/DiscoveryManager.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DiscoveryPlugin", DiscoveryPlugin, IEditorPlugin)

DiscoveryPlugin::DiscoveryPlugin(IEditor* editor)
:	m_editor(editor)
{
}

bool DiscoveryPlugin::create(ui::Widget* parent, IEditorPageSite* site)
{
	const PropertyGroup* settings = m_editor->getSettings();

	uint32_t mode = net::MdFindServices;
	if (settings->getProperty< bool >(L"Editor.Discoverable", true))
		mode |= net::MdPublishServices;

	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(mode);
	
	m_editor->setStoreObject(L"DiscoveryManager", m_discoveryManager);
	return true;
}

void DiscoveryPlugin::destroy()
{
	m_editor->setStoreObject(L"DiscoveryManager", nullptr);
	safeDestroy(m_discoveryManager);
}

bool DiscoveryPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void DiscoveryPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void DiscoveryPlugin::handleWorkspaceOpened()
{
}

void DiscoveryPlugin::handleWorkspaceClosed()
{
}

void DiscoveryPlugin::handleEditorClosed()
{
}

}
