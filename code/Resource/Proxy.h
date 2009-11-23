#ifndef traktor_resource_Proxy_H
#define traktor_resource_Proxy_H

#include "Core/Guid.h"
#include "Core/Ref.h"

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
class Proxy
{
public:
	Proxy< ResourceType >();

	Proxy< ResourceType >(const Proxy< ResourceType >& rs);

	Proxy< ResourceType >(ResourceType* rs);

	Proxy< ResourceType >(const Ref< ResourceType >& rs);

	Proxy< ResourceType >(const Guid& guid);

	Proxy< ResourceType >(IResourceHandle* handle);
	
	/*! \brief Get resource's guid. */
	const Guid& getGuid() const;

	/*! \brief Replace resource handle. */
	void replace(IResourceHandle* handle);

	/*! \brief Check if proxy is valid. */
	bool valid() const;

	/*! \brief Validate proxy; ie get resource from handle. */
	bool validate();

	inline operator ResourceType* ();

	inline operator Ref< ResourceType > ();
	
	inline ResourceType& operator * ();

	inline ResourceType* operator -> ();

	inline const ResourceType* operator -> () const;

	inline Proxy< ResourceType >& operator = (const Guid& guid);

	inline bool operator == (const ResourceType* rs);

	inline bool operator != (const ResourceType* rs);

	inline bool operator == (const Ref< ResourceType >& rs);

	inline bool operator != (const Ref< ResourceType >& rs);

private:
	Ref< IResourceHandle > m_handle;
	Ref< ResourceType > m_resource;
	Guid m_guid;
};

	}
}

#include "Resource/Proxy.inl"

#endif	// traktor_resource_Proxy_H
