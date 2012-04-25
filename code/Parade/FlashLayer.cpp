#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Log/Log.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/ISoundRenderer.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Parade/FlashLayer.h"
#include "Parade/Stage.h"
#include "Render/IRenderView.h"
#include "Script/Any.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.FlashLayer", FlashLayer, Layer)

FlashLayer::FlashLayer(
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< flash::FlashMovie >& movie,
	bool clearBackground
)
:	Layer(name, scriptContext)
,	m_environment(environment)
,	m_movie(movie)
,	m_clearBackground(clearBackground)
{
}

void FlashLayer::update(Stage* stage, const amalgam::IUpdateInfo& info)
{
	std::wstring command, args;

	if (m_movie.changed())
	{
		m_displayRenderer = 0;
		m_moviePlayer = 0;
		m_movie.consume();
	}

	// Re-create movie player.
	if (!m_moviePlayer)
	{
		createMoviePlayer();
		if (!m_moviePlayer)
			return;

		// Flush script to ensure it will get recreated as-well.
		flushScript();
	}

	// Issue script update method.
	invokeScriptUpdate(stage, info);

	// Update movie player.
	m_moviePlayer->progressFrame(info.getSimulationDeltaTime());

	// Dispatch "fscommand"s to script.
	while (m_moviePlayer->getFsCommand(command, args))
	{
		script::Any argv[] =
		{
			script::Any(stage),
			script::Any(args)
		};
		invokeScriptMethod(stage, command, sizeof_array(argv), argv);
	}
}

void FlashLayer::build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_displayRenderer)
		return;

	m_displayRenderer->build(frame);
	m_moviePlayer->renderFrame();
}

void FlashLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
{
	if (!m_displayRenderer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	m_displayRenderer->render(
		renderView,
		frame,
		eye
	);
}

void FlashLayer::leave(Stage* stage)
{
}

void FlashLayer::reconfigured(Stage* stage)
{
	if (!m_moviePlayer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	m_moviePlayer->postViewResize(width, height);
}

flash::FlashMoviePlayer* FlashLayer::getMoviePlayer()
{
	return m_moviePlayer;
}

flash::ActionObject* FlashLayer::getGlobal()
{
	if (!m_moviePlayer)
		return 0;

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return cx->getGlobal();
}

flash::ActionObject* FlashLayer::getRoot()
{
	if (!m_moviePlayer)
		return 0;

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return movieInstance->getAsObject(cx);
}

void FlashLayer::createMoviePlayer()
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create accelerated Flash renderer.
	Ref< flash::AccDisplayRenderer > displayRenderer = new flash::AccDisplayRenderer();
	if (!displayRenderer->create(
		m_environment->getResource()->getResourceManager(),
		m_environment->getRender()->getRenderSystem(),
		m_environment->getWorld()->getFrameCount(),
		m_clearBackground,
		0.006f
	))
	{
		log::error << L"Unable to create display renderer" << Endl;
		return;
	}

	// Create sound Flash renderer.
	Ref< flash::SoundRenderer > soundRenderer;
	if (m_environment->getAudio())
	{
		soundRenderer = new flash::SoundRenderer();
		soundRenderer->create(m_environment->getAudio()->getSoundSystem());
	}

	// Create Flash movie player.
	Ref< flash::FlashMoviePlayer > moviePlayer = new flash::FlashMoviePlayer(displayRenderer, soundRenderer);
	if (!moviePlayer->create(m_movie, width, height))
	{
		log::error << L"Unable to create movie player" << Endl;
		return;
	}

	// All success, replace instances.
	m_displayRenderer = displayRenderer;
	m_moviePlayer = moviePlayer;
}

	}
}
