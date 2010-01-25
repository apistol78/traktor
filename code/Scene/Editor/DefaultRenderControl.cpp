#include "Core/Misc/String.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "I18N/Text.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/ReferencesRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultRenderControl", DefaultRenderControl, ISceneRenderControl)

bool DefaultRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t index)
{
	m_context = context;
	T_ASSERT (m_context);

	m_index = index;

	Ref< editor::Settings > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	int32_t viewType = settings->getProperty< editor::PropertyInteger >(L"SceneEditor.View" + toString(index), 0);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsClientBorder, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBarView = new ui::custom::ToolBarDropDown(ui::Command(), 100, i18n::Text(L"View mode"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_REFERENCES"));
	m_toolBarView->add(i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG"));
	m_toolBarView->select(viewType);

	m_toolBar->addItem(m_toolBarView);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &DefaultRenderControl::eventToolClick));

	createRenderControl(viewType);

	return true;
}

void DefaultRenderControl::destroy()
{
	m_toolBarView = 0;

	if (m_renderControl)
	{
		m_renderControl->destroy();
		m_renderControl = 0;
	}

	if (m_container)
	{
		m_container->destroy();
		m_container = 0;
	}
}

void DefaultRenderControl::updateWorldRenderer()
{
	if (m_renderControl)
		m_renderControl->updateWorldRenderer();
}

bool DefaultRenderControl::handleCommand(const ui::Command& command)
{
	if (m_renderControl)
		return m_renderControl->handleCommand(command);
	else
		return false;
}

void DefaultRenderControl::update()
{
	if (m_renderControl)
		m_renderControl->update();
}

void DefaultRenderControl::createRenderControl(int32_t type)
{
	if (m_renderControl)
	{
		m_renderControl->destroy();
		m_renderControl = 0;
	}

	switch (type)
	{
	case 0:
		{
			Ref< PerspectiveRenderControl > renderControl = new PerspectiveRenderControl();
			if (renderControl->create(m_container, m_context, m_index))
				m_renderControl = renderControl;
		}
		break;

	case 1:	// Front
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveZ))
				m_renderControl = renderControl;
		}
		break;

	case 2:	// Back
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeZ))
				m_renderControl = renderControl;
		}
		break;

	case 3:	// Top
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveY))
				m_renderControl = renderControl;
		}
		break;

	case 4:	// Bottom
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeY))
				m_renderControl = renderControl;
		}
		break;

	case 5:	// Left
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveX))
				m_renderControl = renderControl;
		}
		break;

	case 6:	// Right
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeX))
				m_renderControl = renderControl;
		}
		break;

	case 7:	// References
		{
			Ref< ReferencesRenderControl > renderControl = new ReferencesRenderControl();
			if (renderControl->create(m_container, m_context))
				m_renderControl = renderControl;
		}
		break;

	case 8:	// Debug
		{
			Ref< DebugRenderControl > renderControl = new DebugRenderControl();
			if (renderControl->create(m_container, m_context))
				m_renderControl = renderControl;
		}
		break;
	}

	T_ASSERT (m_renderControl);

	m_container->update();
	m_renderControl->updateWorldRenderer();

	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyInteger >(L"SceneEditor.View" + toString(m_index), type);
}

void DefaultRenderControl::eventToolClick(ui::Event* event)
{
	int32_t selected = m_toolBarView->getSelected();
	T_ASSERT (selected >= 0);
	createRenderControl(selected);
}

	}
}
