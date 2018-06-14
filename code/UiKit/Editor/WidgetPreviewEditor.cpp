#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "Flash/MovieResourceFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/ScriptChunk.h"
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
	Ref< WidgetScaffolding > ws = mandatory_non_null_type_cast< WidgetScaffolding* >(object);

	// Create LUA script manager.
	m_scriptManager = new script::ScriptManagerLua();

	// Register all runtime classes, first collect all classes
	// and then register them in class dependency order.
	OrderedClassRegistrar registrar;
	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}
	registrar.registerClassesInOrder(m_scriptManager);

	// Create script context.
	m_scriptContext = m_scriptManager->createContext(false);
	if (!m_scriptContext)
		return false;

	// Create resource manager.
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();
	if (!database)
		return false;

	m_resourceManager = new resource::ResourceManager(database, true);
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	m_resourceManager->addFactory(new script::ScriptFactory(m_scriptManager, m_scriptContext));
	m_resourceManager->addFactory(new flash::MovieResourceFactory());

	// Create preview control.
	m_previewControl = new WidgetPreviewControl(m_editor, m_resourceManager, renderSystem);
	if (!m_previewControl->create(parent))
		return false;

	// Bind widget scaffolding.
	m_previewControl->setScaffolding(ws);

	resource::Proxy< IRuntimeClass > scaffoldingClass;
	if (m_resourceManager->bind(ws->getScaffoldingClass(), scaffoldingClass))
		(scaffoldingClass);

	return true;
}

void WidgetPreviewEditor::destroy()
{
	safeDestroy(m_previewControl);
	safeDestroy(m_resourceManager);
	safeDestroy(m_scriptContext);
	safeDestroy(m_scriptManager);
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
