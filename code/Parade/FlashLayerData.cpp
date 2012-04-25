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

	resource::Proxy< script::IScriptContext > script;
	resource::Proxy< flash::FlashMovie > movie;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return 0;
	if (m_script && !resourceManager->bind(m_script, script))
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
