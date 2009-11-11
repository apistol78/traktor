#ifndef traktor_db_RemoteGroup_H
#define traktor_db_RemoteGroup_H

#include "Core/Heap/Ref.h"
#include "Database/Provider/IProviderGroup.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Remote group.
 * \ingroup Database
 */
class RemoteGroup : public IProviderGroup
{
	T_RTTI_CLASS(RemoteGroup)

public:
	RemoteGroup(Connection* connection, uint32_t handle);

	virtual ~RemoteGroup();

	virtual std::wstring getName() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName);

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid);

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups);

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances);

private:
	Ref< Connection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteGroup_H
