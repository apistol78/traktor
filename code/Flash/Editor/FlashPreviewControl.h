#ifndef traktor_flash_FlashPreviewControl_H
#define traktor_flash_FlashPreviewControl_H

#define T_USE_ACCELERATED_RENDERER	1

#include "Ui/Widget.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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

class RenderSystem;
class RenderView;
class Shader;
class RenderTargetSet;

	}
#endif

	namespace graphics
	{

class GraphicsSystem;

	}

	namespace flash
	{

class FlashMovie;
class AccDisplayRenderer;
class SwDisplayRenderer;
class FlashMoviePlayer;

class T_DLLCLASS FlashPreviewControl : public ui::Widget
{
	T_RTTI_CLASS(FlashPreviewControl)

public:
	FlashPreviewControl();

	bool create(ui::Widget* parent, int style, resource::IResourceManager* resourceManager, render::RenderSystem* renderSystem);

	void destroy();

	void setMovie(FlashMovie* movie);

	void rewind();

	void play();

	void stop();

	void forward();

	virtual ui::Size getPreferedSize() const;

private:
#if T_USE_ACCELERATED_RENDERER
	Ref< render::RenderSystem > m_renderSystem;
	Ref< render::RenderView > m_renderView;
	Ref< AccDisplayRenderer > m_displayRenderer;
#else
	Ref< graphics::GraphicsSystem > m_graphicsSystem;
	Ref< SwDisplayRenderer > m_displayRenderer;
#endif
	Ref< ui::EventHandler > m_idleHandler;
	Ref< FlashMoviePlayer > m_moviePlayer;
	Ref< FlashMovie > m_movie;
	Timer m_timer;
	float m_frameLength;
	bool m_playing;

	ui::Point getTwips(const ui::Point& pt) const;

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventIdle(ui::Event* event);

	void eventKeyDown(ui::Event* event);

	void eventKeyUp(ui::Event* event);

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);
};

	}
}

#endif	// traktor_flash_FlashPreviewControl_H
