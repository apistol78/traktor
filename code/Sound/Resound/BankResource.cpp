/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/GrainFactory.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Resound/IGrainData.h"
#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 7, BankResource, ISoundResource)

BankResource::BankResource()
:	m_gain(0.0f)
,	m_range(0.0f)
{
}

BankResource::BankResource(const RefArray< IGrainData >& grains, const std::wstring& category, float gain, float range)
:	m_grains(grains)
,	m_category(category)
,	m_gain(gain)
,	m_range(range)
{
}

Ref< Sound > BankResource::createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const
{
	GrainFactory grainFactory(resourceManager);
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = grainFactory.createInstance(m_grains[i]);
		if (!grains[i])
			return nullptr;
	}

	return new Sound(
		new BankBuffer(grains),
		getParameterHandle(m_category),
		m_gain,
		m_range
	);
}

void BankResource::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion() >= 7);
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"range", m_range);
}

	}
}
