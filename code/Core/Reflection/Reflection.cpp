#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/ReflectionApplySerializer.h"
#include "Core/Reflection/ReflectionInspectSerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Reflection", Reflection, RfmCompound)

Ref< Reflection > Reflection::create(const ISerializable* object)
{
	if (!object)
		return nullptr;

	Ref< Reflection > r = new Reflection(type_of(object));

	ReflectionInspectSerializer s(r);
	s.serialize(const_cast< ISerializable* >(object));

	return r;
}

bool Reflection::apply(ISerializable* object) const
{
	ReflectionApplySerializer s(this);
	s.serialize(object);
	return true;
}

Ref< ISerializable > Reflection::clone() const
{
	Ref< ISerializable > object = checked_type_cast< ISerializable* >(m_objectType.createInstance());
	if (apply(object))
		return object;
	else
		return nullptr;
}

Reflection::Reflection(const TypeInfo& objectType)
:	RfmCompound(L"")
,	m_objectType(objectType)
{
}

}
