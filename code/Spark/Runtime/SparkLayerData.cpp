/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Spark/Movie.h"
#include "Spark/Runtime/SparkLayer.h"
#include "Spark/Runtime/SparkLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkLayerData", 1, SparkLayerData, runtime::LayerData)

SparkLayerData::SparkLayerData()
:	m_clearBackground(false)
,	m_enableSound(true)
{
}

Ref< runtime::Layer > SparkLayerData::createInstance(runtime::Stage* stage, runtime::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< Movie > movie;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return nullptr;

	// Bind external movies.
	std::map< std::wstring, resource::Proxy< Movie > > externalMovies;
	for (std::map< std::wstring, resource::Id< Movie > >::const_iterator i = m_externalMovies.begin(); i != m_externalMovies.end(); ++i)
	{
		if (!resourceManager->bind(i->second, externalMovies[i->first]))
			return nullptr;
	}

	// Create layer instance.
	return new SparkLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		movie,
		externalMovies,
		m_clearBackground,
		m_enableSound
	);
}

void SparkLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< Movie >(L"movie", m_movie);

	s >> MemberStlMap<
		std::wstring,
		resource::Id< Movie >,
		Member< std::wstring >,
		resource::Member< Movie >
	>(L"externalMovies", m_externalMovies);

	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);

	if (s.getVersion< SparkLayerData >() < 1)
	{
		bool enableShapeCache;
		s >> Member< bool >(L"enableShapeCache", enableShapeCache);

		bool enableDirtyRegions;
		s >> Member< bool >(L"enableDirtyRegions", enableDirtyRegions);

		uint32_t contextSize;
		s >> Member< uint32_t >(L"contextSize", contextSize);
	}
}

	}
}
