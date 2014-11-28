#include "Amalgam/FrameProfiler.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Engine/FlashCast.h"
#include "Amalgam/Engine/FlashLayer.h"
#include "Amalgam/Engine/Stage.h"
#include "Amalgam/Engine/Action/Classes/As_traktor_amalgam_Configuration.h"
#include "Amalgam/Engine/Action/Classes/As_traktor_amalgam_DisplayMode.h"
#include "Amalgam/Engine/Action/Classes/As_traktor_amalgam_I18N.h"
#include "Amalgam/Engine/Action/Classes/As_traktor_amalgam_InputFabricator.h"
#include "Amalgam/Engine/Action/Classes/As_traktor_amalgam_SoundDriver.h"
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
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Script/Any.h"
#include "Spray/Feedback/IFeedbackManager.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"

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

class CustomFlashMovieLoader : public flash::FlashMovieLoader
{
public:
	CustomFlashMovieLoader(db::Database* database, const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& externalMovies)
	:	flash::FlashMovieLoader(database)
	,	m_externalMovies(externalMovies)
	{
	}

	virtual Ref< flash::FlashMovie > load(const std::wstring& name) const
	{
		std::map< std::wstring, resource::Proxy< flash::FlashMovie > >::const_iterator i = m_externalMovies.find(name);
		if (i != m_externalMovies.end())
			return i->second.getResource();
		else
			return 0;
	}

private:
	const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& m_externalMovies;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.FlashLayer", FlashLayer, Layer)

FlashLayer::FlashLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	amalgam::IEnvironment* environment,
	const resource::Proxy< flash::FlashMovie >& movie,
	const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& externalMovies,
	const resource::Proxy< world::PostProcessSettings >& postProcessSettings,
	bool clearBackground,
	bool enableSound
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_movie(movie)
,	m_externalMovies(externalMovies)
,	m_postProcessSettings(postProcessSettings)
,	m_clearBackground(clearBackground)
,	m_enableSound(enableSound)
,	m_visible(true)
,	m_offset(0.0f, 0.0f)
,	m_scale(1.0f)
,	m_lastMouseX(-1)
,	m_lastMouseY(-1)
{
	// Register ourself for UI shake.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
		feedbackManager->addListener(spray::FbtUI, this);
}

FlashLayer::~FlashLayer()
{
	destroy();
}

void FlashLayer::destroy()
{
	// Remove ourself from feedback manager.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
		feedbackManager->removeListener(spray::FbtUI, this);

	m_environment = 0;
	m_movie.clear();
	m_externalMovies.clear();
	m_postProcessSettings.clear();

	safeDestroy(m_moviePlayer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_soundRenderer);
	safeDestroy(m_postTargetSet);
	safeDestroy(m_postProcess);
}

void FlashLayer::transition(Layer* fromLayer)
{
	FlashLayer* fromFlashLayer = checked_type_cast< FlashLayer*, false >(fromLayer);
	bool shouldFlush = true;

	// Ensure matching settings.
	if (m_clearBackground != fromFlashLayer->m_clearBackground)
		return;

	// Pass movie as well, if it's the same movie and we're allowed.
	bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (m_movie == fromFlashLayer->m_movie)
	{
		if (permit)
		{
			m_movie.consume();
			m_moviePlayer = fromFlashLayer->m_moviePlayer;
			m_moviePlayer->setExternalCall(this);
			fromFlashLayer->m_moviePlayer = 0;
		}

		// Also do not flush caches if same movie will be used again;
		// this improve performance as images and such doesn't need to be reloaded.
		shouldFlush = false;
	}

	// Keep display and sound renderer.
	m_displayRenderer = fromFlashLayer->m_displayRenderer;
	m_soundRenderer = fromFlashLayer->m_soundRenderer;
	fromFlashLayer->m_displayRenderer = 0;
	fromFlashLayer->m_soundRenderer = 0;

	// Ensure display renderer's caches are fresh.
	if (m_displayRenderer && shouldFlush)
		m_displayRenderer->flushCaches();
}

void FlashLayer::prepare()
{
	if (m_movie.changed())
	{
		m_moviePlayer = 0;
		m_movie.consume();
	}

	if (m_postProcessSettings.changed())
	{
		m_postProcess = 0;
		m_postTargetSet = 0;
		m_postProcessSettings.consume();
	}

	// Re-create if necessary movie player.
	createMoviePlayer();
	if (!m_moviePlayer)
		return;

	// Re-create post processing.
	if (m_postProcessSettings && !m_postProcess)
	{
		resource::IResourceManager* resourceManager = m_environment->getResource()->getResourceManager();
		render::IRenderSystem* renderSystem = m_environment->getRender()->getRenderSystem();
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();

		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		m_postProcess = new world::PostProcess();
		m_postProcess->create(m_postProcessSettings, 0, resourceManager, renderSystem, width, height);

		render::RenderTargetSetCreateDesc desc;
		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = false;
		desc.ignoreStencil = false;
		desc.generateMips = false;
		desc.targets[0].format = render::TfR8G8B8A8;
		desc.targets[0].sRGB = false;
		m_postTargetSet = renderSystem->createRenderTargetSet(desc);
	}
}

void FlashLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	input::InputSystem* inputSystem = m_environment->getInput()->getInputSystem();
	std::string command, args;

