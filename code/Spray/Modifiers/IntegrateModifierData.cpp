#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/IntegrateModifier.h"
#include "Spray/Modifiers/IntegrateModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.IntegrateModifierData", 1, IntegrateModifierData, ModifierData)

IntegrateModifierData::IntegrateModifierData()
:	m_timeScale(1.0f)
,	m_linear(true)
,	m_angular(true)
{
}

Ref< Modifier > IntegrateModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new IntegrateModifier(m_timeScale, m_linear, m_angular);
}

void IntegrateModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"timeScale", m_timeScale);
	
	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"linear", m_linear);
		s >> Member< bool >(L"angular", m_angular);
	}
}

	}
}
