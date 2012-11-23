#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Flash/FlashMovie.h"
#include "Parade/FlashLayer.h"
#include "Parade/FlashLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.FlashLayerData", 0, FlashLayerData, LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
,	m_enableSound(true)
{
}

Ref< Layer > FlashLayerData::createInstance(Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< flash::FlashMovie > movie;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return 0;

	// Create layer instance.
	return new FlashLayer(
		stage,
		m_name,
		environment,
		movie,
		m_clearBackground,
		m_enableSound
	);
}

bool FlashLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< flash::FlashMovie >(L"movie", m_movie);
	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);

	return true;
}

	}
}
