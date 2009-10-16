#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/ReferencesRenderControl.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "I18N/Text.h"
#include "Core/Heap/GcNew.h"
#include "Core/Misc/String.h"

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

	m_container = gc_new< ui::Container >();
	if (!m_container->create(parent, ui::WsClientBorder, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = gc_new< ui::custom::ToolBar >();
	if (!m_toolBar->create(m_container))
		return false;

	Ref< editor::Settings > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	int32_t viewType = settings->getProperty< editor::PropertyInteger >(L"SceneEditor.View" + toString(index), 0);

	m_toolButtonView = gc_new< ui::custom::ToolBarButton >(L"N/A", ui::Command(), 0, ui::custom::ToolBarButton::BsText);
	m_toolBar->addItem(m_toolButtonView);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &DefaultRenderControl::eventToolClick));

	createRenderControl(viewType);

	return true;
}

void DefaultRenderControl::destroy()
{
	m_toolButtonView = 0;

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

void DefaultRenderControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;

	if (m_renderControl)
		m_renderControl->setWorldRenderSettings(worldRenderSettings);
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
			Ref< PerspectiveRenderControl > renderControl = gc_new< PerspectiveRenderControl >();
			if (renderControl->create(m_container, m_context, m_index))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 1:	// Front
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveZ))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 2:	// Back
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeZ))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 3:	// Top
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveY))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 4:	// Bottom
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeY))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 5:	// Left
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveX))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 6:	// Right
		{
			Ref< OrthogonalRenderControl > renderControl = gc_new< OrthogonalRenderControl >();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeX))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 7:	// References
		{
			Ref< ReferencesRenderControl > renderControl = gc_new< ReferencesRenderControl >();
			if (renderControl->create(m_container, m_context))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_REFERENCES"));
				m_renderControl = renderControl;
			}
		}
		break;

	case 8:	// Debug
		{
			Ref< DebugRenderControl > renderControl = gc_new< DebugRenderControl >();
			if (renderControl->create(m_container, m_context))
			{
				m_toolButtonView->setText(i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG"));
				m_renderControl = renderControl;
			}
		}
		break;
	}

	T_ASSERT (m_renderControl);

	m_container->update();
	m_renderControl->setWorldRenderSettings(m_worldRenderSettings);

	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyInteger >(L"SceneEditor.View" + toString(m_index), type);
}

void DefaultRenderControl::eventToolClick(ui::Event* event)
{
	Ref< ui::PopupMenu > popupMenu = gc_new< ui::PopupMenu >();
	if (popupMenu->create())
	{
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(0), i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(1), i18n::Text(L"SCENE_EDITOR_VIEW_FRONT")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(2), i18n::Text(L"SCENE_EDITOR_VIEW_BACK")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(3), i18n::Text(L"SCENE_EDITOR_VIEW_TOP")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(4), i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(5), i18n::Text(L"SCENE_EDITOR_VIEW_LEFT")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(6), i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT")));
		popupMenu->add(gc_new< ui::MenuItem >(L"-"));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(7), i18n::Text(L"SCENE_EDITOR_VIEW_REFERENCES")));
		popupMenu->add(gc_new< ui::MenuItem >(ui::Command(8), i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG")));

		Ref< ui::MenuItem > selectedItem = popupMenu->show(m_toolBar, ui::Point(0, 0));
		if (selectedItem)
		{
			uint32_t id = selectedItem->getCommand().getId();
			createRenderControl(id);
		}

		popupMenu->destroy();
	}
}

	}
}
