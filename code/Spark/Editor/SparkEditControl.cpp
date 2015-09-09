#pragma optimize( "", off )

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Spark/Character.h"
#include "Spark/DisplayList.h"
#include "Spark/DisplayRenderer.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditControl", SparkEditControl, ui::Widget)

SparkEditControl::SparkEditControl()
{
}

bool SparkEditControl::create(
	ui::Widget* parent,
	int style,
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!Widget::create(parent, style))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem))
		return false;

	m_displayRenderer = new DisplayRenderer();
	m_displayRenderer->create(1);

	addEventHandler< ui::SizeEvent >(this, &SparkEditControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &SparkEditControl::eventPaint);

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &SparkEditControl::eventIdle);

	m_database = database;
	m_resourceManager = resourceManager;

	return true;
}

void SparkEditControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	Widget::destroy();
}

void SparkEditControl::setRootCharacter(Character* character)
{
	m_character = character;
}

void SparkEditControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();
	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}
}

void SparkEditControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	if (m_renderView->begin(render::EtCyclop))
	{
		const static Color4f clearColor(1.0f, 1.0f, 1.0f, 0.0);
		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&clearColor,
			1.0f,
			0
		);

		//Matrix44 projection = orthoLh(sz.cx, sz.cy, 0.0f, 1.0f);

		//if (m_primitiveRenderer->begin(m_renderView, projection))
		//{
		//	m_primitiveRenderer->pushDepthState(false, false, false);

		//	for (int32_t i = -10; i < 10; ++i)
		//	{
		//		float f = i / 10.0f;
		//		m_primitiveRenderer->drawLine(
		//			Vector4(-1.0f, f, 1.0f, 1.0f),
		//			Vector4( 1.0f, f, 1.0f, 1.0f),
		//			Color4ub(220, 220, 220, 255)
		//		);
		//		m_primitiveRenderer->drawLine(
		//			Vector4(f, -1.0f, 1.0f, 1.0f),
		//			Vector4(f,  1.0f, 1.0f, 1.0f),
		//			Color4ub(220, 220, 220, 255)
		//		);
		//	}

		//	m_primitiveRenderer->popDepthState();

		//	m_primitiveRenderer->end();
		//}


		if (m_character)
		{
			Ref< CharacterInstance > characterInstance = m_character->createInstance(0, m_resourceManager);
			if (characterInstance)
			{
				DisplayList displayList;
				displayList.place(0, characterInstance);

				m_displayRenderer->build(&displayList, 0);
				m_displayRenderer->render(
					m_renderView,
					Vector2(0.0f, 0.0f),
					Vector2(sz.cx, sz.cy),
					0
				);
			}
		}


		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void SparkEditControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

	}
}
