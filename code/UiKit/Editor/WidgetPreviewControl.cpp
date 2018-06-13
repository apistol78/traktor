#pragma optimize( "", off )

#include "Core/Class/IRuntimeClass.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IEditor.h"
#include "Flash/DefaultCharacterFactory.h"
#include "Flash/Frame.h"
#include "Flash/Movie.h"
#include "Flash/MoviePlayer.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "UiKit/Editor/WidgetPreviewControl.h"

namespace traktor
{
	namespace uikit
	{

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
	Ref< flash::Movie > movie = new flash::Movie(
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
	m_displayRenderer->create(
		m_resourceManager,
		m_renderSystem,
		1,
		4 * 1024 * 1024,
		true,
		false,
		false,
		0.0f
	);

	// Create flash movie player.
	int32_t width = m_renderView->getWidth();
	int32_t height = m_renderView->getHeight();

	m_moviePlayer = new flash::MoviePlayer(
		m_displayRenderer,
		0,
		new flash::DefaultCharacterFactory(),
		0,
		0
	);
	if (!m_moviePlayer->create(movie, width, height))
		return false;

	while (!m_moviePlayer->progressFrame(1.0f / 60.0f));

	// Add widget event handler.
	addEventHandler< ui::SizeEvent >(this, &WidgetPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &WidgetPreviewControl::eventPaint);

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

void WidgetPreviewControl::setScaffoldingClass(const IRuntimeClass* scaffoldingClass)
{
	Ref< flash::SpriteInstance > root = m_moviePlayer->getMovieInstance();

	Any argv[] =
	{
		Any::fromObject(m_resourceManager),
		Any::fromObject(root)
	};

	m_scaffoldingClass = scaffoldingClass;
	m_scaffolding = m_scaffoldingClass->construct(0, sizeof_array(argv), argv);
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
	if (!m_renderView || !m_moviePlayer)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
		// Build render context.
		m_displayRenderer->build(uint32_t(0));
		m_moviePlayer->renderFrame();

		// Flush render context.
		const Color4f clearColor(0.8f, 0.8f, 0.8f, 0.0);
		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&clearColor,
			1.0f,
			0
		);

		m_displayRenderer->render(m_renderView, 0, render::EtCyclop, Vector2(0.0f, 0.0f), 1.0f);

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

		if (m_moviePlayer->progressFrame(deltaTime))
			update();

		event->requestMore();
	}
}

	}
}
