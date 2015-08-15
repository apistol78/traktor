#ifndef traktor_flash_FlashPreviewControl_H
#define traktor_flash_FlashPreviewControl_H

#define T_USE_ACCELERATED_RENDERER 1

#include "Core/Timer/Timer.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class Shader;

	}

	namespace sound
	{

class SoundSystem;

	}

	namespace graphics
	{

class IGraphicsSystem;

	}

	namespace flash
	{

class AccDisplayRenderer;
class FlashMovie;
class FlashMoviePlayer;
class SoundRenderer;
class SwDisplayRenderer;

class T_DLLCLASS FlashPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	FlashPreviewControl();

	bool create(
		ui::Widget* parent,
		int style,
		db::Database* database,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::SoundSystem* soundSystem
	);

	void destroy();

	void setMovie(FlashMovie* movie);

	void rewind();

	void play();

	void stop();

	void forward();

	bool playing() const;

	void setWireFrame(bool wireframe);

	virtual ui::Size getPreferedSize() const;

	FlashMoviePlayer* getMoviePlayer() const { return m_moviePlayer; }

private:
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< db::Database > m_database;
#if T_USE_ACCELERATED_RENDERER
	Ref< render::IRenderView > m_renderView;
	Ref< AccDisplayRenderer > m_displayRenderer;
#else
	Ref< graphics::IGraphicsSystem > m_graphicsSystem;
	Ref< SwDisplayRenderer > m_displayRenderer;
#endif
	Ref< SoundRenderer > m_soundRenderer;
	Ref< FlashMoviePlayer > m_moviePlayer;
	Ref< FlashMovie > m_movie;
	Timer m_timer;
	bool m_playing;
	bool m_wireframe;

	ui::Point getTwips(const ui::Point& pt) const;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);

	void eventKey(ui::KeyEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);

	void eventKeyUp(ui::KeyUpEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

	}
}

#endif	// traktor_flash_FlashPreviewControl_H
