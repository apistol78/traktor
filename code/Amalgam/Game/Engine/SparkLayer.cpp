#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/SparkLayer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Spark/SparkPlayer.h"
#include "Spark/SparkRenderer.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.SparkLayer", SparkLayer, Layer)

SparkLayer::SparkLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< spark::Sprite >& sprite,
	const resource::Proxy< render::ImageProcessSettings >& imageProcessSettings,
	const Color4ub& background,
	int32_t width,
	int32_t height
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_sprite(sprite)
,	m_imageProcessSettings(imageProcessSettings)
,	m_background(background)
,	m_width(width)
,	m_height(height)
,	m_lastMouseX(-1)
,	m_lastMouseY(-1)
{
}

SparkLayer::~SparkLayer()
{
}

void SparkLayer::destroy()
{
	m_environment = 0;

	m_sprite.clear();
	m_imageProcessSettings.clear();

	safeDestroy(m_sparkRenderer);
	safeDestroy(m_imageTargetSet);
	safeDestroy(m_imageProcess);
}

void SparkLayer::transition(Layer* fromLayer)
{
}

void SparkLayer::prepare()
{
	if (!m_sparkRenderer)
	{
		m_sparkRenderer = new spark::SparkRenderer();
		m_sparkRenderer->create(m_environment->getRender()->getThreadFrameQueueCount());
	}

	if (!m_spriteInstance)
	{
		m_spriteInstance = checked_type_cast< spark::SpriteInstance* >(m_sprite->createInstance(
			0,
			m_environment->getResource()->getResourceManager(),
			m_environment->getAudio() ? m_environment->getAudio()->getSoundPlayer() : 0
		));
		if (m_spriteInstance)
			m_sparkPlayer = new spark::SparkPlayer(m_spriteInstance);
		else
			m_sparkPlayer = 0;
	}

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
		desc.multiSample = m_environment->getRender()->getMultiSample();
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = false;
		desc.ignoreStencil = false;
		desc.generateMips = false;
		desc.targets[0].format = render::TfR8G8B8A8;
		desc.targets[0].sRGB = false;
		m_imageTargetSet = renderSystem->createRenderTargetSet(desc);
	}

	updateProjection();
}

void SparkLayer::update(const UpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	input::InputSystem* inputSystem = m_environment->getInput()->getInputSystem();

	if (!m_sparkPlayer)
		return;

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
					m_sparkPlayer->postKey(ke.character);
				else
				{
					if (ke.type == input::IInputDevice::KtDown)
						m_sparkPlayer->postKeyDown(ke.keyCode);
					else if (ke.type == input::IInputDevice::KtUp)
						m_sparkPlayer->postKeyUp(ke.keyCode);
				}
			}
		}

		// Propagate mouse input to movie; don't send mouse events if mouse cursor isn't visible.
		if (renderView->isCursorVisible())
		{
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

					Vector4 clientPosition(2.0f * x - 1.0f, 1.0f - 2.0f * y, 0.0f, 1.0f);
					Vector4 viewPosition = m_projection.inverse() * clientPosition;

					int32_t mx = int32_t(viewPosition.x());
					int32_t my = int32_t(viewPosition.y());

					int32_t mb =
						(mouseDevice->getControlValue(button1) > 0.5f ? 1 : 0) |
						(mouseDevice->getControlValue(button2) > 0.5f ? 2 : 0);

					if (mx != m_lastMouseX || my != m_lastMouseY)
					{
						m_sparkPlayer->postMouseMove(Vector2(mx, my), mb);
						m_lastMouseX = mx;
						m_lastMouseY = my;
					}

					if (mb != last.button)
					{
						if (mb)
							m_sparkPlayer->postMouseDown(Vector2(mx, my), mb);
						else
							m_sparkPlayer->postMouseUp(Vector2(mx, my), mb);

						last.button = mb;
					}

					if (axisZ != -1)
					{
						int32_t wheel = int32_t(mouseDevice->getControlValue(axisZ) * 3.0f);
						if (wheel != last.wheel)
						{
							m_sparkPlayer->postMouseWheel(Vector2(mx, my), wheel);
							last.wheel = wheel;
						}
					}
				}
			}
		}
	}

	// Update player.
	m_sparkPlayer->update();
}

void SparkLayer::build(const UpdateInfo& info, uint32_t frame)
{
	if (!m_spriteInstance)
		return;

	m_sparkRenderer->build(m_spriteInstance, frame);
}

void SparkLayer::render(render::EyeType eye, uint32_t frame)
{
	if (!m_sparkRenderer || !m_spriteInstance)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	if (m_imageProcess)
	{
		if (!renderView->begin(m_imageTargetSet, 0))
			return;
	}

	if (m_background.a != 0)
	{
		Color4f clearColor(
			m_background.r / 255.0f,
			m_background.g / 255.0f,
			m_background.b / 255.0f
		);
		renderView->clear(
			render::CfColor,
			&clearColor,
			0.0f,
			0
		);
	}

	m_sparkRenderer->render(renderView, m_projection, frame);

	if (m_imageProcess)
	{
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

void SparkLayer::flush()
{
}

void SparkLayer::preReconfigured()
{
	// Discard post processing; need to be fully re-created if used.
	m_imageProcess = 0;
}

void SparkLayer::postReconfigured()
{
	updateProjection();
}

void SparkLayer::suspend()
{
}

void SparkLayer::resume()
{
}

spark::SpriteInstance* SparkLayer::getSprite() const
{
	return m_spriteInstance;
}

void SparkLayer::updateProjection()
{
	float viewRatio = m_environment->getRender()->getAspectRatio();

	float renderWidth = float(m_width);
	float renderHeight = renderWidth / viewRatio;
	if (renderHeight < m_height)
	{
		renderHeight = float(m_height);
		renderWidth = m_height * viewRatio;
	}

	float offsetX = renderWidth - float(m_width);
	float offsetY = renderHeight - float(m_height);

	m_projection = Matrix44(
		2.0f / renderWidth, 0.0f, 0.0f, -1.0f + offsetX / renderWidth,
		0.0f, -2.0f / renderHeight, 0.0f, 1.0f - offsetY / renderHeight,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

	}
}
