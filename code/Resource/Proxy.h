#ifndef traktor_resource_Proxy_H
#define traktor_resource_Proxy_H

#include "Core/Heap/Ref.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace resource
	{

class IResourceHandle;

/*! \brief Resource proxy.
 * \ingroup Resource
 *
 * A resource proxy is a reference wrapper with the
 * added functionality to resolve the object when
 * the reference is being dereferenced.
 */
template < typename ResourceType >
class Proxy : public Ref< ResourceType >
{
public:
	Proxy< ResourceType >();

	Proxy< ResourceType >(ResourceType* resource);

	Proxy< ResourceType >(const Ref< ResourceType >& resource);

	Proxy< ResourceType >(const Proxy< ResourceType >& resource);

	Proxy< ResourceType >(const Guid& guid);

	Proxy< ResourceType >(IResourceHandle* handle);
	
	/*! \brief Get resource's guid. */
	const Guid& getGuid() const;

	/*! \brief Replace resource handle. */
	void replace(IResourceHandle* handle);

	/*! \brief Check if proxy contains a valid pointer. */
	bool valid() const;

	/*! \brief Validate proxy; update resource if it's been replaced in handle. */
	bool validate();

	inline operator ResourceType* ();
	
	inline ResourceType& operator * ();

	inline ResourceType* operator -> ();

	inline const ResourceType* operator -> () const;

	inline Proxy< ResourceType >& operator = (const Guid& guid);

	inline bool operator == (const ResourceType* resource);

	inline bool operator != (const ResourceType* resource);

	inline bool operator == (const Ref< ResourceType >& ref);

	inline bool operator != (const Ref< ResourceType >& ref);

private:
	Ref< IResourceHandle > m_handle;
	Guid m_guid;
};

	}
}

#include "Resource/Proxy.inl"

#endif	// traktor_resource_Proxy_H
