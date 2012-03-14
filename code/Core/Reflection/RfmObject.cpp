#include "Core/Reflection/RfmObject.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmObject", RfmObject, ReflectionMember)

RfmObject::RfmObject(const wchar_t* name, ISerializable* value)
:	ReflectionMember(name)
,	m_value(value)
{
}

}
