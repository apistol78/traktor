#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Flash/DefaultCharacterFactory.h"
#include "Flash/Frame.h"
#include "Flash/Movie.h"
#include "Flash/MoviePlayer.h"
#include "Flash/MovieRenderer.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/Common/Classes/AsKey.h"
#include "Flash/Debug/WireDisplayRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "UiKit/Editor/WidgetPreviewControl.h"
#include "UiKit/Editor/WidgetScaffolding.h"

namespace traktor
{
	namespace uikit
	{
		namespace
		{

const struct
{
	ui::VirtualKey vk;
	int32_t ak;
}
c_askeys[] =
{
	{ ui::VkBackSpace, flash::AsKey::AkBackspace },
	{ ui::VkControl, flash::AsKey::AkControl },
	{ ui::VkDelete, flash::AsKey::AkDeleteKey },
	{ ui::VkDown, flash::AsKey::AkDown },
	{ ui::VkEnd, flash::AsKey::AkEnd },
	{ ui::VkReturn, flash::AsKey::AkEnter },
	{ ui::VkEscape, flash::AsKey::AkEscape },
	{ ui::VkHome, flash::AsKey::AkHome },
	{ ui::VkInsert, flash::AsKey::AkInsert },
	{ ui::VkLeft, flash::AsKey::AkLeft },
	{ ui::VkPageDown, flash::AsKey::AkPgDn },
	{ ui::VkPageUp, flash::AsKey::AkPgUp },
	{ ui::VkRight, flash::AsKey::AkRight },
	{ ui::VkShift, flash::AsKey::AkShift },
	{ ui::VkSpace, flash::AsKey::AkSpace },
	{ ui::VkTab, flash::AsKey::AkTab },
	{ ui::VkUp, flash::AsKey::AkUp }
};

int32_t translateVirtualKey(ui::VirtualKey vk)
{
	for (int i = 0; i < sizeof_array(c_askeys); ++i)
	{
		if (vk == c_askeys[i].vk)
			return c_askeys[i].ak;
	}
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.uikit.WidgetPreviewControl", WidgetPreviewControl, ui::Widget)

WidgetPreviewControl::WidgetPreviewControl(editor::IEditor* editor, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_editor(editor)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

bool WidgetPreviewControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsNone))
		return false;

	// Create an empty flash movie.
	Ref< flash::Sprite > movieClip = new flash::Sprite(0, 60);
	Ref< flash::Frame > frame = new flash::Frame();
	frame->changeBackgroundColor(Color4f(0.2f, 0.2f, 0.2f, 1.0f));
	movieClip->addFrame(frame);
	m_movie = new flash::Movie(
		Aabb2(Vector2(0.0f, 0.0f), Vector2(1280.0f * 20.0f, 720.0f * 20.0f)),
		movieClip
	);

	// Create render view.
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	// Create flash display renderer.
	m_displayRenderer = new flash::AccDisplayRenderer();
	if (!m_displayRenderer->create(
		m_resourceManager,
		m_renderSystem,
		1,
		4 * 1024 * 1024,
		true,
		false,
		false,
		0.0f
	))
		return false;

	m_displayRendererWire = new flash::WireDisplayRenderer();
	if (!m_displayRendererWire->create(
		m_resourceManager,
		m_renderSystem,
		1
	))
	{
		log::error << L"Unable to create wire renderer." << Endl;
		return false;
	}

	// Create separate movie renderer for each display renderer.
	m_movieRenderer = new flash::MovieRenderer(m_displayRenderer, nullptr);
	m_movieRendererWire = new flash::MovieRenderer(m_displayRendererWire, nullptr);

	// Create flash movie player.
	int32_t width = m_renderView->getWidth();
	int32_t height = m_renderView->getHeight();

	m_moviePlayer = new flash::MoviePlayer(
		new flash::DefaultCharacterFactory(),
		nullptr,
		nullptr
	);
	if (!m_moviePlayer->create(m_movie, width, height, nullptr))
		return false;

	while (!m_moviePlayer->progress(1.0f / 60.0f, nullptr));

