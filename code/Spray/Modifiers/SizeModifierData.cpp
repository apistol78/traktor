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

Ref< Modifier > SizeModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new SizeModifier(m_adjustRate);
}

bool SizeModifierData::serialize(ISerializer& s)
{
	return s >> Member< float >(L"adjustRate", m_adjustRate);
}

	}
}
