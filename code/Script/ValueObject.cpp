#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/ValueObject.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ValueObject", 0, ValueObject, IValue)

ValueObject::ValueObject()
:	m_objectRef(0)
{
}

ValueObject::ValueObject(uint32_t objectRef)
:	m_objectRef(objectRef)
{
}

void ValueObject::setObjectRef(uint32_t objectRef)
{
	m_objectRef = objectRef;
}

uint32_t ValueObject::getObjectRef() const
{
	return m_objectRef;
}

void ValueObject::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"objectRef", m_objectRef);
}

	}
}
