#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/SparkLayer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderView.h"
#include "Spark/CharacterRenderer.h"
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
	const Color4ub& background
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_sprite(sprite)
,	m_background(background)
{
}

SparkLayer::~SparkLayer()
{
}

void SparkLayer::destroy()
{
	m_environment = 0;
	m_sprite.clear();
	safeDestroy(m_characterRenderer);
}

void SparkLayer::transition(Layer* fromLayer)
{
}

void SparkLayer::prepare()
{
	if (!m_characterRenderer)
	{
		m_characterRenderer = new spark::CharacterRenderer();
		m_characterRenderer->create(m_environment->getRender()->getThreadFrameQueueCount());
	}

	if (!m_spriteInstance)
		m_spriteInstance = checked_type_cast< spark::SpriteInstance* >(m_sprite->createInstance(
			0,
			m_environment->getResource()->getResourceManager(),
			m_environment->getAudio() ? m_environment->getAudio()->getSoundPlayer() : 0
		));
}

void SparkLayer::update(const UpdateInfo& info)
{
	if (!m_spriteInstance)
		return;

	m_spriteInstance->update();
}

void SparkLayer::build(const UpdateInfo& info, uint32_t frame)
{
	if (!m_spriteInstance)
		return;

	m_characterRenderer->build(m_spriteInstance, frame);
}

void SparkLayer::render(render::EyeType eye, uint32_t frame)
{
	if (!m_characterRenderer || !m_spriteInstance)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

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

	const int32_t stageWidth = 1080;
	const int32_t stageHeight = 1920;

	float viewWidth = stageWidth;
	float viewHeight = stageHeight;

	Matrix44 projection(
		2.0f / viewWidth, 0.0f, 0.0f, -1.0f,
		0.0f, -2.0f / viewHeight, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	m_characterRenderer->render(renderView, projection, frame);
}

void SparkLayer::flush()
{
}

void SparkLayer::preReconfigured()
{
}

void SparkLayer::postReconfigured()
{
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

	}
}
