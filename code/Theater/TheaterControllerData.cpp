#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/TheaterController.h"
#include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerData", 1, TheaterControllerData, scene::ISceneControllerData)

TheaterControllerData::TheaterControllerData()
:	m_repeatActs(false)
,	m_randomizeActs(false)
{
}

Ref< scene::ISceneController > TheaterControllerData::createController(const SmallMap< Guid, Ref< world::Entity > >& entityProducts, bool editor) const
{
	RefArray< const Act > acts(m_acts.size());
	for (size_t i = 0; i < m_acts.size(); ++i)
	{
		acts[i] = m_acts[i]->createInstance(entityProducts);
		if (!acts[i])
			return nullptr;
	}

	// Do not randomize acts if in editor.
	if (!editor && m_randomizeActs)
	{
		AlignedVector< size_t > indices(acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			indices[i] = i;

		std::random_shuffle(indices.begin(), indices.end());

		RefArray< const Act > tmp(m_acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			tmp[i] = acts[indices[i]];

		acts = tmp;
	}

	return new TheaterController(acts, m_repeatActs);
}

float TheaterControllerData::getActStartTime(int32_t act) const
{
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < std::min(act, int32_t(m_acts.size())); ++i)
		actStartTime += m_acts[i]->getDuration();
	return actStartTime;
}

void TheaterControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< TheaterControllerData >() >= 1);

	s >> MemberRefArray< ActData >(L"acts", m_acts);
	s >> Member< bool >(L"repeatActs", m_repeatActs);
	s >> Member< bool >(L"randomizeActs", m_randomizeActs);
}

	}
}
