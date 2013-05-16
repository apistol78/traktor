#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/TheaterController.h"
#include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerData", 0, TheaterControllerData, scene::ISceneControllerData)

Ref< scene::ISceneController > TheaterControllerData::createController(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const
{
	RefArray< const Act > acts(m_acts.size());
	for (size_t i = 0; i < m_acts.size(); ++i)
	{
		acts[i] = m_acts[i]->createInstance(entityProducts);
		if (!acts[i])
			return 0;
	}
	return new TheaterController(acts);
}

void TheaterControllerData::serialize(ISerializer& s)
{
	s >> MemberRefArray< ActData >(L"acts", m_acts);
}

	}
}
