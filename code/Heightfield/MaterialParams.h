#ifndef traktor_hf_MaterialParams_H
#define traktor_hf_MaterialParams_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS MaterialParams : public Object
{
	T_RTTI_CLASS;

public:
	void set(Object* object);

	Object* get(const TypeInfo& objectType) const;

	template < typename T >
	T* get() const
	{
		return checked_type_cast< T*, true >(get(type_of< T >()));
	}

private:
	SmallMap< const TypeInfo*, Ref< Object > > m_objects;
};

	}
}

#endif	// traktor_hf_MaterialParams_H
