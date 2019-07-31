#include "Runtime/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Spark/Movie.h"
#include "Spark/Runtime/SparkLayer.h"
#include "Spark/Runtime/SparkLayerData.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkLayerData", 0, SparkLayerData, runtime::LayerData)

SparkLayerData::SparkLayerData()
:	m_clearBackground(false)
,	m_enableShapeCache(false)
,	m_enableDirtyRegions(false)
,	m_enableSound(true)
,	m_contextSize(1)
{
}

Ref< runtime::Layer > SparkLayerData::createInstance(runtime::Stage* stage, runtime::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< Movie > movie;
	resource::Proxy< render::ImageProcessSettings > imageProcess;

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

	// Bind optional post processing.
	if (m_imageProcess)
	{
		if (!resourceManager->bind(m_imageProcess, imageProcess))
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
		imageProcess,
		m_clearBackground,
		m_enableShapeCache,
		m_enableDirtyRegions,
		m_enableSound,
		m_contextSize
	);
}

void SparkLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< Movie >(L"movie", m_movie);

	s >> MemberStlMap<
		std::wstring,
		resource::Id< Movie >,
		MemberStlPair<
			std::wstring,
			resource::Id< Movie >,
			Member< std::wstring >,
			resource::Member< Movie >
		>
	>(L"externalMovies", m_externalMovies);

	s >> resource::Member< render::ImageProcessSettings >(L"imageProcess", m_imageProcess);
	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);
	s >> Member< bool >(L"enableShapeCache", m_enableShapeCache);
	s >> Member< bool >(L"enableDirtyRegions", m_enableDirtyRegions);
	s >> Member< uint32_t >(L"contextSize", m_contextSize);
}

	}
}
