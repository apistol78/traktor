#ifndef traktor_db_RemoteGroup_H
#define traktor_db_RemoteGroup_H

#include "Database/Provider/IProviderGroup.h"

namespace traktor
{
	namespace db
	{

class RemoteConnection;

/*! \brief Remote group.
 * \ingroup Database
 */
class RemoteGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	RemoteGroup(RemoteConnection* connection, uint32_t handle);

	virtual ~RemoteGroup();

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual bool rename(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual bool remove() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) T_OVERRIDE T_FINAL;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) T_OVERRIDE T_FINAL;

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteGroup_H
