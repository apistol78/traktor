#include "Resource/IResourceHandle.h"

namespace traktor
{
	namespace resource
	{

template < typename ResourceType >
Proxy< ResourceType >::Proxy()
:	Ref< ResourceType >()
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(ResourceType* resource)
:	Ref< ResourceType >(resource)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Ref< ResourceType >& resource)
:	Ref< ResourceType >(resource)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Proxy< ResourceType >& resource)
:	Ref< ResourceType >(resource)
,	m_handle(resource.m_handle)
,	m_guid(resource.m_guid)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(const Guid& guid)
:	Ref< ResourceType >()
,	m_guid(guid)
{
}

template < typename ResourceType >
Proxy< ResourceType >::Proxy(IResourceHandle* handle)
:	Ref< ResourceType >()
,	m_handle(handle)
{
	validate();
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
	validate();
}

template < typename ResourceType >
bool Proxy< ResourceType >::valid() const
{
	return bool(Ref< ResourceType >::ptr() != 0);
}

template < typename ResourceType >
bool Proxy< ResourceType >::validate()
{
	T_ASSERT_M (m_handle, L"Trying to validate unbound proxy");
	Ref< ResourceType >::replace(
		(ResourceType*)m_handle->get().ptr()
	);
	return valid();
}

template < typename ResourceType >
inline Proxy< ResourceType >::operator ResourceType* ()
{
	return Ref< ResourceType >::ptr();
}

template < typename ResourceType >
inline ResourceType& Proxy< ResourceType >::operator * ()
{
	T_ASSERT_M (valid() || m_handle, L"Trying to dereference unbound proxy");
	if (!valid())
		T_FATAL_ERROR;
	return *Ref< ResourceType >::ptr();
}

template < typename ResourceType >
inline ResourceType* Proxy< ResourceType >::operator -> ()
{
	T_ASSERT_M (valid() || m_handle, L"Trying to dereference unbound proxy");
	if (!valid())
		T_FATAL_ERROR;
	return Ref< ResourceType >::ptr();
}

template < typename ResourceType >
inline const ResourceType* Proxy< ResourceType >::operator -> () const
{
	T_ASSERT_M (valid() || m_handle, L"Trying to dereference unbound proxy");
	if (!valid())
		T_FATAL_ERROR;
	return Ref< ResourceType >::ptr();
}

template < typename ResourceType >
inline Proxy< ResourceType >& Proxy< ResourceType >::operator = (const Guid& guid)
{
	if (guid != m_guid)
	{
		Ref< ResourceType >::replace(0);
		m_handle = 0;
		m_guid = guid;
	}
	return *this;
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator == (const ResourceType* resource)
{
	return bool(Ref< ResourceType >::ptr() == resource);
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator != (const ResourceType* resource)
{
	return bool(Ref< ResourceType >::ptr() != resource);
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator == (const Ref< ResourceType >& ref)
{
	return bool(Ref< ResourceType >::ptr() == ref.ptr());
}

template < typename ResourceType >
inline bool Proxy< ResourceType >::operator != (const Ref< ResourceType >& ref)
{
	return bool(Ref< ResourceType >::ptr() != ref.ptr());
}

	}
}
