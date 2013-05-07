#include "Amalgam/FlashCast.h"
#include "Amalgam/FlashLayer.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Stage.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_Configuration.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_DisplayMode.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_InputFabricator.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/ISoundRenderer.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Action/Classes/BitmapData.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Render/IRenderView.h"
#include "Script/Any.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const struct InputKeyCode
{
	uint32_t inputKeyCode;
	uint32_t asKeyCode;
}
c_inputKeyCodes[] =
{
	{ input::DtKeyLeftControl, flash::AsKey::AkControl },
	{ input::DtKeyRightControl, flash::AsKey::AkControl },
	{ input::DtKeyDelete, flash::AsKey::AkDeleteKey },
	{ input::DtKeyDown, flash::AsKey::AkDown },
	{ input::DtKeyEnd, flash::AsKey::AkEnd },
	{ input::DtKeyReturn, flash::AsKey::AkEnter },
	{ input::DtKeyEscape, flash::AsKey::AkEscape },
	{ input::DtKeyHome, flash::AsKey::AkHome },
	{ input::DtKeyInsert, flash::AsKey::AkInsert },
	{ input::DtKeyLeft, flash::AsKey::AkLeft },
	{ input::DtKeyRight, flash::AsKey::AkRight },
	{ input::DtKeyLeftShift, flash::AsKey::AkShift },
	{ input::DtKeyRightShift, flash::AsKey::AkShift },
	{ input::DtKeySpace, flash::AsKey::AkSpace },
	{ input::DtKeyTab, flash::AsKey::AkTab },
	{ input::DtKeyUp, flash::AsKey::AkUp }
};

uint32_t translateInputKeyCode(uint32_t inputKeyCode)
{
	for (uint32_t i = 0; i < sizeof_array(c_inputKeyCodes); ++i)
	{
		if (c_inputKeyCodes[i].inputKeyCode == inputKeyCode)
			return c_inputKeyCodes[i].asKeyCode;
	}
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.FlashLayer", FlashLayer, Layer)

FlashLayer::FlashLayer(
	Stage* stage,
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< flash::FlashMovie >& movie,
	bool clearBackground,
	bool enableSound
)
:	Layer(stage, name)
,	m_environment(environment)
,	m_movie(movie)
,	m_clearBackground(clearBackground)
,	m_enableSound(enableSound)
,	m_visible(true)
,	m_lastX(-1)
,	m_lastY(-1)
,	m_lastButton(0)
,	m_lastWheel(0)
{
}

FlashLayer::~FlashLayer()
{
	destroy();
}

void FlashLayer::destroy()
{
	m_movie.clear();
	safeDestroy(m_moviePlayer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_soundRenderer);
}

void FlashLayer::prepare()
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
	}
}

void FlashLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	input::InputSystem* inputSystem = m_environment->getInput()->getInputSystem();
	std::string command, args;

	// Do NOT propagate input in case user is fabricating input.
	if (!m_environment->getInput()->isFabricating())
	{
		// Propagate keyboard input to movie.
		input::IInputDevice* keyboardDevice = inputSystem->getDevice(input::CtKeyboard, 0, true);
		if (keyboardDevice)
		{
			input::IInputDevice::KeyEvent ke;
			while (keyboardDevice->getKeyEvent(ke))
			{
				if (ke.type == input::IInputDevice::KtCharacter)
					m_moviePlayer->postKey(ke.character);
				else
				{
					uint32_t keyCode = translateInputKeyCode(ke.keyCode);
					if (keyCode != 0)
					{
						if (ke.type == input::IInputDevice::KtDown)
							m_moviePlayer->postKeyDown(keyCode);
						else if (ke.type == input::IInputDevice::KtUp)
							m_moviePlayer->postKeyUp(keyCode);
					}
				}
			}
		}

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

			int32_t axisZ;
			mouseDevice->getDefaultControl(input::DtAxisZ, true, axisZ);

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

				int32_t width = renderView->getWidth();
				int32_t height = renderView->getHeight();

				float viewRatio = m_environment->getRender()->getViewAspectRatio();
				float aspectRatio = m_environment->getRender()->getAspectRatio();

				width = int32_t(width * aspectRatio / viewRatio);

				int32_t mx = int32_t(width * x);
				int32_t my = int32_t(height * y);

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

				int32_t wheel = int32_t(mouseDevice->getControlValue(axisZ) * 3.0f);
				if (wheel != m_lastWheel)
				{
					m_moviePlayer->postMouseWheel(mx, my, wheel);
					m_lastWheel = wheel;
				}
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
			script::Any::fromString(args)
		};
		getStage()->invokeScript(command, sizeof_array(argv), argv);
	}
}

void FlashLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_displayRenderer || !m_visible)
		return;

	m_displayRenderer->build(frame);
	m_moviePlayer->renderFrame();
}

