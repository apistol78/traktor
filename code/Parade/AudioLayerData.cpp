#include "Parade/AudioLayer.h"
#include "Parade/AudioLayerData.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.AudioLayerData", 0, AudioLayerData, LayerData)

Ref< Layer > AudioLayerData::createInstance(amalgam::IEnvironment* environment) const
{
	return new AudioLayer(
		m_name,
		environment,
		m_script
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
