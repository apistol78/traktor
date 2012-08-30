#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/ISoundRenderer.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Parade/FlashLayer.h"
#include "Parade/Stage.h"
#include "Parade/Action/Classes/As_traktor_parade_Configuration.h"
#include "Parade/Action/Classes/As_traktor_parade_DisplayMode.h"
#include "Parade/Action/Classes/As_traktor_parade_InputFabricator.h"
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
	bool clearBackground,
	bool enableSound
)
:	Layer(name, scriptContext)
,	m_environment(environment)
,	m_movie(movie)
,	m_clearBackground(clearBackground)
,	m_enableSound(enableSound)
,	m_visible(true)
,	m_lastX(-1)
,	m_lastY(-1)
,	m_lastButton(0)
{
}

void FlashLayer::prepare(Stage* stage)
{
	if (m_movie.changed())
	{
		m_displayRenderer = 0;
		m_soundRenderer = 0;
		m_moviePlayer = 0;
		m_movie.consume();
	}

	// Re-create movie player.
	if (!m_moviePlayer)
	{
		createMoviePlayer();
		if (!m_moviePlayer)
			return;

		flushScript();
	}
}

void FlashLayer::update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	input::InputSystem* inputSystem = m_environment->getInput()->getInputSystem();
	std::wstring command, args;

	// Issue script update method.
	invokeScriptUpdate(stage, control, info);

	// Propagate mouse input to movie.
	input::IInputDevice* mouseDevice = inputSystem->getDevice(input::CtMouse, 0, true);
	if (mouseDevice)
	{
		int32_t positionX, positionY;
		mouseDevice->getDefaultControl(input::DtPositionX, true, positionX);
		mouseDevice->getDefaultControl(input::DtPositionY, true, positionY);

		int32_t button1, button2;
		mouseDevice->getDefaultControl(input::DtButton1, false, button1);
		mouseDevice->getDefaultControl(input::DtButton2, false, button2);

		float minX, minY;
		float maxX, maxY;
		mouseDevice->getControlRange(positionX, minX, maxX);
		mouseDevice->getControlRange(positionY, minY, maxY);

		if (maxX > minX && maxY > minY)
		{
			float x = mouseDevice->getControlValue(positionX);
			float y = mouseDevice->getControlValue(positionY);

			x = (x - minX) / (maxX - minX);
			y = (y - minY) / (maxY - minY);

			int32_t mx = int32_t(renderView->getWidth() * x);
			int32_t my = int32_t(renderView->getHeight() * y);

			int32_t mb =
				(mouseDevice->getControlValue(button1) > 0.5f ? 1 : 0) |
				(mouseDevice->getControlValue(button2) > 0.5f ? 2 : 0);

			if (mx != m_lastX || my != m_lastY)
			{
				m_moviePlayer->postMouseMove(mx, my, mb);
				m_lastX = mx;
				m_lastY = my;
			}

			if (mb != m_lastButton)
			{
				if (mb)
					m_moviePlayer->postMouseDown(mx, my, mb);
				else
					m_moviePlayer->postMouseUp(mx, my, mb);

				m_lastButton = mb;
			}
		}
	}

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
	if (!m_displayRenderer || !m_visible)
		return;

	m_displayRenderer->build(frame);
	m_moviePlayer->renderFrame();
}

void FlashLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
{
	if (!m_displayRenderer || !m_visible)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	m_displayRenderer->render(
		renderView,
		frame,
		eye
	);
}

void FlashLayer::leave(Stage* stage)
{
	m_movie.clear();
	safeDestroy(m_moviePlayer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_soundRenderer);
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
	if (m_enableSound && m_environment->getAudio())
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

	// Register additional AS classes.
	flash::ActionContext* context = moviePlayer->getMovieInstance()->getContext();
	T_ASSERT (context);

	Ref< flash::ActionObject > asTraktor = moviePlayer->getGlobal("traktor").getObject();
	if (!asTraktor)
		asTraktor = new flash::ActionObject(context);
	{
		Ref< flash::ActionObject > asParade = new flash::ActionObject(context);
		{
			asParade->setMember("Configuration", flash::ActionValue(new As_traktor_parade_Configuration(context, m_environment)));
			asParade->setMember("DisplayMode", flash::ActionValue(new As_traktor_parade_DisplayMode(context, m_environment)));
			asParade->setMember("InputFabricator", flash::ActionValue(new As_traktor_parade_InputFabricator(context, m_environment)));
		}
		asTraktor->setMember("parade", flash::ActionValue(asParade));
	}
	moviePlayer->setGlobal("traktor", flash::ActionValue(asTraktor));

	// Execute first frame.
	while (!moviePlayer->progressFrame(1.0f / 60.0f));

	// All success, replace instances.
	m_displayRenderer = displayRenderer;
	m_soundRenderer = soundRenderer;
	m_moviePlayer = moviePlayer;
}

	}
}