	info.getProfiler()->beginScope(FptFlashLayer);

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

		// Propagate mouse input to movie; don't send mouse events if mouse cursor isn't visible.
		if (renderView->isCursorVisible())
		{
			int32_t width = renderView->getWidth();
			int32_t height = renderView->getHeight();

			float viewRatio = m_environment->getRender()->getViewAspectRatio();
			float aspectRatio = m_environment->getRender()->getAspectRatio();

			width = int32_t(width * aspectRatio / viewRatio);

			int32_t mouseDeviceCount = inputSystem->getDeviceCount(input::CtMouse, true);
			if (mouseDeviceCount >= sizeof_array(m_lastMouse))
				mouseDeviceCount = sizeof_array(m_lastMouse);

			for (int32_t i = 0; i < mouseDeviceCount; ++i)
			{
				input::IInputDevice* mouseDevice = inputSystem->getDevice(input::CtMouse, i, true);
				T_ASSERT (mouseDevice);

				LastMouseState& last = m_lastMouse[i];

				int32_t positionX = -1, positionY = -1;
				mouseDevice->getDefaultControl(input::DtPositionX, true, positionX);
				mouseDevice->getDefaultControl(input::DtPositionY, true, positionY);

				int32_t button1 = -1, button2 = -1;
				mouseDevice->getDefaultControl(input::DtButton1, false, button1);
				mouseDevice->getDefaultControl(input::DtButton2, false, button2);

				int32_t axisZ = -1;
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

					int32_t mx = int32_t(width * x);
					int32_t my = int32_t(height * y);

					int32_t mb =
						(mouseDevice->getControlValue(button1) > 0.5f ? 1 : 0) |
						(mouseDevice->getControlValue(button2) > 0.5f ? 2 : 0);

					if (mx != m_lastMouseX || my != m_lastMouseY)
					{
						m_moviePlayer->postMouseMove(mx, my, mb);
						m_lastMouseX = mx;
						m_lastMouseY = my;
					}

					if (mb != last.button)
					{
						if (mb)
							m_moviePlayer->postMouseDown(mx, my, mb);
						else
							m_moviePlayer->postMouseUp(mx, my, mb);

						last.button = mb;
					}

					if (axisZ != -1)
					{
						int32_t wheel = int32_t(mouseDevice->getControlValue(axisZ) * 3.0f);
						if (wheel != last.wheel)
						{
							m_moviePlayer->postMouseWheel(mx, my, wheel);
							last.wheel = wheel;
						}
					}
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

	info.getProfiler()->endScope();
}

void FlashLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_displayRenderer || !m_moviePlayer || !m_visible)
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

	if (m_postProcess)
	{
		if (renderView->begin(m_postTargetSet, 0))
		{
			const static Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
			renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);

			m_displayRenderer->render(
				renderView,
				frame,
				eye,
				m_offset,
				m_scale
			);

			renderView->end();

			world::PostProcessStep::Instance::RenderParams params;
			m_postProcess->render(
				renderView,
				m_postTargetSet,
				0,
				0,
				params
			);
		}
	}
	else
	{
		m_displayRenderer->render(
			renderView,
			frame,
			eye,
			m_offset,
			m_scale
		);
	}
}

void FlashLayer::flush()
{
	if (m_displayRenderer)
		m_displayRenderer->flush();
}

void FlashLayer::preReconfigured()
{
	// Discard post processing; need to be fully re-created if used.
	m_postProcess = 0;
}

void FlashLayer::postReconfigured()
{
	// Post resize to movie player; adjust width to keep aspect ratio.
	if (m_moviePlayer)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		float viewRatio = m_environment->getRender()->getViewAspectRatio();
		float aspectRatio = m_environment->getRender()->getAspectRatio();

		int32_t width = int32_t(renderView->getWidth() * aspectRatio / viewRatio);
		int32_t height = renderView->getHeight();

		m_moviePlayer->postViewResize(width, height);
	}
}

