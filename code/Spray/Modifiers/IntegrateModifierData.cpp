#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/IntegrateModifier.h"
#include "Spray/Modifiers/IntegrateModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.IntegrateModifierData", 0, IntegrateModifierData, ModifierData)

IntegrateModifierData::IntegrateModifierData()
:	m_timeScale(1.0f)
{
}

Ref< Modifier > IntegrateModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new IntegrateModifier(m_timeScale);
}

bool IntegrateModifierData::serialize(ISerializer& s)
{
	return s >> Member< float >(L"timeScale", m_timeScale);
}

	}
}
