#pragma optimize( "", off )

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
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

SparkEditControl::SparkEditControl(editor::IEditor* editor)
:	m_editor(editor)
,	m_viewOffset(0.0f, 0.0f)
,	m_viewScale(1.0f)
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

	addEventHandler< ui::MouseButtonDownEvent >(this, &SparkEditControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &SparkEditControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &SparkEditControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &SparkEditControl::eventMouseWheel);
	addEventHandler< ui::SizeEvent >(this, &SparkEditControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &SparkEditControl::eventPaint);

	m_database = database;
	m_resourceManager = resourceManager;

	return true;
}

void SparkEditControl::destroy()
{
	safeDestroy(m_displayRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	Widget::destroy();
}

void SparkEditControl::setRootCharacter(Character* character)
{
	m_character = character;
	update();
}

void SparkEditControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void SparkEditControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void SparkEditControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 deltaMove(
		 (mousePosition.x - m_lastMousePosition.x),
		-(mousePosition.y - m_lastMousePosition.y)
	);
	m_viewOffset += deltaMove / m_viewScale;

	m_lastMousePosition = mousePosition;
	update();
}

void SparkEditControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_viewScale += event->getRotation() * 0.1f;
	m_viewScale = clamp(m_viewScale, 0.1f, 1000.0f);
	update();
}

void SparkEditControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();
	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}
	update();
}

void SparkEditControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	if (m_renderView->begin(render::EtCyclop))
	{
		const PropertyGroup* settings = m_editor->getSettings();
		T_ASSERT (settings);

		Color4ub clearColor = settings->getProperty< PropertyColor >(L"Editor.Colors/Background");

		float c[4];
		clearColor.getRGBA32F(c);
		const Color4f cc(c);

		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&cc,
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
					m_viewOffset,
					Vector2(sz.cx / m_viewScale, sz.cy / m_viewScale),
					0
				);
			}
		}


		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