void FlashLayer::suspend()
{
}

void FlashLayer::resume()
{
}

flash::FlashMoviePlayer* FlashLayer::getMoviePlayer()
{
	return m_moviePlayer;
}

flash::ActionObject* FlashLayer::getGlobal()
{
	if (!m_moviePlayer)
	{
		log::warning << L"FlashLayer::getGlobal fail; no movie player initialized." << Endl;
		return 0;
	}

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return cx->getGlobal();
}

flash::ActionObject* FlashLayer::getRoot()
{
	if (!m_moviePlayer)
	{
		log::warning << L"FlashLayer::getRoot fail; no movie player initialized." << Endl;
		return 0;
	}

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return movieInstance->getAsObject(cx);
}

Ref< flash::ActionObject > FlashLayer::createObject() const
{
	if (!m_moviePlayer)
	{
		log::warning << L"FlashLayer::createObject fail; no movie player initialized." << Endl;
		return 0;
	}

	flash::FlashSpriteInstance* movieInstance = m_moviePlayer->getMovieInstance();
	T_ASSERT (movieInstance);

	flash::ActionContext* cx = movieInstance->getContext();
	T_ASSERT (cx);

	return new flash::ActionObject(cx);
}

Ref< flash::ActionObject > FlashLayer::createObject(uint32_t argc, const script::Any* argv) const
{
	if (!m_moviePlayer)
	{
		log::warning << L"FlashLayer::createObject fail; no movie player initialized." << Endl;
		return 0;
	}

	if (argc < 1)
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
	{
		log::warning << L"FlashLayer::createBitmap fail; no movie player initialized." << Endl;
		return 0;
	}

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
	// Create accelerated Flash renderer.
	if (!m_displayRenderer)
	{
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
		
		m_displayRenderer = displayRenderer;
	}

	// Create sound Flash renderer.
	if (!m_soundRenderer)
	{
		if (m_enableSound && m_environment->getAudio())
		{
			Ref< flash::SoundRenderer > soundRenderer = new flash::SoundRenderer();
			if (!soundRenderer->create(m_environment->getAudio()->getSoundPlayer()))
			{
				log::error << L"Unable to create sound renderer" << Endl;
				return;
			}
			m_soundRenderer = soundRenderer;
		}
	}

	// Create Flash movie player.
	if (!m_moviePlayer)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		T_ASSERT (renderView);

		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		float viewRatio = m_environment->getRender()->getViewAspectRatio();
		float aspectRatio = m_environment->getRender()->getAspectRatio();

		width = int32_t(width * aspectRatio / viewRatio);

		Ref< flash::FlashMoviePlayer > moviePlayer = new flash::FlashMoviePlayer(
			m_displayRenderer,
			m_soundRenderer,
			new CustomFlashMovieLoader(m_environment->getDatabase(), m_externalMovies)
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
			Ref< flash::ActionObject > asAmalgam = new flash::ActionObject(context);
			{
				asAmalgam->setMember("Configuration", flash::ActionValue(new As_traktor_amalgam_Configuration(context, m_environment)));
				asAmalgam->setMember("DisplayMode", flash::ActionValue(new As_traktor_amalgam_DisplayMode(context, m_environment)));
				asAmalgam->setMember("I18N", flash::ActionValue(new As_traktor_amalgam_I18N(context)));
				asAmalgam->setMember("InputFabricator", flash::ActionValue(new As_traktor_amalgam_InputFabricator(context, m_environment)));
				asAmalgam->setMember("SoundDriver", flash::ActionValue(new As_traktor_amalgam_SoundDriver(context)));
			}
			asTraktor->setMember("amalgam", flash::ActionValue(asAmalgam));
		}
		moviePlayer->setGlobal("traktor", flash::ActionValue(asTraktor));

		// Set ourself as external call hook.
		moviePlayer->setExternalCall(this);

		// Pre-cache resources.
		m_displayRenderer->precache(*context->getDictionary());

		// Execute first frame.
		while (!moviePlayer->progressFrame(1.0f / 60.0f));

		// All success, replace instances.
		m_moviePlayer = moviePlayer;
	}
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

void FlashLayer::feedbackValues(spray::FeedbackType type, const float* values, int32_t count)
{
	T_ASSERT (count >= 3);
	m_offset = Vector2(values[0] * 0.01f, values[1] * 0.01f);
	m_scale = values[2] * 0.01f + 1.0f;
}

	}
}
