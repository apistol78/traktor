/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/DragModifier.h"
#include "Spray/Modifiers/DragModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DragModifierData", 0, DragModifierData, ModifierData)

DragModifierData::DragModifierData()
:	m_linearDrag(0.0f)
,	m_angularDrag(0.0f)
{
}

Ref< const Modifier > DragModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if (abs(m_linearDrag) > FUZZY_EPSILON || abs(m_angularDrag) > FUZZY_EPSILON)
		return new DragModifier(m_linearDrag, m_angularDrag);
	else
		return 0;
}

void DragModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"linearDrag", m_linearDrag);
	s >> Member< float >(L"angularDrag", m_angularDrag);
}

	}
}
