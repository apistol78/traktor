#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Spark/MovieResourceFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptChunk.h"
#include "Script/ScriptFactory.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
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
	m_resourceManager = new resource::ResourceManager(database, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new script::ScriptFactory(m_scriptContext));
	m_resourceManager->addFactory(new spark::MovieResourceFactory());
	m_resourceManager->addFactory(new video::VideoFactory(renderSystem));

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0));

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(m_container);
	toolBar->addItem(new ui::ToolBarButton(
		L"Debug wires",
		ui::Command(L"UiKit.ToggleDebugWires"),
		ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle
	));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >([&](ui::ToolBarButtonClickEvent* event) {
		if (event->getCommand() == L"UiKit.ToggleDebugWires")
		{
			m_previewControl->setDebugWires(
				mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem())->isToggled()
			);
		}
	});

	// Create preview control.
	m_previewControl = new WidgetPreviewControl(m_editor, m_resourceManager, renderSystem);
	if (!m_previewControl->create(m_container))
		return false;
	m_previewControl->addEventHandler< ui::SizeEvent >(this, &WidgetPreviewEditor::eventPreviewSize);

	// Create status bar.
	m_statusBar = new ui::StatusBar();
	m_statusBar->create(m_container, ui::WsDoubleBuffer);
	m_statusBar->addColumn(-1);

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
	if (m_resourceManager && database == m_editor->getOutputDatabase())
	{
		if (m_resourceManager->reload(eventId, false))
			m_previewControl->invalidateScaffolding();
	}
}

ui::Size WidgetPreviewEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(1280),
		ui::dpi96(720)
	);
}

void WidgetPreviewEditor::eventPreviewSize(ui::SizeEvent* event)
{
	ui::Size innerSize = m_previewControl->getInnerRect().getSize();

	StringOutputStream ss;
	ss << innerSize.cx << L" * " << innerSize.cy;
	m_statusBar->setText(0, ss.str());
}

	}
}
