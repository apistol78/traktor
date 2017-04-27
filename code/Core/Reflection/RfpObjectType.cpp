/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpObjectType.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpObjectType", RfpObjectType, ReflectionMemberPredicate)

RfpObjectType::RfpObjectType(const TypeInfo& objectType)
:	m_objectType(objectType)
{
}

bool RfpObjectType::operator () (const ReflectionMember* member) const
{
	T_ASSERT (member);
	if (const RfmObject* objectMember = dynamic_type_cast< const RfmObject* >(member))
	{
		if (objectMember->get())
		{
			const TypeInfo& objectType = type_of(objectMember->get());
			if (is_type_of(m_objectType, objectType))
				return true;
		}
	}
	return false;
}

}
