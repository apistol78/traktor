#include "Parade/AudioLayer.h"
#include "Parade/AudioLayerData.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.AudioLayerData", 0, AudioLayerData, LayerData)

Ref< Layer > AudioLayerData::createInstance(amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	resource::Proxy< script::IScriptContext > script;
	if (m_script && !resourceManager->bind(m_script, script))
		return 0;

	return new AudioLayer(
		m_name,
		environment,
		script
	);
}

bool AudioLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	return true;
}

	}
}
