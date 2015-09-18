#ifndef traktor_spark_SparkEditControl_H
#define traktor_spark_SparkEditControl_H

#include "Core/Math/Vector2.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;
class RenderTargetSet;
class Shader;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spark
	{

class CharacterRenderer;
class Sprite;
class SpriteInstance;

/*! \brief
 * \ingroup Spark
 */
class SparkEditControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	SparkEditControl(editor::IEditor* editor);

	bool create(
		ui::Widget* parent,
		int style,
		db::Database* database,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::ISoundPlayer* soundPlayer
	);

	void destroy();

	void setSprite(const Sprite* sprite);

	void refresh();

private:
	enum EditMode
	{
		EmIdle,
		EmPanView
	};

	editor::IEditor* m_editor;
	Ref< db::Database > m_database;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	Ref< CharacterRenderer > m_characterRenderer;
	Ref< const Sprite > m_sprite;
	Ref< SpriteInstance > m_spriteInstance;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	ui::Point m_lastMousePosition;
	EditMode m_editMode;
	Vector2 m_viewOffset;
	float m_viewScale;

	void eventMouseButtonDown(ui::MouseButtonDownEvent* event);

	void eventMouseButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_spark_SparkEditControl_H
