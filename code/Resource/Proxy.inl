#include "Resource/ResourceManager.h"
#include "Resource/ResourceCache.h"

namespace traktor
{
	namespace resource
	{

template < typename T >
Proxy< T >::Proxy()
:	Ref< T >()
{
}

template < typename T >
Proxy< T >::Proxy(T* resource)
:	Ref< T >(resource)
{
}

template < typename T >
Proxy< T >::Proxy(const Ref< T >& resource)
:	Ref< T >(resource)
{
}

template < typename T >
Proxy< T >::Proxy(T* resource, const Guid& guid)
:	Ref< T >(resource)
,	m_guid(guid)
{
	ResourceManager::getInstance().setResource(T::getClassType(), guid, resource);
}

template < typename T >
Proxy< T >::Proxy(const Ref< T >& resource, const Guid& guid)
:	Ref< T >(resource)
,	m_guid(guid)
{
	ResourceManager::getInstance().setResource(T::getClassType(), guid, resource);
}

template < typename T >
Proxy< T >::Proxy(const Proxy< T >& resource)
:	Ref< T >(resource)
,	m_guid(resource.m_guid)
{
}

template < typename T >
Proxy< T >::Proxy(const Guid& guid)
:	Ref< T >()
,	m_guid(guid)
{
	ResourceManager::getInstance().requestResource(T::getClassType(), guid);
}

template < typename T >
const Guid& Proxy< T >::getGuid() const
{
	return m_guid;
}

template < typename T >
bool Proxy< T >::valid() const
{
	return bool(Ref< T >::getPtr() != 0);
}

template < typename T >
bool Proxy< T >::validate()
{
	if (valid())
		return true;

	Ref< T > tmp = checked_type_cast< T* >(ResourceManager::getInstance().getResource(T::getClassType(), m_guid));
	if (!tmp)
		return false;

	setPtr(tmp);
	return true;
}

template < typename T >
void Proxy< T >::flush()
{
	if (Ref< T >::getPtr())
	{
		Ref< IResourceCache > cache = ResourceManager::getInstance().getCache();
		if (cache)
		{
			cache->flush(m_guid);
			T_ASSERT (Ref< T >::getPtr() == 0);
		}
	}
}

template < typename T >
inline Proxy< T >::operator T* ()
{
	if (!validate())
		T_FATAL_ERROR;
	return Ref< T >::getPtr();
}

template < typename T >
inline T& Proxy< T >::operator * ()
{
	if (!validate())
		T_FATAL_ERROR;
	return *Ref< T >::getPtr();
}

template < typename T >
inline T* Proxy< T >::operator -> ()
{
	if (!validate())
		T_FATAL_ERROR;
	return Ref< T >::getPtr();
}

template < typename T >
inline const T* Proxy< T >::operator -> () const
{
	return Ref< T >::getPtr();
}

template < typename T >
inline Proxy< T >& Proxy< T >::operator = (const Guid& guid)
{
	if (guid != m_guid)
	{
		m_guid = guid;
		Ref< T >::setPtr(0);
		ResourceManager::getInstance().requestResource(T::getClassType(), guid);
	}
	return *this;
}

template < typename T >
inline bool Proxy< T >::operator == (const T* resource)
{
	if (!validate())
		T_FATAL_ERROR;
	return bool(Ref< T >::getPtr() == resource);
}

template < typename T >
inline bool Proxy< T >::operator != (const T* resource)
{
	if (!validate())
		T_FATAL_ERROR;
	return bool(Ref< T >::getPtr() != resource);
}

template < typename T >
inline bool Proxy< T >::operator == (const Ref< T >& ref)
{
	if (!validate())
		T_FATAL_ERROR;
	return bool(Ref< T >::getPtr() == ref.getPtr());
}

template < typename T >
inline bool Proxy< T >::operator != (const Ref< T >& ref)
{
	if (!validate())
		T_FATAL_ERROR;
	return bool(Ref< T >::getPtr() != ref.getPtr());
}

	}
}

