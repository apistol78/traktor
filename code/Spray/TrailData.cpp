/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/Trail.h"
#include "Spray/TrailData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.TrailData", 1, TrailData, ISerializable)

TrailData::TrailData()
:	m_width(1.0f)
,	m_age(1.0f)
,	m_lengthThreshold(1.0f)
,	m_breakThreshold(0.0f)
{
}

Ref< Trail > TrailData::createTrail(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	return new Trail(shader, m_width, m_age, m_lengthThreshold, m_breakThreshold);
}

void TrailData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"width", m_width);

	if (s.getVersion< TrailData >() >= 1)
		s >> Member< float >(L"age", m_age);

	s >> Member< float >(L"lengthThreshold", m_lengthThreshold);
	s >> Member< float >(L"breakThreshold", m_breakThreshold);
}

	}
}
