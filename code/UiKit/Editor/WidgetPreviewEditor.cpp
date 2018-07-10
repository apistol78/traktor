#include "Core/Class/IRuntimeClass.h"
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "Flash/MovieResourceFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptChunk.h"
#include "Script/ScriptFactory.h"
#include "Ui/Application.h"
#include "UiKit/Editor/WidgetPreviewControl.h"
#include "UiKit/Editor/WidgetPreviewEditor.h"
#include "UiKit/Editor/WidgetScaffolding.h"
#include "Video/VideoFactory.h"

namespace traktor
{
	namespace uikit
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.uikit.WidgetPreviewEditor", WidgetPreviewEditor, editor::IObjectEditor)

WidgetPreviewEditor::WidgetPreviewEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool WidgetPreviewEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	Ref< WidgetScaffolding > ws = mandatory_non_null_type_cast< WidgetScaffolding* >(object);

	// Get systems and managers.
	Ref< script::IScriptManager > scriptManager = m_editor->getStoreObject< script::IScriptManager >(L"ScriptManager");
	if (!scriptManager)
		return false;

	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();
	if (!database)
		return false;

	// Create script context.
	m_scriptContext = scriptManager->createContext(false);
	if (!m_scriptContext)
		return false;

	// Create resource manager.
	m_resourceManager = new resource::ResourceManager(database, false);
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new script::ScriptFactory(m_scriptContext));
	m_resourceManager->addFactory(new flash::MovieResourceFactory());
	m_resourceManager->addFactory(new video::VideoFactory(renderSystem));

	// Create preview control.
	m_previewControl = new WidgetPreviewControl(m_editor, m_resourceManager, renderSystem);
	if (!m_previewControl->create(parent))
		return false;

	// Bind widget scaffolding.
	m_previewControl->setScaffolding(ws);
	return true;
}

void WidgetPreviewEditor::destroy()
{
	safeDestroy(m_previewControl);
	safeDestroy(m_resourceManager);
	safeDestroy(m_scriptContext);
}

void WidgetPreviewEditor::apply()
{
}

bool WidgetPreviewEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void WidgetPreviewEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (database == m_editor->getOutputDatabase())
	{
		if (m_resourceManager)
			m_resourceManager->reload(eventId, false);
	}
}

ui::Size WidgetPreviewEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(640),
		ui::dpi96(480)
	);
}

	}
}
