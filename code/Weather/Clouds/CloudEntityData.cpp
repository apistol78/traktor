#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Resource/TextureResource.h"
#include "Weather/Clouds/CloudEntityData.h"
#include "Weather/Clouds/CloudEntity.h"
#include "Weather/Clouds/CloudMask.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.CloudEntityData", 1, CloudEntityData, world::SpatialEntityData)

CloudEntityData::CloudEntityData()
:	m_impostorTargetResolution(256)
,	m_impostorSliceCount(1)
,	m_updateFrequency(10)
,	m_updatePositionThreshold(0.1f)
,	m_updateDirectionThreshold(0.1f)
{
}

Ref< CloudEntity > CloudEntityData::createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_particleShader))
		return 0;
	if (!resourceManager->bind(m_particleTexture))
		return 0;
	if (!resourceManager->bind(m_impostorShader))
		return 0;

	resourceManager->bind(m_mask);

	Ref< CloudEntity > cloudEntity = new CloudEntity();
	if (cloudEntity->create(
		renderSystem,
		m_particleShader,
		m_particleTexture,
		m_impostorShader,
		m_mask,
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

bool CloudEntityData::serialize(ISerializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> resource::Member< render::Shader, render::ShaderGraph >(L"particleShader", m_particleShader);
	s >> resource::Member< render::ITexture, render::TextureResource >(L"particleTexture", m_particleTexture);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"impostorShader", m_impostorShader);

	if (s.getVersion() >= 1)
		s >> resource::Member< CloudMask, CloudMaskResource >(L"mask", m_mask);

	s >> Member< uint32_t >(L"impostorTargetResolution", m_impostorTargetResolution);
	s >> Member< uint32_t >(L"impostorSliceCount", m_impostorSliceCount);
	s >> Member< uint32_t >(L"updateFrequency", m_updateFrequency);
	s >> Member< float >(L"updatePositionThreshold", m_updatePositionThreshold, AttributeRange(0.0f));
	s >> Member< float >(L"updateDirectionThreshold", m_updateDirectionThreshold, AttributeRange(0.0f, 180.0f));
	s >> MemberComposite< CloudParticleData >(L"particleData", m_particleData);

	return true;
}

	}
}
