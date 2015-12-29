#include "Amalgam/Game/FrameProfiler.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_Configuration.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_DisplayMode.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_I18N.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_InputFabricator.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_SoundDriver.h"
#include "Core/Class/Any.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashCast.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/ISoundRenderer.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/Common/Classes/AsKey.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Spray/Feedback/IFeedbackManager.h"

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

bool isWhiteSpace(wchar_t ch)
{
	return ch == 0 || ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

uint32_t translateInputKeyCode(uint32_t inputKeyCode)
{
	for (uint32_t i = 0; i < sizeof_array(c_inputKeyCodes); ++i)
	{
		if (c_inputKeyCodes[i].inputKeyCode == inputKeyCode)
			return c_inputKeyCodes[i].asKeyCode;
	}
	return 0;
}

class CustomFlashMovieLoader : public flash::IFlashMovieLoader
{
public:
	CustomFlashMovieLoader(const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& externalMovies)
	:	m_externalMovies(externalMovies)
	{
	}

	virtual Ref< IHandle > loadAsync(const std::wstring& url) const T_OVERRIDE T_FINAL
	{
		return 0;
	}

	virtual Ref< flash::FlashMovie > load(const std::wstring& url) const T_OVERRIDE T_FINAL
	{
		std::map< std::wstring, resource::Proxy< flash::FlashMovie > >::const_iterator i = m_externalMovies.find(url);
		if (i != m_externalMovies.end())
			return i->second.getResource();
		else
		{
			log::error << L"Unable to load external movie \"" << url << L"\"; no such movie defined." << Endl;
			return 0;
		}
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
	IEnvironment* environment,
	const resource::Proxy< flash::FlashMovie >& movie,
	const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& externalMovies,
	const resource::Proxy< render::ImageProcessSettings >& imageProcessSettings,
	bool clearBackground,
	bool enableShapeCache,
	bool enableDirtyRegions,
	bool enableSound,
	uint32_t contextSize
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_movie(movie)
,	m_externalMovies(externalMovies)
,	m_imageProcessSettings(imageProcessSettings)
,	m_clearBackground(clearBackground)
,	m_enableShapeCache(enableShapeCache)
,	m_enableDirtyRegions(enableDirtyRegions)
,	m_enableSound(enableSound)
,	m_contextSize(contextSize)
,	m_visible(true)
,	m_offset(0.0f, 0.0f)
,	m_scale(1.0f)
,	m_lastUpValue(false)
,	m_lastDownValue(false)
,	m_lastConfirmValue(false)
,	m_lastEscapeValue(false)
,	m_lastMouseX(-1)
,	m_lastMouseY(-1)
{
	// Register ourself for UI shake.
	if (m_environment->getWorld())
	{
		spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
		if (feedbackManager)
			feedbackManager->addListener(spray::FbtUI, this);
	}
}

void FlashLayer::destroy()
{
	// Remove ourself from feedback manager.
	if (m_environment && m_environment->getWorld())
	{
		spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
		if (feedbackManager)
			feedbackManager->removeListener(spray::FbtUI, this);
	}

	m_environment = 0;
	m_movie.clear();
	m_externalMovies.clear();
	m_imageProcessSettings.clear();

	safeDestroy(m_moviePlayer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_soundRenderer);
	safeDestroy(m_imageTargetSet);
	safeDestroy(m_imageProcess);

	Layer::destroy();
}

void FlashLayer::transition(Layer* fromLayer)
{
	FlashLayer* fromFlashLayer = checked_type_cast< FlashLayer*, false >(fromLayer);
	bool shouldFlush = true;

	// Ensure matching settings.
	if (
		m_clearBackground != fromFlashLayer->m_clearBackground ||
		m_enableShapeCache != fromFlashLayer->m_enableShapeCache ||
		m_enableDirtyRegions != fromFlashLayer->m_enableDirtyRegions
	)
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

	if (m_imageProcessSettings.changed())
	{
		m_imageProcess = 0;
		m_imageTargetSet = 0;
		m_imageProcessSettings.consume();
	}

	// Re-create if necessary movie player.
	createMoviePlayer();
	if (!m_moviePlayer)
		return;

	// Re-create post processing.
	if (m_imageProcessSettings && !m_imageProcess)
	{
		resource::IResourceManager* resourceManager = m_environment->getResource()->getResourceManager();
		render::IRenderSystem* renderSystem = m_environment->getRender()->getRenderSystem();
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();

		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		m_imageProcess = new render::ImageProcess();
		m_imageProcess->create(m_imageProcessSettings, 0, resourceManager, renderSystem, width, height, false);

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
		m_imageTargetSet = renderSystem->createRenderTargetSet(desc);
	}
}

void FlashLayer::update(const UpdateInfo& info)
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

		// Propagate joystick input to movie; synthesized as keyboard events.
		int32_t joystickDeviceCount = inputSystem->getDeviceCount(input::CtJoystick, true);
		for (int32_t i = 0; i < joystickDeviceCount; ++i)
		{
			input::IInputDevice* joystickDevice = inputSystem->getDevice(input::CtJoystick, i, true);
			T_ASSERT (joystickDevice);

			int32_t up = -1, down = -1;
			joystickDevice->getDefaultControl(input::DtUp, false, up);
			joystickDevice->getDefaultControl(input::DtDown, false, down);

			if (up != -1)
			{
				bool upValue = bool(joystickDevice->getControlValue(up) > 0.5f);
				if (upValue != m_lastUpValue)
				{
					if (upValue)
						m_moviePlayer->postKeyDown(flash::AsKey::AkUp);
					else
						m_moviePlayer->postKeyUp(flash::AsKey::AkUp);
				}
				m_lastUpValue = upValue;
			}

			if (down != -1)
			{
				bool downValue = bool(joystickDevice->getControlValue(down) > 0.5f);
				if (downValue != m_lastDownValue)
				{
					if (downValue)
						m_moviePlayer->postKeyDown(flash::AsKey::AkDown);
					else
						m_moviePlayer->postKeyUp(flash::AsKey::AkDown);
				}
				m_lastDownValue = downValue;
			}

			int32_t buttonConfirm = -1, buttonEscape = -1;
			joystickDevice->getDefaultControl(input::DtButton1, false, buttonConfirm);
			joystickDevice->getDefaultControl(input::DtButton2, false, buttonEscape);

			if (buttonConfirm != -1)
			{
				bool confirmValue = bool(joystickDevice->getControlValue(buttonConfirm) > 0.5f);
				if (confirmValue != m_lastConfirmValue)
				{
					if (confirmValue)
						m_moviePlayer->postKeyDown(flash::AsKey::AkEnter);
					else
						m_moviePlayer->postKeyUp(flash::AsKey::AkEnter);
				}
				m_lastConfirmValue = confirmValue;
			}

			if (buttonEscape != -1)
			{
				bool escapeValue = bool(joystickDevice->getControlValue(buttonEscape) > 0.5f);
				if (escapeValue != m_lastEscapeValue)
				{
					if (escapeValue)
						m_moviePlayer->postKeyDown(flash::AsKey::AkEscape);
					else
						m_moviePlayer->postKeyUp(flash::AsKey::AkEscape);
				}
				m_lastEscapeValue = escapeValue;
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
		Any argv[] =
		{
			Any::fromString(args)
		};
		getStage()->invokeScript(command, sizeof_array(argv), argv);
	}

	info.getProfiler()->endScope();
}

void FlashLayer::build(const UpdateInfo& info, uint32_t frame)
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

	if (m_imageProcess)
	{
		if (renderView->begin(m_imageTargetSet, 0))
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

			render::ImageProcessStep::Instance::RenderParams params;
			m_imageProcess->render(
				renderView,
				m_imageTargetSet,
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
	m_imageProcess = 0;
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

flash::FlashSpriteInstance* FlashLayer::getRoot()
{
	if (!m_moviePlayer)
	{
		log::warning << L"FlashLayer::getRoot fail; no movie player initialized." << Endl;
		return 0;
	}
	return m_moviePlayer->getMovieInstance();
}

flash::FlashMovie* FlashLayer::getExternal(const std::wstring& id) const
{
	std::map< std::wstring, resource::Proxy< flash::FlashMovie > >::const_iterator i = m_externalMovies.find(id);
	return i != m_externalMovies.end() ? i->second.getResource() : 0;
}

Any FlashLayer::externalCall(const std::string& methodName, uint32_t argc, const Any* argv)
{
	if (!m_moviePlayer)
		return Any();

	flash::ActionValue av[16];
	T_ASSERT (argc < sizeof_array(av));

	for (uint32_t i = 0; i < argc; ++i)
		av[i] = CastAny< flash::ActionValue >::get(argv[i]);

	flash::ActionValue ret = m_moviePlayer->dispatchCallback(methodName, argc, av);

	return CastAny< flash::ActionValue >::set(ret);
}

std::wstring FlashLayer::getPrintableString(const std::wstring& text, const std::wstring& empty) const
{
	if (!m_movie)
		return L"";

	StringOutputStream ss;

	const SmallMap< uint16_t, Ref< flash::FlashFont > >& fonts = m_movie->getFonts();
	for (size_t i = 0; i < text.length(); ++i)
	{
		wchar_t ch = text[i];
		bool valid = true;

		if (!isWhiteSpace(ch))
		{
			valid = false;
			for (SmallMap< uint16_t, Ref< flash::FlashFont > >::const_iterator j = fonts.begin(); j != fonts.end(); ++j)
			{
				if (j->second->lookupIndex(ch) != 0)
				{
					valid = true;
					break;
				}
			}
		}

		if (valid)
			ss << ch;
	}

	if (!ss.empty())
		return ss.str();
	else
		return empty;
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
			m_environment->getRender()->getThreadFrameQueueCount(),
			m_contextSize * 1024 * 1024,
			m_clearBackground,
			m_enableShapeCache,
			m_enableDirtyRegions,
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
			new CustomFlashMovieLoader(m_externalMovies)
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

		// Execute first frame.
		while (!moviePlayer->progressFrame(1.0f / 60.0f));

		// All success, replace instances.
		m_moviePlayer = moviePlayer;
	}
}

flash::ActionValue FlashLayer::dispatchExternalCall(const std::string& methodName, int32_t argc, const flash::ActionValue* argv)
{
	Any av[16];
	T_ASSERT (argc < sizeof_array(av));

	for (int32_t i = 0; i < argc; ++i)
		av[i] = CastAny< flash::ActionValue >::set(argv[i]);

	Any ret = getStage()->invokeScript(
		methodName,
		argc,
		av
	);

	return CastAny< flash::ActionValue >::get(ret);
}

void FlashLayer::feedbackValues(spray::FeedbackType type, const float* values, int32_t count)
{
	T_ASSERT (count >= 3);
	m_offset = Vector2(values[0] * 0.01f, values[1] * 0.01f);
	m_scale = values[2] * 0.01f + 1.0f;
}

	}
}