void FlashLayer::render(render::EyeType eye, uint32_t frame)
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

void FlashLayer::reconfigured()
{
	if (!m_moviePlayer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	float viewRatio = m_environment->getRender()->getViewAspectRatio();
	float aspectRatio = m_environment->getRender()->getAspectRatio();

	width = int32_t(width * aspectRatio / viewRatio);

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

Ref< flash::ActionObject > FlashLayer::createObject() const
{
	if (!m_moviePlayer)
		return 0;

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return new flash::ActionObject(cx);
}

Ref< flash::ActionObject > FlashLayer::createObject(uint32_t argc, const script::Any* argv) const
{
	if (!m_moviePlayer || argc < 1)
		return 0;

	std::string prototype = argv[0].getString();

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	flash::ActionValue classFunctionValue;
	cx->getGlobal()->getMemberByQName(prototype, classFunctionValue);

	Ref< flash::ActionFunction > classFunction = classFunctionValue.getObject< flash::ActionFunction >();
	if (!classFunction)
	{
		log::error << L"Unable to create object; no such prototype \"" << mbstows(prototype) << L"\"" << Endl;
		return 0;
	}

	flash::ActionValue classPrototypeValue;
	classFunction->getLocalMember(flash::ActionContext::IdPrototype, classPrototypeValue);

	Ref< flash::ActionObject > classPrototype = classPrototypeValue.getObject();
	if (!classPrototype)
	{
		log::error << L"Unable to create object; no such prototype \"" << mbstows(prototype) << L"\"" << Endl;
		return 0;
	}

	Ref< flash::ActionObject > self = new flash::ActionObject(cx, classPrototype);
	self->setMember(flash::ActionContext::Id__ctor__, classFunctionValue);

	flash::ActionValueArray args(cx->getPool(), argc - 1);
	for (uint32_t i = 0; i < argc - 1; ++i)
		args[i] = script::CastAny< flash::ActionValue >::get(argv[i + 1]);

	classFunction->call(self, args);

	return self;
}

Ref< flash::ActionObject > FlashLayer::createBitmap(drawing::Image* image) const
{
	if (!m_moviePlayer)
		return 0;

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	Ref< flash::BitmapData > bitmap = new flash::BitmapData(image);
	return bitmap->getAsObject(cx);
}

script::Any FlashLayer::externalCall(const std::string& methodName, uint32_t argc, const script::Any* argv)
{
	if (!m_moviePlayer)
		return script::Any();

	flash::ActionValue av[16];
	T_ASSERT (argc < sizeof_array(av));

	for (uint32_t i = 0; i < argc; ++i)
		av[i] = script::CastAny< flash::ActionValue >::get(argv[i]);

	flash::ActionValue ret = m_moviePlayer->dispatchCallback(methodName, argc, av);

	return script::CastAny< flash::ActionValue >::set(ret);
}

void FlashLayer::createMoviePlayer()
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	float viewRatio = m_environment->getRender()->getViewAspectRatio();
	float aspectRatio = m_environment->getRender()->getAspectRatio();

	width = int32_t(width * aspectRatio / viewRatio);

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
		soundRenderer->create(m_environment->getAudio()->getSoundPlayer());
	}

	// Create Flash movie player.
	Ref< flash::FlashMoviePlayer > moviePlayer = new flash::FlashMoviePlayer(
		displayRenderer,
		soundRenderer,
		new flash::FlashMovieLoader(m_environment->getDatabase())
	);
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
			asParade->setMember("Configuration", flash::ActionValue(new As_traktor_amalgam_Configuration(context, m_environment)));
			asParade->setMember("DisplayMode", flash::ActionValue(new As_traktor_amalgam_DisplayMode(context, m_environment)));
			asParade->setMember("InputFabricator", flash::ActionValue(new As_traktor_amalgam_InputFabricator(context, m_environment)));
		}
		asTraktor->setMember("amalgam", flash::ActionValue(asParade));
	}
	moviePlayer->setGlobal("traktor", flash::ActionValue(asTraktor));

	// Set ourself as external call hook.
	moviePlayer->setExternalCall(this);

	// Execute first frame.
	while (!moviePlayer->progressFrame(1.0f / 60.0f));

	// All success, replace instances.
	m_displayRenderer = displayRenderer;
	m_soundRenderer = soundRenderer;
	m_moviePlayer = moviePlayer;
}

flash::ActionValue FlashLayer::dispatchExternalCall(const std::string& methodName, int32_t argc, const flash::ActionValue* argv)
{
	script::Any av[16];
	T_ASSERT (argc < sizeof_array(av));

	for (int32_t i = 0; i < argc; ++i)
		av[i] = script::CastAny< flash::ActionValue >::set(argv[i]);

	script::Any ret = getStage()->invokeScript(
		methodName,
		argc,
		av
	);

	return script::CastAny< flash::ActionValue >::get(ret);
}

	}
}
