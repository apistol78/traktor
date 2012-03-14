#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/ReflectionApplySerializer.h"
#include "Core/Reflection/ReflectionInspectSerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Reflection", Reflection, RfmCompound)

Ref< Reflection > Reflection::create(const ISerializable* object)
{
	Ref< Reflection > r = new Reflection(type_of(object));

	ReflectionInspectSerializer s(r);
	if (!s.serialize(const_cast< ISerializable* >(object), type_of(object).getVersion(), 0))
		return 0;

	return r;
}

bool Reflection::apply(ISerializable* object) const
{
	ReflectionApplySerializer s(this);
	return s.serialize(object, type_of(object).getVersion(), 0);
}

Ref< ISerializable > Reflection::clone() const
{
	Ref< ISerializable > object = checked_type_cast< ISerializable* >(m_objectType.createInstance());
	if (apply(object))
		return object;
	else
		return 0;
}

Reflection::Reflection(const TypeInfo& objectType)
:	RfmCompound(L"")
,	m_objectType(objectType)
{
}

}
