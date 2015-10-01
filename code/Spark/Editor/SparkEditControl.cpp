#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Spark/External.h"
#include "Spark/SparkPlayer.h"
#include "Spark/SparkRenderer.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
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

void drawBound(CharacterInstance* character, render::PrimitiveRenderer* primitiveRenderer)
{
	if (!character)
		return;

	Aabb2 bounds = character->getBounds();

	Matrix33 T = character->getFullTransform();

	Vector2 e[] =
	{
		T * bounds.mn,
		T * Vector2(bounds.mx.x, bounds.mn.y),
		T * bounds.mx,
		T * Vector2(bounds.mn.x, bounds.mx.y)
	};

	float cx = T.e13;
	float cy = T.e23;
	float r = 20.0f;

	primitiveRenderer->drawLine(
		Vector4(cx - r, cy, 1.0f, 1.0f),
		Vector4(cx + r, cy, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(cx, cy - r, 1.0f, 1.0f),
		Vector4(cx, cy + r, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);

	primitiveRenderer->drawLine(
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditControl", SparkEditControl, ui::Widget)

SparkEditControl::SparkEditControl(editor::IEditor* editor, editor::IEditorPageSite* site)
:	m_editor(editor)
,	m_site(site)
,	m_editMode(EmIdle)
,	m_viewWidth(1920)
,	m_viewHeight(1080)
,	m_viewOffset(1920.0f, 1080.0f)
,	m_viewScale(0.3f)
,	m_playing(false)
{
}

bool SparkEditControl::create(
	ui::Widget* parent,
	int style,
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::ISoundPlayer* soundPlayer
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
	addEventHandler< ui::KeyEvent >(this, &SparkEditControl::eventKey);
	addEventHandler< ui::KeyDownEvent >(this, &SparkEditControl::eventKeyDown);
	addEventHandler< ui::KeyUpEvent >(this, &SparkEditControl::eventKeyUp);
	addEventHandler< ui::MouseButtonDownEvent >(this, &SparkEditControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &SparkEditControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &SparkEditControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &SparkEditControl::eventMouseWheel);

	m_database = database;
	m_resourceManager = resourceManager;
	m_soundPlayer = soundPlayer;

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

void SparkEditControl::setSprite(Sprite* sprite)
{
	m_sprite = sprite;
	refresh();
}

void SparkEditControl::setViewSize(int32_t width, int32_t height)
{
	if (width != m_viewWidth || height != m_viewHeight)
	{
		m_viewWidth = width;
		m_viewHeight = height;
		if (m_sparkPlayer)
			m_sparkPlayer->postViewResize(width, height);
		update();
	}
}

void SparkEditControl::refresh()
{
	if (m_sprite)
		m_spriteInstance = checked_type_cast< SpriteInstance* >(m_sprite->createInstance(0, m_resourceManager, m_soundPlayer));
	else
		m_spriteInstance = 0;

	if (m_spriteInstance)
	{
		m_sparkPlayer = new SparkPlayer(m_spriteInstance);
		m_sparkPlayer->postViewResize(m_viewWidth, m_viewHeight);
	}
	else
		m_sparkPlayer = 0;

	m_site->setPropertyObject(m_sprite);

	update();
}

bool SparkEditControl::dropInstance(db::Instance* instance, const ui::Point& position)
{
	if (!is_type_of< Character >(*instance->getPrimaryType()))
		return false;

	ui::Point viewPosition = clientToView(screenToClient(position));

	m_sprite->place(
		L"",
		new External(resource::Id< Character >(instance->getGuid())),
		translate(viewPosition.x, viewPosition.y)
	);

	refresh();
	return true;
}

bool SparkEditControl::play()
{
	m_playing = true;
	return true;
}

bool SparkEditControl::stop()
{
	m_playing = false;
	return true;
}

bool SparkEditControl::rewind()
{
	refresh();
	stop();
	return true;
}

bool SparkEditControl::isPlaying() const
{
	return m_playing;
}

ui::Point SparkEditControl::clientToView(const ui::Point& point) const
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

	return ui::Point(
		int32_t(viewPosition.x()),
		int32_t(viewPosition.y())
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

			for (int32_t x = 0; x < m_viewWidth; x += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(x, 0.0f, 1.0f, 1.0f),
					Vector4(x, m_viewHeight, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}
			for (int32_t y = 0; y < m_viewHeight; y += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(0.0f, y, 1.0f, 1.0f),
					Vector4(m_viewWidth, y, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}

			m_primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, 1.0f, 1.0f), Vector4(m_viewWidth, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(m_viewWidth, 0.0f, 1.0f, 1.0f), Vector4(m_viewWidth, m_viewHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(m_viewWidth, m_viewHeight, 1.0f, 1.0f), Vector4(0.0f, m_viewHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(0.0f, m_viewHeight, 1.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->end();
		}

		// Update player if playing in editor.
		if (m_playing)
		{
			if (m_sparkPlayer)
				m_sparkPlayer->update();
		}

		// Draw sprites.
		if (m_sparkRenderer && m_spriteInstance)
		{
			m_sparkRenderer->build(m_spriteInstance, 0);
			m_sparkRenderer->render(m_renderView, projection, 0);

			// Draw bounding boxes of children to this sprite.
			if (m_primitiveRenderer->begin(m_renderView, projection))
			{
				m_primitiveRenderer->pushDepthState(false, false, false);

				RefArray< CharacterInstance > children;
				m_spriteInstance->getCharacters(children);
				for (RefArray< CharacterInstance >::const_iterator i = children.begin(); i != children.end(); ++i)
					drawBound(*i, m_primitiveRenderer);

				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->end();
			}
		}

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void SparkEditControl::eventKey(ui::KeyEvent* event)
{
	if (m_playing)
	{
		if (m_sparkPlayer)
			m_sparkPlayer->postKey(event->getCharacter());
	}
}

void SparkEditControl::eventKeyDown(ui::KeyDownEvent* event)
{
	if (m_playing)
	{
		if (m_sparkPlayer)
			m_sparkPlayer->postKeyDown(event->getVirtualKey());
	}
}

void SparkEditControl::eventKeyUp(ui::KeyUpEvent* event)
{
	if (m_playing)
	{
		if (m_sparkPlayer)
			m_sparkPlayer->postKeyUp(event->getVirtualKey());
	}
}

void SparkEditControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();

	if (m_playing)
	{
		if (m_sparkPlayer)
		{
			ui::Point mousePosition = clientToView(event->getPosition());
			m_sparkPlayer->postMouseDown(Vector2(mousePosition.x, mousePosition.y), event->getButton());
		}
	}
	else
	{
		m_editMode = EmPanView;
	}

	setCapture();
}

void SparkEditControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	if (m_playing)
	{
		if (m_sparkPlayer)
		{
			ui::Point mousePosition = clientToView(event->getPosition());
			m_sparkPlayer->postMouseUp(Vector2(mousePosition.x, mousePosition.y), event->getButton());
		}
	}
	else
	{
		m_editMode = EmIdle;
	}

	releaseCapture();
}

void SparkEditControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	ui::Point mousePosition = event->getPosition();

	if (m_playing)
	{
		if (m_sparkPlayer)
		{
			ui::Point mousePosition = clientToView(event->getPosition());
			m_sparkPlayer->postMouseMove(Vector2(mousePosition.x, mousePosition.y), event->getButton());
		}
	}
	else
	{
		if (m_editMode == EmPanView)
		{
			Vector2 deltaMove(
				-(mousePosition.x - m_lastMousePosition.x),
				-(mousePosition.y - m_lastMousePosition.y)
			);
			m_viewOffset += 2.0f * deltaMove / m_viewScale;
		}
	}

	m_lastMousePosition = mousePosition;
	update();
}

void SparkEditControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	if (m_playing)
	{
		if (m_sparkPlayer)
		{
			ui::Point mousePosition = clientToView(event->getPosition());
			m_sparkPlayer->postMouseWheel(Vector2(mousePosition.x, mousePosition.y), event->getRotation());
		}
	}
	else
	{
		m_viewScale += event->getRotation() * 0.1f;
		m_viewScale = clamp(m_viewScale, 0.1f, 1000.0f);
	}

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
