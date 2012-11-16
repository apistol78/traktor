#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Parade/LayerData.h"
#include "Parade/Stage.h"
#include "Parade/StageData.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.StageData", 0, StageData, ISerializable)

Ref< Stage > StageData::createInstance(amalgam::IEnvironment* environment, const Object* params) const
{
	Ref< Stage > stage = new Stage(environment, m_transitions, params);
	for (RefArray< LayerData >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Ref< Layer > layer = (*i)->createInstance(stage, environment);
		if (!layer)
			return 0;

		stage->addLayer(layer);
	}
	return stage;
}

bool StageData::serialize(ISerializer& s)
{
	s >> MemberRefArray< LayerData >(L"layers", m_layers);
	s >> MemberStlMap< std::wstring, Guid >(L"transitions", m_transitions);
	return true;
}

	}
}
