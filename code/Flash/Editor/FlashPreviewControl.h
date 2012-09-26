#ifndef traktor_flash_FlashPreviewControl_H
#define traktor_flash_FlashPreviewControl_H

#define T_USE_ACCELERATED_RENDERER	1

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
	namespace resource
	{

class IResourceManager;

	}

#if T_USE_ACCELERATED_RENDERER
	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class Shader;

	}
#endif

	namespace sound
	{

class SoundSystem;

	}

	namespace graphics
	{

class GraphicsSystem;

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

	virtual ui::Size getPreferedSize() const;

private:
	Ref< ui::EventHandler > m_idleHandler;
#if T_USE_ACCELERATED_RENDERER
	Ref< render::IRenderView > m_renderView;
	Ref< AccDisplayRenderer > m_displayRenderer;
#else
	Ref< graphics::GraphicsSystem > m_graphicsSystem;
	Ref< SwDisplayRenderer > m_displayRenderer;
#endif
	Ref< SoundRenderer > m_soundRenderer;
	Ref< FlashMoviePlayer > m_moviePlayer;
	Ref< FlashMovie > m_movie;
	Timer m_timer;
	bool m_playing;

	ui::Point getTwips(const ui::Point& pt) const;

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventIdle(ui::Event* event);

	void eventKey(ui::Event* event);

	void eventKeyDown(ui::Event* event);

	void eventKeyUp(ui::Event* event);

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);
};

	}
}

#endif	// traktor_flash_FlashPreviewControl_H
