#ifndef traktor_resource_ResourceLoader_H
#define traktor_resource_ResourceLoader_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class ResourceFactory;

/*! \brief Resource loader.
 *
 * A resource loader object is a set of resource factories.
 * It's only responsibility is to determine which factory to
 * use for a specific resource type.
 *
 * The resource manager is invoking the loader when a resource
 * needs to be resolved.
 */
class T_DLLCLASS ResourceLoader : public Object
{
	T_RTTI_CLASS(ResourceLoader)

public:
	void addFactory(ResourceFactory* factory);

	Object* load(const Type& type, const Guid& guid);

	template < typename T >
	T* load(const Guid& guid)
	{
		return dynamic_type_cast< T* >(load(T::getClassType(), guid));
	}

private:
	RefArray< ResourceFactory > m_factories;
};

	}
}

#endif	// traktor_resource_ResourceLoader_H
