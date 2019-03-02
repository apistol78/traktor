#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/SizeModifier.h"
#include "Spray/Modifiers/SizeModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SizeModifierData", 0, SizeModifierData, ModifierData)

SizeModifierData::SizeModifierData()
:	m_adjustRate(0.0f)
{
}

Ref< const Modifier > SizeModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if (abs(m_adjustRate) > FUZZY_EPSILON)
		return new SizeModifier(m_adjustRate);
	else
		return 0;
}

void SizeModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"adjustRate", m_adjustRate);
}

	}
}
