/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/Any.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Timer/Profiler.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/Frame/RenderGraph.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Stage.h"
#include "Spark/DefaultCharacterFactory.h"
#include "Spark/Font.h"
#include "Spark/Key.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/MovieRenderer.h"
#include "Spark/MoviePlayer.h"
#include "Spark/SpriteInstance.h"
#include "Spark/ISoundRenderer.h"
#include "Spark/Acc/AccDisplayRenderer.h"
#include "Spark/Debug/MovieDebugger.h"
#include "Spark/Runtime/SparkLayer.h"
#include "Spark/Sound/SoundRenderer.h"
#include "Spray/Feedback/IFeedbackManager.h"

namespace traktor::spark
{
	namespace
	{

const struct InputKeyCode
{
	input::DefaultControl inputKeyCode;
	uint32_t asKeyCode;
}
c_inputKeyCodes[] =
{
	{ input::DefaultControl::KeyLeftControl, Key::AkControl },
	{ input::DefaultControl::KeyRightControl, Key::AkControl },
	{ input::DefaultControl::KeyDelete, Key::AkDeleteKey },
	{ input::DefaultControl::KeyDown, Key::AkDown },
	{ input::DefaultControl::KeyEnd, Key::AkEnd },
	{ input::DefaultControl::KeyReturn, Key::AkEnter },
	{ input::DefaultControl::KeyEscape, Key::AkEscape },
	{ input::DefaultControl::KeyHome, Key::AkHome },
	{ input::DefaultControl::KeyInsert, Key::AkInsert },
	{ input::DefaultControl::KeyLeft, Key::AkLeft },
	{ input::DefaultControl::KeyRight, Key::AkRight },
	{ input::DefaultControl::KeyLeftShift, Key::AkShift },
	{ input::DefaultControl::KeyRightShift, Key::AkShift },
	{ input::DefaultControl::KeySpace, Key::AkSpace },
	{ input::DefaultControl::KeyTab, Key::AkTab },
	{ input::DefaultControl::KeyUp, Key::AkUp }
};

bool isWhiteSpace(wchar_t ch)
{
	return ch == 0 || ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

uint32_t translateInputKeyCode(uint32_t inputKeyCode)
{
	for (uint32_t i = 0; i < sizeof_array(c_inputKeyCodes); ++i)
	{
		if (c_inputKeyCodes[i].inputKeyCode == (input::DefaultControl)inputKeyCode)
			return c_inputKeyCodes[i].asKeyCode;
	}
	return 0;
}

class CustomMovieLoader : public IMovieLoader
{
public:
	explicit CustomMovieLoader(const std::map< std::wstring, resource::Proxy< Movie > >& externalMovies)
	:	m_externalMovies(externalMovies)
	{
	}

	virtual Ref< MovieResult > loadAsync(const std::wstring& url) const override final
	{
		return nullptr;
	}

	virtual Ref< Movie > load(const std::wstring& url) const override final
	{
		auto it = m_externalMovies.find(url);
		if (it != m_externalMovies.end())
			return it->second.getResource();
		else
		{
			log::error << L"Unable to load external movie \"" << url << L"\"; no such movie defined." << Endl;
			return nullptr;
		}
	}

private:
	const std::map< std::wstring, resource::Proxy< Movie > >& m_externalMovies;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkLayer", SparkLayer, runtime::Layer)

SparkLayer::SparkLayer(
	runtime::Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	runtime::IEnvironment* environment,
	const resource::Proxy< Movie >& movie,
	const std::map< std::wstring, resource::Proxy< Movie > >& externalMovies,
	bool clearBackground,
	bool enableSound
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_movie(movie)
,	m_externalMovies(externalMovies)
,	m_clearBackground(clearBackground)
,	m_enableSound(enableSound)
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

void SparkLayer::destroy()
{
	// Remove ourself from feedback manager.
	if (m_environment && m_environment->getWorld())
	{
		spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
		if (feedbackManager)
			feedbackManager->removeListener(spray::FbtUI, this);
	}

	m_environment = nullptr;
	m_movie.clear();
	m_externalMovies.clear();

	safeDestroy(m_moviePlayer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_soundRenderer);

	Layer::destroy();
}

void SparkLayer::transition(Layer* fromLayer)
{
	SparkLayer* fromSparkLayer = checked_type_cast< SparkLayer*, false >(fromLayer);

	// Pass movie as well, if it's the same movie and we're allowed.
	const bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (m_movie == fromSparkLayer->m_movie)
	{
		if (permit)
		{
			m_movie.consume();
			m_moviePlayer = fromSparkLayer->m_moviePlayer;
			fromSparkLayer->m_moviePlayer = nullptr;
		}
	}

	// Keep display and sound renderer.
	m_displayRenderer = fromSparkLayer->m_displayRenderer;
	m_soundRenderer = fromSparkLayer->m_soundRenderer;
	fromSparkLayer->m_displayRenderer = nullptr;
	fromSparkLayer->m_soundRenderer = nullptr;

	// Ensure display renderer's caches are fresh.
	if (m_displayRenderer)
		m_displayRenderer->flushCaches();

	// Update setting on display renderer.
	if (m_displayRenderer)
		m_displayRenderer->setClearBackground(m_clearBackground);
}

void SparkLayer::preUpdate(const runtime::UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SparkLayer pre-update");

	if (m_movie.changed())
	{
		m_moviePlayer = nullptr;
		m_movie.consume();
	}

	// Re-create if necessary movie player.
	createMoviePlayer();
}

void SparkLayer::update(const runtime::UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SparkLayer update");
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	input::InputSystem* inputSystem = m_environment->getInput()->getInputSystem();
	std::string command, args;

	if (!m_moviePlayer)
		return;

	// Do NOT propagate input in case user is fabricating input.
	if (!m_environment->getInput()->isFabricating())
	{
		// Propagate keyboard input to movie.
		input::IInputDevice* keyboardDevice = inputSystem->getDevice(input::InputCategory::Keyboard, 0, true);
		if (keyboardDevice)
		{
			input::IInputDevice::KeyEvent ke;
			while (keyboardDevice->getKeyEvent(ke))
			{
				if (ke.type == input::KeyEventType::Character)
					m_moviePlayer->postKey(ke.character);
				else
				{
					uint32_t keyCode = translateInputKeyCode(ke.keyCode);
					if (keyCode != 0)
					{
						if (ke.type == input::KeyEventType::Down)
							m_moviePlayer->postKeyDown(keyCode);
						else if (ke.type == input::KeyEventType::Up)
							m_moviePlayer->postKeyUp(keyCode);
					}
				}
			}
		}

		// Propagate joystick input to movie; synthesized as keyboard events.
		int32_t joystickDeviceCount = inputSystem->getDeviceCount(input::InputCategory::Joystick, true);
		for (int32_t i = 0; i < joystickDeviceCount; ++i)
		{
			input::IInputDevice* joystickDevice = inputSystem->getDevice(input::InputCategory::Joystick, i, true);
			T_ASSERT(joystickDevice);

			int32_t up = -1, down = -1;
			joystickDevice->getDefaultControl(input::DefaultControl::Up, false, up);
			joystickDevice->getDefaultControl(input::DefaultControl::Down, false, down);

			if (up != -1)
			{
				const bool upValue = bool(joystickDevice->getControlValue(up) > 0.5f);
				if (upValue != m_lastUpValue)
				{
					if (upValue)
						m_moviePlayer->postKeyDown(Key::AkUp);
					else
						m_moviePlayer->postKeyUp(Key::AkUp);
				}
				m_lastUpValue = upValue;
			}

			if (down != -1)
			{
				const bool downValue = bool(joystickDevice->getControlValue(down) > 0.5f);
				if (downValue != m_lastDownValue)
				{
					if (downValue)
						m_moviePlayer->postKeyDown(Key::AkDown);
					else
						m_moviePlayer->postKeyUp(Key::AkDown);
				}
				m_lastDownValue = downValue;
			}

			int32_t buttonConfirm = -1, buttonEscape = -1;
			joystickDevice->getDefaultControl(input::DefaultControl::Button1, false, buttonConfirm);
			joystickDevice->getDefaultControl(input::DefaultControl::Button2, false, buttonEscape);

			if (buttonConfirm != -1)
			{
				const bool confirmValue = bool(joystickDevice->getControlValue(buttonConfirm) > 0.5f);
				if (confirmValue != m_lastConfirmValue)
				{
					if (confirmValue)
						m_moviePlayer->postKeyDown(Key::AkEnter);
					else
						m_moviePlayer->postKeyUp(Key::AkEnter);
				}
				m_lastConfirmValue = confirmValue;
			}

			if (buttonEscape != -1)
			{
				const bool escapeValue = bool(joystickDevice->getControlValue(buttonEscape) > 0.5f);
				if (escapeValue != m_lastEscapeValue)
				{
					if (escapeValue)
						m_moviePlayer->postKeyDown(Key::AkEscape);
					else
						m_moviePlayer->postKeyUp(Key::AkEscape);
				}
				m_lastEscapeValue = escapeValue;
			}
		}

		// Propagate mouse input to movie; don't send mouse events if mouse cursor isn't visible.
		if (renderView->isCursorVisible())
		{
			int32_t width = renderView->getWidth();
			int32_t height = renderView->getHeight();

			const float viewRatio = m_environment->getRender()->getViewAspectRatio();
			const float aspectRatio = m_environment->getRender()->getAspectRatio();

			width = int32_t(width * aspectRatio / viewRatio);

			int32_t mouseDeviceCount = inputSystem->getDeviceCount(input::InputCategory::Mouse, true);
			if (mouseDeviceCount >= sizeof_array(m_lastMouse))
				mouseDeviceCount = sizeof_array(m_lastMouse);

			for (int32_t i = 0; i < mouseDeviceCount; ++i)
			{
				input::IInputDevice* mouseDevice = inputSystem->getDevice(input::InputCategory::Mouse, i, true);
				T_ASSERT(mouseDevice);

				LastMouseState& last = m_lastMouse[i];

				int32_t positionX = -1, positionY = -1;
				mouseDevice->getDefaultControl(input::DefaultControl::PositionX, true, positionX);
				mouseDevice->getDefaultControl(input::DefaultControl::PositionY, true, positionY);

				int32_t button1 = -1, button2 = -1;
				mouseDevice->getDefaultControl(input::DefaultControl::Button1, false, button1);
				mouseDevice->getDefaultControl(input::DefaultControl::Button2, false, button2);

				int32_t axisZ = -1;
				mouseDevice->getDefaultControl(input::DefaultControl::AxisZ, true, axisZ);

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

					const int32_t mx = int32_t(width * x);
					const int32_t my = int32_t(height * y);

					const int32_t mb =
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
						const int32_t wheel = int32_t(mouseDevice->getControlValue(axisZ) * 3.0f);
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
	m_moviePlayer->progress((float)info.getSimulationDeltaTime(), m_soundRenderer);
}

void SparkLayer::postUpdate(const runtime::UpdateInfo& info)
{
}

void SparkLayer::preSetup(const runtime::UpdateInfo& info)
{
	if (m_movie.changed())
	{
		m_moviePlayer = nullptr;
		m_movie.consume();
	}

	// Re-create if necessary movie player.
	createMoviePlayer();
}

void SparkLayer::setup(const runtime::UpdateInfo& info, render::RenderGraph& renderGraph)
{
	if (!m_movieRenderer)
		return;

	m_displayRenderer->beginSetup(&renderGraph);
	m_moviePlayer->render(m_movieRenderer);
	m_displayRenderer->endSetup();
}

void SparkLayer::preReconfigured()
{
}

void SparkLayer::postReconfigured()
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

void SparkLayer::suspend()
{
}

void SparkLayer::resume()
{
}

void SparkLayer::hotReload()
{
}

MoviePlayer* SparkLayer::getMoviePlayer()
{
	return m_moviePlayer;
}

Context* SparkLayer::getContext()
{
	if (!m_moviePlayer)
	{
		log::warning << L"Unable to get context; no movie player initialized." << Endl;
		return nullptr;
	}
	return m_moviePlayer->getMovieInstance()->getContext();
}

SpriteInstance* SparkLayer::getRoot()
{
	if (!m_moviePlayer)
	{
		log::warning << L"Unable to get root; no movie player initialized." << Endl;
		return nullptr;
	}
	return m_moviePlayer->getMovieInstance();
}

Movie* SparkLayer::getExternal(const std::wstring& id) const
{
	auto it = m_externalMovies.find(id);
	return it != m_externalMovies.end() ? it->second.getResource() : nullptr;
}

std::wstring SparkLayer::getPrintableString(const std::wstring& text, const std::wstring& empty) const
{
	if (!m_movie)
		return L"";

	StringOutputStream ss;

	const SmallMap< uint16_t, Ref< Font > >& fonts = m_movie->getFonts();
	for (size_t i = 0; i < text.length(); ++i)
	{
		wchar_t ch = text[i];
		bool valid = true;

		if (!isWhiteSpace(ch))
		{
			valid = false;
			for (auto font : fonts)
			{
				if (font.second->lookupIndex(ch) != 0)
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

void SparkLayer::createMoviePlayer()
{
	// Create accelerated Spark renderer.
	if (!m_displayRenderer)
	{
		Ref< AccDisplayRenderer > displayRenderer = new AccDisplayRenderer();
		if (!displayRenderer->create(
			m_environment->getResource()->getResourceManager(),
			m_environment->getRender()->getRenderSystem(),
			m_environment->getRender()->getThreadFrameQueueCount(),
			m_clearBackground
		))
		{
			log::error << L"Unable to create display renderer." << Endl;
			return;
		}
		m_displayRenderer = displayRenderer;
	}

	// Create sound Spark renderer.
	if (!m_soundRenderer)
	{
		if (m_enableSound && m_environment->getAudio())
		{
			Ref< SoundRenderer > soundRenderer = new SoundRenderer();
			if (!soundRenderer->create(m_environment->getAudio()->getSoundPlayer()))
			{
				log::error << L"Unable to create sound renderer" << Endl;
				return;
			}
			m_soundRenderer = soundRenderer;
		}
	}

	// Create Spark movie renderer.
	if (!m_movieRenderer)
	{
		T_ASSERT(m_displayRenderer);
		m_movieRenderer = new MovieRenderer(m_displayRenderer);
	}

	// Create Spark movie player.
	if (!m_moviePlayer)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		T_ASSERT(renderView);

		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		const float viewRatio = m_environment->getRender()->getViewAspectRatio();
		const float aspectRatio = m_environment->getRender()->getAspectRatio();

		width = int32_t(width * aspectRatio / viewRatio);

		// Connect to remote debugger.
		Ref< MovieDebugger > movieDebugger;
#if 0
		Ref< net::TcpSocket > remoteDebuggerSocket = new net::TcpSocket();
		if (remoteDebuggerSocket->connect(net::SocketAddressIPv4(L"localhost", 12345)))
			movieDebugger = new MovieDebugger(
				new net::BidirectionalObjectTransport(remoteDebuggerSocket),
				getName()
			);
#endif

		Ref< MoviePlayer > moviePlayer = new MoviePlayer(
			new DefaultCharacterFactory(),
			new CustomMovieLoader(m_externalMovies),
			movieDebugger
		);
		if (!moviePlayer->create(m_movie, width, height, m_soundRenderer))
		{
			log::error << L"Unable to create movie player" << Endl;
			return;
		}

		// Execute first frame, do not provide sound renderer so we don't trigger alot of sounds initially.
		while (!moviePlayer->progress(1.0f / 60.0f, 0));

		// All success, replace instances.
		m_moviePlayer = moviePlayer;
	}
}

void SparkLayer::feedbackValues(spray::FeedbackType type, const float* values, int32_t count)
{
	T_ASSERT(count >= 3);
	m_offset = Vector2(values[0] * 0.01f, values[1] * 0.01f);
	m_scale = values[2] * 0.01f + 1.0f;
}

}