	// Add widget event handler.
	addEventHandler< ui::SizeEvent >(this, &WidgetPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &WidgetPreviewControl::eventPaint);
	addEventHandler< ui::KeyEvent >(this, &WidgetPreviewControl::eventKey);
	addEventHandler< ui::KeyDownEvent >(this, &WidgetPreviewControl::eventKeyDown);
	addEventHandler< ui::KeyUpEvent >(this, &WidgetPreviewControl::eventKeyUp);
	addEventHandler< ui::MouseButtonDownEvent >(this, &WidgetPreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &WidgetPreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &WidgetPreviewControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &WidgetPreviewControl::eventMouseWheel);

	// Register our idle event handler.
	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &WidgetPreviewControl::eventIdle);
	return true;
}

void WidgetPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	safeDestroy(m_displayRenderer);
	safeClose(m_renderView);

	ui::Widget::destroy();
}

void WidgetPreviewControl::setScaffolding(const WidgetScaffolding* scaffolding)
{
	m_scaffoldingObject = 0;
	if ((m_scaffolding = scaffolding) != 0)
		m_resourceManager->bind(m_scaffolding->getScaffoldingClass(), m_scaffoldingClass);
	else
		m_scaffoldingClass.clear();
}

void WidgetPreviewControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();

	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}

	if (m_moviePlayer)
		m_moviePlayer->postViewResize(sz.cx, sz.cy);
}

void WidgetPreviewControl::eventPaint(ui::PaintEvent* event)
{
	// Initialize scaffolding.
	if (m_scaffoldingClass.changed())
	{
		// Create scaffolding object.
		if (m_scaffoldingClass)
		{
			// Remove previous scaffolding.
			if (m_scaffoldingObject)
			{
				const IRuntimeDispatch* method = findRuntimeClassMethod(m_scaffoldingClass, "remove");
				if (method != 0)
					method->invoke(m_scaffoldingObject, 0, 0);
			}

			// Construct new scaffolding.
			Any argv[] =
			{
				Any::fromObject(m_editor->getSourceDatabase()),
				Any::fromObject(m_resourceManager),
				Any::fromObject(m_moviePlayer->getMovieInstance())
			};
			m_scaffoldingObject = createRuntimeClassInstance(m_scaffoldingClass, 0, sizeof_array(argv), argv);
		}
		else
			m_scaffoldingObject = 0;

		m_scaffoldingClass.consume();
	}

	if (!m_renderView || !m_moviePlayer)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
		if (m_scaffoldingObject)
		{
			const IRuntimeDispatch* method = findRuntimeClassMethod(m_scaffoldingClass, "update");
			if (method != 0)
				method->invoke(m_scaffoldingObject, 0, 0);
		}

		// Build render context.
		m_displayRenderer->build(uint32_t(0));
		m_displayRendererWire->begin(uint32_t(0));

		m_moviePlayer->render(m_movieRenderer);
		m_moviePlayer->render(m_movieRendererWire);

		m_displayRendererWire->end(uint32_t(0));

		// Flush render context.
		const Color4f clearColor(0.8f, 0.8f, 0.8f, 0.0);
		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&clearColor,
			1.0f,
			0
		);

		m_displayRenderer->render(m_renderView, 0, render::EtCyclop, Vector2(0.0f, 0.0f), 1.0f);
		m_displayRendererWire->render(m_renderView, uint32_t(0));

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void WidgetPreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (!m_moviePlayer)
		return;

	if (isVisible(true))
	{
		float deltaTime = float(m_timer.getDeltaTime());

		if (m_moviePlayer->progress(deltaTime, nullptr))
			update();

		event->requestMore();
	}
}

void WidgetPreviewControl::eventKey(ui::KeyEvent* event)
{
	if (event->getCharacter() != '\r' && m_moviePlayer)
		m_moviePlayer->postKey(event->getCharacter());
}

void WidgetPreviewControl::eventKeyDown(ui::KeyDownEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyDown(ak);
	}
}

void WidgetPreviewControl::eventKeyUp(ui::KeyUpEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyUp(ak);
	}
}

void WidgetPreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseDown(mousePosition.x, mousePosition.y, event->getButton());
	}
	setCapture();
	setFocus();
}

void WidgetPreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseUp(mousePosition.x, mousePosition.y, event->getButton());
	}
	releaseCapture();
}

void WidgetPreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseMove(mousePosition.x, mousePosition.y, event->getButton());
	}
}

void WidgetPreviewControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseWheel(mousePosition.x, mousePosition.y, event->getRotation());
	}
}

	}
}
