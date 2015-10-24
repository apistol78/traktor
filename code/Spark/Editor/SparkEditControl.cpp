#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Spark/SpriteData.h"
#include "Spark/Character.h"
#include "Spark/SparkRenderer.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/EditContext.h"
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
		namespace
		{

int32_t viewGridOffset(int32_t x, int32_t spacing)
{
	return (x / spacing) * spacing;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditControl", SparkEditControl, ui::Widget)

SparkEditControl::SparkEditControl(editor::IEditor* editor, editor::IEditorPageSite* site, EditContext* editContext)
:	m_editor(editor)
,	m_site(site)
,	m_editContext(editContext)
,	m_viewOffset(1920.0f, 1080.0f)
,	m_viewScale(0.3f)
,	m_panView(false)
{
}

bool SparkEditControl::create(
	ui::Widget* parent,
	int style,
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

	m_sparkRenderer = new SparkRenderer();
	m_sparkRenderer->create(1);

	addEventHandler< ui::SizeEvent >(this, &SparkEditControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &SparkEditControl::eventPaint);
	addEventHandler< ui::MouseButtonDownEvent >(this, &SparkEditControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &SparkEditControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &SparkEditControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &SparkEditControl::eventMouseWheel);

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &SparkEditControl::eventIdle);
	return true;
}

void SparkEditControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);
	safeDestroy(m_sparkRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	Widget::destroy();
}

