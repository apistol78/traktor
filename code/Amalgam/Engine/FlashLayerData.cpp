#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/FlashLayer.h"
#include "Amalgam/Engine/FlashLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMovie.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.FlashLayerData", 2, FlashLayerData, LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
,	m_enableSound(true)
{
}

Ref< Layer > FlashLayerData::createInstance(Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< flash::FlashMovie > movie;
	resource::Proxy< world::PostProcessSettings > postProcess;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return 0;

	// Bind external movies.
	std::map< std::wstring, resource::Proxy< flash::FlashMovie > > externalMovies;
	for (std::map< std::wstring, resource::Id< flash::FlashMovie > >::const_iterator i = m_externalMovies.begin(); i != m_externalMovies.end(); ++i)
	{
		if (!resourceManager->bind(i->second, externalMovies[i->first]))
			return 0;
	}
	
	// Bind optional post processing.
	if (m_postProcess)
	{
		if (!resourceManager->bind(m_postProcess, postProcess))
			return 0;
	}

	// Create layer instance.
	return new FlashLayer(
		stage,
		m_name,
		environment,
		movie,
		externalMovies,
		postProcess,
		m_clearBackground,
		m_enableSound
	);
}

void FlashLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< flash::FlashMovie >(L"movie", m_movie);

	if (s.getVersion() >= 1)
		s >> MemberStlMap<
			std::wstring,
			resource::Id< flash::FlashMovie >,
			MemberStlPair<
				std::wstring,
				resource::Id< flash::FlashMovie >,
				Member< std::wstring >,
				resource::Member< flash::FlashMovie >
			>
		>(L"externalMovies", m_externalMovies);

	if (s.getVersion() >= 2)
		s >> resource::Member< world::PostProcessSettings >(L"postProcess", m_postProcess);

	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);
}

	}
}
