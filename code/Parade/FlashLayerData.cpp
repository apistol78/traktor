#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Flash/FlashMovie.h"
#include "Parade/FlashLayer.h"
#include "Parade/FlashLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.FlashLayerData", 1, FlashLayerData, LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
{
}

Ref< Layer > FlashLayerData::createInstance(amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	// Create a copy of all immutable proxies.
	resource::Proxy< script::IScriptContext > script = m_script;
	resource::Proxy< flash::FlashMovie > movie = m_movie;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(script) ||
		!resourceManager->bind(movie)
	)
		return 0;

	// Create layer instance.
	return new FlashLayer(
		m_name,
		environment,
		script,
		movie,
		m_clearBackground
	);
}

bool FlashLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< flash::FlashMovie >(L"movie", m_movie);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"clearBackground", m_clearBackground);

	return true;
}

	}
}
