#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/BrownianModifier.h"
#include "Spray/Modifiers/BrownianModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.BrownianModifierData", 0, BrownianModifierData, ModifierData)

Ref< const Modifier > BrownianModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if (abs(m_factor) > FUZZY_EPSILON)
		return new BrownianModifier(m_factor);
	else
		return nullptr;
}

void BrownianModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"factor", m_factor);
}

	}
}
