#include "Weather/Clouds/CloudEntityData.h"
#include "Weather/Clouds/CloudEntity.h"
#include "Resource/IResourceManager.h"
#include "Render/ITexture.h"
#include "Render/TextureResource.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.weather.CloudEntityData", CloudEntityData, world::SpatialEntityData)

CloudEntityData::CloudEntityData()
:	m_impostorTargetResolution(256)
,	m_impostorSliceCount(1)
,	m_updateFrequency(10)
,	m_updatePositionThreshold(0.1f)
,	m_updateDirectionThreshold(0.1f)
{
}

CloudEntity* CloudEntityData::createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_particleShader))
		return 0;
	if (!resourceManager->bind(m_particleTexture))
		return 0;
	if (!resourceManager->bind(m_impostorShader))
		return 0;

	Ref< CloudEntity > cloudEntity = gc_new< CloudEntity >();
	if (cloudEntity->create(
		renderSystem,
		m_particleShader,
		m_particleTexture,
		m_impostorShader,
		m_impostorTargetResolution,
		m_impostorSliceCount,
		m_updateFrequency,
		m_updatePositionThreshold,
		m_updateDirectionThreshold,
		m_particleData
	))
	{
		cloudEntity->setTransform(getTransform());
		return cloudEntity;
	}

	return 0;
}

bool CloudEntityData::serialize(Serializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> resource::Member< render::Shader, render::ShaderGraph >(L"particleShader", m_particleShader);
	s >> resource::Member< render::ITexture, render::TextureResource >(L"particleTexture", m_particleTexture);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"impostorShader", m_impostorShader);
	s >> Member< uint32_t >(L"impostorTargetResolution", m_impostorTargetResolution);
	s >> Member< uint32_t >(L"impostorSliceCount", m_impostorSliceCount);
	s >> Member< uint32_t >(L"updateFrequency", m_updateFrequency);
	s >> Member< float >(L"updatePositionThreshold", m_updatePositionThreshold, 0.0f);
	s >> Member< float >(L"updateDirectionThreshold", m_updateDirectionThreshold, 0.0f, 180.0f);
	s >> MemberComposite< CloudParticleData >(L"particleData", m_particleData);

	return true;
}

	}
}
