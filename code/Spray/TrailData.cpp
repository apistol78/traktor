/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/TrailData.h"

#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/Trail.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.TrailData", 0, TrailData, ISerializable)

Ref< Trail > TrailData::createTrail(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	return new Trail(shader, m_width, m_age, m_lengthThreshold, m_breakThreshold);
}

void TrailData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"width", m_width);
	s >> Member< float >(L"age", m_age);
	s >> Member< float >(L"lengthThreshold", m_lengthThreshold);
	s >> Member< float >(L"breakThreshold", m_breakThreshold);
}

}
