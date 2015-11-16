#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/FlashLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMovie.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.FlashLayerData", LayerData::Version, FlashLayerData, LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
,	m_enableSound(true)
,	m_contextSize(1)
{
}

Ref< Layer > FlashLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< flash::FlashMovie > movie;
	resource::Proxy< render::ImageProcessSettings > imageProcess;

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
	if (m_imageProcess)
	{
		if (!resourceManager->bind(m_imageProcess, imageProcess))
			return 0;
	}

	// Create layer instance.
	return new FlashLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		movie,
		externalMovies,
		imageProcess,
		m_clearBackground,
		m_enableSound,
		m_contextSize
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
		s >> resource::Member< render::ImageProcessSettings >(L"imageProcess", m_imageProcess);

	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);

	if (s.getVersion() >= 4)
		s >> Member< uint32_t >(L"contextSize", m_contextSize);
}

	}
}
