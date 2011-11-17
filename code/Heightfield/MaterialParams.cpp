#include "Heightfield/MaterialParams.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.MaterialParams", MaterialParams, Object)

void MaterialParams::set(Object* object)
{
	T_ASSERT (object);
	m_objects[&type_of(object)] = object;
}

Object* MaterialParams::get(const TypeInfo& objectType) const
{
	SmallMap< const TypeInfo*, Ref< Object > >::const_iterator i = m_objects.find(&objectType);
	return i != m_objects.end() ? i->second : 0;
}

	}
}
