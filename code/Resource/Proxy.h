#ifndef traktor_resource_Proxy_H
#define traktor_resource_Proxy_H

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Resource/IResourceHandle.h"

namespace traktor
{
	namespace resource
	{

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
	Proxy< ResourceType >()
	{
	}

	Proxy< ResourceType >(const Proxy< ResourceType >& rs)
	:	m_handle(rs.m_handle)
	,	m_resource(rs.m_resource)
	,	m_guid(rs.m_guid)
	{
	}

	Proxy< ResourceType >(const Guid& guid)
	:	m_guid(guid)
	{
	}

	Proxy< ResourceType >(IResourceHandle* handle)
	:	m_handle(handle)
	{
	}

	Proxy< ResourceType >(ResourceType* rs)
	:	m_resource(rs)
	{
	}

	Proxy< ResourceType >(const Ref< ResourceType >& rs)
	:	m_resource(rs)
	{
	}

	template < typename DerivedType >
	Proxy< ResourceType >(const Proxy< DerivedType >& rs)
	:	m_handle(rs.m_handle)
	,	m_resource(rs.m_resource)
	,	m_guid(rs.m_guid)
	{
	}

	template < typename DerivedType >
	Proxy< ResourceType >(const Ref< DerivedType >& rs)
	:	m_resource(rs)
	{
	}

	/*! \brief Get resource's handle. */
	IResourceHandle* getHandle() const
	{
		return m_handle;
	}

	/*! \brief Get resource's guid. */
	const Guid& getGuid() const
	{
		return m_guid;
	}

	/*! \brief Replace resource handle.
	 *
	 * \param handle New resource handle.
	 */
	void replace(IResourceHandle* handle)
	{
		m_handle = handle;
		validate();
	}

	/*! \brief Check if proxy is valid. */
	bool valid() const
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

	/*! \brief Validate proxy; ie get resource from handle. */
	bool validate()
	{
		if (m_handle)
			m_resource = (ResourceType*)m_handle->get();
		return m_resource != 0;
	}

	/*! \brief Invalidate proxy; reset resource. */
	void invalidate()
	{
		m_resource = 0;
	}

	inline operator ResourceType* ()
	{
		return m_resource;
	}

	inline operator Ref< ResourceType > ()
	{
		return m_resource;
	}

	inline ResourceType& operator * ()
	{
		T_ASSERT_M (m_resource, L"Trying to dereference null pointer");
		return *m_resource;
	}

	inline ResourceType* operator -> ()
	{
		T_ASSERT_M (m_resource, L"Trying to call null pointer");
		return m_resource;
	}

	inline const ResourceType* operator -> () const
	{
		T_ASSERT_M (m_resource, L"Trying to call null pointer");
		return m_resource;
	}

	inline Proxy< ResourceType >& operator = (const Guid& guid)
	{
		if (guid != m_guid)
		{
			m_handle = 0;
			m_guid = guid;
		}
		return *this;
	}

	inline bool operator == (const ResourceType* rs)
	{
		return bool(m_resource == rs);
	}

	inline bool operator != (const ResourceType* rs)
	{
		return bool(m_resource != rs);
	}

	inline bool operator == (const Ref< ResourceType >& rs)
	{
		return bool(m_resource == rs);
	}

	inline bool operator != (const Ref< ResourceType >& rs)
	{
		return bool(m_resource != rs);
	}

private:
	Ref< IResourceHandle > m_handle;
	Ref< ResourceType > m_resource;
	Guid m_guid;
};

	}
}

#endif	// traktor_resource_Proxy_H
