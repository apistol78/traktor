#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Resound/BlendGrain.h"
#include "Sound/Resound/BlendGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BlendGrainData", 1, BlendGrainData, IGrainData)

Ref< IGrain > BlendGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< IGrain > grains[2];

	grains[0] = m_grains[0]->createInstance(resourceManager);
	if (!grains[0])
		return 0;

	grains[1] = m_grains[1]->createInstance(resourceManager);
	if (!grains[1])
		return 0;

	return new BlendGrain(
		getParameterHandle(m_id),
		grains[0],
		grains[1]
	);
}

void BlendGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"id", m_id);

	s >> MemberStaticArray< Ref< IGrainData >, 2, MemberRef< IGrainData > >(L"grains", m_grains);
}

	}
}
