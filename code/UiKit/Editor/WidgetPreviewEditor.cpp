#include "Core/Class/IRuntimeClass.h"
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/ScriptFactory.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Ui/Application.h"
#include "UiKit/Editor/WidgetPreviewControl.h"
#include "UiKit/Editor/WidgetPreviewEditor.h"
#include "UiKit/Editor/WidgetScaffolding.h"

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
	// Create LUA script manager and context.
	Ref< script::IScriptManager > scriptManager = new script::ScriptManagerLua();

	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(true);
	if (!scriptContext)
		return false;

	// Create resource manager.
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();
	if (!database)
		return false;

	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(database, true);
	resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	resourceManager->addFactory(new script::ScriptFactory(scriptManager, scriptContext));

	// Create preview control.
	m_previewControl = new WidgetPreviewControl(m_editor);
	if (!m_previewControl->create(parent, resourceManager, renderSystem))
		return false;

	// Bind widget scaffolding.
	Ref< WidgetScaffolding > ws = mandatory_non_null_type_cast< WidgetScaffolding* >(object);

	resource::Proxy< IRuntimeClass > scaffoldingClass;
	if (resourceManager->bind(ws->getScaffoldingClass(), scaffoldingClass))
		m_previewControl->setScaffoldingClass(scaffoldingClass);

	return true;
}

void WidgetPreviewEditor::destroy()
{
	safeDestroy(m_previewControl);
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
