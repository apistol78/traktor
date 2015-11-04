#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Clouds/CloudMask.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.CloudComponentData", 0, CloudComponentData, world::IEntityComponentData)

CloudComponentData::CloudComponentData()
:	m_impostorTargetResolution(256)
,	m_impostorSliceCount(1)
,	m_updateFrequency(10)
,	m_updatePositionThreshold(0.1f)
,	m_updateDirectionThreshold(0.1f)
{
}

Ref< CloudComponent > CloudComponentData::createComponent(world::Entity* owner, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > particleShader;
	resource::Proxy< render::ITexture > particleTexture;
	resource::Proxy< render::Shader > impostorShader;
	resource::Proxy< CloudMask > mask;

	if (!resourceManager->bind(m_particleShader, particleShader))
		return 0;
	if (!resourceManager->bind(m_particleTexture, particleTexture))
		return 0;
	if (!resourceManager->bind(m_impostorShader, impostorShader))
		return 0;

	if (m_mask)
	{
		if (!resourceManager->bind(m_mask, mask))
			return 0;
	}

	Ref< CloudComponent > cloudComponent = new CloudComponent(owner);
	if (cloudComponent->create(
		renderSystem,
		particleShader,
		particleTexture,
		impostorShader,
		mask,
		m_impostorTargetResolution,
		m_impostorSliceCount,
		m_updateFrequency,
		m_updatePositionThreshold,
		m_updateDirectionThreshold,
		m_particleData
	))
		return cloudComponent;

	return 0;
}

void CloudComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"particleShader", m_particleShader);
	s >> resource::Member< render::ITexture >(L"particleTexture", m_particleTexture);
	s >> resource::Member< render::Shader >(L"impostorShader", m_impostorShader);
	s >> resource::Member< CloudMask >(L"mask", m_mask);
	s >> Member< uint32_t >(L"impostorTargetResolution", m_impostorTargetResolution);
	s >> Member< uint32_t >(L"impostorSliceCount", m_impostorSliceCount);
	s >> Member< uint32_t >(L"updateFrequency", m_updateFrequency);
	s >> Member< float >(L"updatePositionThreshold", m_updatePositionThreshold, AttributeRange(0.0f));
	s >> Member< float >(L"updateDirectionThreshold", m_updateDirectionThreshold, AttributeRange(0.0f, 180.0f));
	s >> MemberComposite< CloudParticleData >(L"particleData", m_particleData);
}

	}
}
