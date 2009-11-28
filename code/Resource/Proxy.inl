#include "Resource/IResourceHandle.h"

namespace traktor
{
	namespace resource
	{

template < typename ResourceType >
Proxy< ResourceType >::Proxy()
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Proxy< ResourceType >& rs)
:	m_handle(rs.m_handle)
,	m_resource(rs.m_resource)
,	m_guid(rs.m_guid)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(ResourceType* rs)
:	m_resource(rs)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Ref< ResourceType >& rs)
:	m_resource(rs)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Guid& guid)
:	m_guid(guid)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(IResourceHandle* handle)
:	m_handle(handle)
{
}

template < typename ResourceType >
const Guid& Proxy< ResourceType >::getGuid() const
{
	return m_guid;
}

template < typename ResourceType >
void Proxy< ResourceType >::replace(IResourceHandle* handle)
{
	m_handle = handle;

	// Get resource from new handle; we automatically try to validate proxy
	// here as it was done so previously.
	if (m_handle)
		m_resource = (ResourceType*)m_handle->get();
}

template < typename ResourceType >
bool Proxy< ResourceType >::valid() const
{
	if (!m_resource)
		return false;

	if (m_handle)
	{	
		if (m_resource != m_handle->get())
			return false;
	}

	return true;
}

template < typename ResourceType >
bool Proxy< ResourceType >::validate()
{
	if (m_handle)
		m_resource = (ResourceType*)m_handle->get();
	return m_resource != 0;
}

template < typename ResourceType >
inline Proxy< ResourceType >::operator ResourceType* ()
{
	return m_resource;
}

template < typename ResourceType >
inline Proxy< ResourceType >::operator Ref< ResourceType > ()
{
	return m_resource;
}

template < typename ResourceType >
inline ResourceType& Proxy< ResourceType >::operator * ()
{
	T_ASSERT_M (m_handle, L"Trying to dereference unbound proxy");
	T_ASSERT_M (m_resource, L"Trying to dereference null pointer");
	return *m_resource;
}

template < typename ResourceType >
inline ResourceType* Proxy< ResourceType >::operator -> ()
{
	T_ASSERT_M (m_handle, L"Trying to dereference unbound proxy");
	T_ASSERT_M (m_resource, L"Trying to call null pointer");
	return m_resource;
}

template < typename ResourceType >
inline const ResourceType* Proxy< ResourceType >::operator -> () const
{
	T_ASSERT_M (m_handle, L"Trying to dereference unbound proxy");
	T_ASSERT_M (m_resource, L"Trying to call null pointer");
	return m_resource;
}

template < typename ResourceType >
inline Proxy< ResourceType >& Proxy< ResourceType >::operator = (const Guid& guid)
{
	if (guid != m_guid)
	{
		m_handle = 0;
		m_guid = guid;
	}
	return *this;
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator == (const ResourceType* rs)
{
	return bool(m_resource == rs);
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator != (const ResourceType* rs)
{
	return bool(m_resource != rs);
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator == (const Ref< ResourceType >& rs)
{
	return bool(m_resource == rs);
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator != (const Ref< ResourceType >& rs)
{
	return bool(m_resource != rs);
}

	}
}