Vector2 SparkEditControl::clientToView(const ui::Point& point) const
{
	ui::Size sz = getInnerRect().getSize();

	float viewWidth = sz.cx / m_viewScale;
	float viewHeight = sz.cy / m_viewScale;

	float viewX = m_viewOffset.x / viewWidth;
	float viewY = m_viewOffset.y / viewHeight;

	Matrix44 projection(
		2.0f / viewWidth, 0.0f, 0.0f, -viewX,
		0.0f, -2.0f / viewHeight, 0.0f, viewY,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Vector4 clientPosition(
		2.0f * float(point.x) / sz.cx - 1.0f,
		1.0f - 2.0f * float(point.y) / sz.cy,
		0.0f,
		1.0f
	);

	Vector4 viewPosition = projection.inverse() * clientPosition;
	return Vector2(
		viewPosition.x(),
		viewPosition.y()
	);
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

		float viewWidth = sz.cx / m_viewScale;
		float viewHeight = sz.cy / m_viewScale;

		float viewX = m_viewOffset.x / viewWidth;
		float viewY = m_viewOffset.y / viewHeight;

		Matrix44 projection(
			2.0f / viewWidth, 0.0f, 0.0f, -viewX,
			0.0f, -2.0f / viewHeight, 0.0f, viewY,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		if (m_primitiveRenderer->begin(m_renderView, projection))
		{
			m_primitiveRenderer->pushDepthState(false, false, false);

			const Aabb2& bounds = mandatory_non_null_type_cast< const SpriteData* >(m_editContext->getRoot()->getCharacterData())->getBounds();
			if (!bounds.empty())
			{
				for (int32_t x = int32_t(bounds.mn.x); x < int32_t(bounds.mx.x); x += m_editContext->getGridSpacing())
				{
					m_primitiveRenderer->drawLine(
						Vector4(x, bounds.mn.y, 1.0f, 1.0f),
						Vector4(x, bounds.mx.y, 1.0f, 1.0f),
						Color4ub(0, 0, 0, 40)
					);
				}
				for (int32_t y = int32_t(bounds.mn.y); y < int32_t(bounds.mx.y); y += m_editContext->getGridSpacing())
				{
					m_primitiveRenderer->drawLine(
						Vector4(bounds.mn.x, y, 1.0f, 1.0f),
						Vector4(bounds.mx.x, y, 1.0f, 1.0f),
						Color4ub(0, 0, 0, 40)
					);
				}

				m_primitiveRenderer->drawLine(Vector4(bounds.mn.x, bounds.mn.y, 1.0f, 1.0f), Vector4(bounds.mx.x, bounds.mn.y, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
				m_primitiveRenderer->drawLine(Vector4(bounds.mx.x, bounds.mn.y, 1.0f, 1.0f), Vector4(bounds.mx.x, bounds.mx.y, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
				m_primitiveRenderer->drawLine(Vector4(bounds.mx.x, bounds.mx.y, 1.0f, 1.0f), Vector4(bounds.mn.x, bounds.mx.y, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
				m_primitiveRenderer->drawLine(Vector4(bounds.mn.x, bounds.mx.y, 1.0f, 1.0f), Vector4(bounds.mn.x, bounds.mn.y, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			}
			else
			{
				Aabb2 viewBounds(
					Vector2(m_viewOffset.x / 2.0f - viewWidth / 2.0f, m_viewOffset.y / 2.0f - viewHeight / 2.0f),
					Vector2(m_viewOffset.x / 2.0f + viewWidth / 2.0f, m_viewOffset.y / 2.0f + viewHeight / 2.0f)
				);
				for (int32_t x = viewGridOffset(int32_t(viewBounds.mn.x), m_editContext->getGridSpacing()); x < int32_t(viewBounds.mx.x); x += m_editContext->getGridSpacing())
				{
					m_primitiveRenderer->drawLine(
						Vector4(x, viewBounds.mn.y, 1.0f, 1.0f),
						Vector4(x, viewBounds.mx.y, 1.0f, 1.0f),
						Color4ub(0, 0, 0, 40)
					);
				}
				for (int32_t y = viewGridOffset(int32_t(viewBounds.mn.y), m_editContext->getGridSpacing()); y < int32_t(viewBounds.mx.y); y += m_editContext->getGridSpacing())
				{
					m_primitiveRenderer->drawLine(
						Vector4(viewBounds.mn.x, y, 1.0f, 1.0f),
						Vector4(viewBounds.mx.x, y, 1.0f, 1.0f),
						Color4ub(0, 0, 0, 40)
					);
				}
			}

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->end();
		}

		// Draw sprites.
		if (m_sparkRenderer && m_editContext->getRoot())
		{
			m_sparkRenderer->build(m_editContext->getRoot()->getCharacter(), 0);
			m_sparkRenderer->render(m_renderView, projection, 0);

			// Draw bounding boxes.
			if (m_primitiveRenderer->begin(m_renderView, projection))
			{
				m_primitiveRenderer->pushDepthState(false, false, false);

				const RefArray< CharacterAdapter >& adapters = m_editContext->getAdapters();
				for (RefArray< CharacterAdapter >::const_iterator i = adapters.begin(); i != adapters.end(); ++i)
					(*i)->paint(m_primitiveRenderer);

				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->end();
			}
		}

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void SparkEditControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	Vector2 viewPosition = clientToView(event->getPosition());
	
	if ((event->getKeyState() & ui::KsMenu) == 0)
	{
		const RefArray< CharacterAdapter >& adapters = m_editContext->getAdapters();
		for (RefArray< CharacterAdapter >::const_iterator i = adapters.begin(); i != adapters.end(); ++i)
			(*i)->mouseDown(this, viewPosition);
	}
	else
	{
		m_lastMousePosition = event->getPosition();
		m_panView = true;
	}

	setCapture();
	update();
}

void SparkEditControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	Vector2 viewPosition = clientToView(event->getPosition());

	if (!m_panView)
	{
		const RefArray< CharacterAdapter >& adapters = m_editContext->getAdapters();
		for (RefArray< CharacterAdapter >::const_iterator i = adapters.begin(); i != adapters.end(); ++i)
			(*i)->mouseUp(this, viewPosition);
	}

	m_panView = false;

	releaseCapture();
	update();
}

void SparkEditControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	Vector2 viewPosition = clientToView(event->getPosition());

	if (!m_panView)
	{
		const RefArray< CharacterAdapter >& adapters = m_editContext->getAdapters();
		for (RefArray< CharacterAdapter >::const_iterator i = adapters.begin(); i != adapters.end(); ++i)
			(*i)->mouseMove(this, viewPosition);
	}
	else
	{
		ui::Point mousePosition = event->getPosition();
		Vector2 deltaMove(
			-(mousePosition.x - m_lastMousePosition.x),
			-(mousePosition.y - m_lastMousePosition.y)
		);
		m_viewOffset += 2.0f * deltaMove / m_viewScale;
		m_lastMousePosition = mousePosition;
	}

	update();
}

void SparkEditControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_viewScale += event->getRotation() * 0.1f;
	m_viewScale = clamp(m_viewScale, 0.1f, 1000.0f);
	update();
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
