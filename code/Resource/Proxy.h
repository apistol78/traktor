#ifndef traktor_resource_Proxy_H
#define traktor_resource_Proxy_H

#include "Core/Heap/Ref.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace resource
	{

/*! \brief Resource proxy.
 *
 * A resource proxy is a reference wrapper with the
 * added functionality to resolve the object when
 * the reference is being dereferenced.
 */
template < typename T >
class Proxy : public Ref< T >
{
public:
	Proxy< T >();

	Proxy< T >(T* resource);

	Proxy< T >(const Ref< T >& resource);

	Proxy< T >(T* resource, const Guid& guid);

	Proxy< T >(const Ref< T >& resource, const Guid& guid);

	Proxy< T >(const Proxy< T >& resource);

	Proxy< T >(const Guid& guid);
	
	const Guid& getGuid() const;
	
	/*! \brief Check if proxy contains a valid pointer. */
	bool valid() const;

	/*! \brief Validate resource container by loading resource. */
	bool validate();

	/*! \brief Flush resource container. */
	void flush();

	inline operator T* ();
	
	inline T& operator * ();

	inline T* operator -> ();

	inline const T* operator -> () const;

	inline Proxy< T >& operator = (const Guid& guid);

	inline bool operator == (const T* resource);

	inline bool operator != (const T* resource);

	inline bool operator == (const Ref< T >& ref);

	inline bool operator != (const Ref< T >& ref);

private:
	Guid m_guid;
};

	}
}

#include "Resource/Proxy.inl"

#endif	// traktor_resource_Proxy_H
