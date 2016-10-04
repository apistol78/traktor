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

ValueObject::ValueObject(uint32_t objectRef, const std::wstring& valueOf)
:	m_objectRef(objectRef)
,	m_valueOf(valueOf)
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

void ValueObject::setValueOf(const std::wstring& valueOf)
{
	m_valueOf = valueOf;
}

const std::wstring& ValueObject::getValueOf() const
{
	return m_valueOf;
}

void ValueObject::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"objectRef", m_objectRef);
	s >> Member< std::wstring >(L"valueOf", m_valueOf);
}

	}
}
