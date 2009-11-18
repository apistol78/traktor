#ifndef traktor_db_CompactGroup_H
#define traktor_db_CompactGroup_H

#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class CompactContext;
class CompactGroupEntry;
class CompactInstance;

/*! \brief Compact group
 * \ingroup Database
 */
class CompactGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	CompactGroup(CompactContext* context);

	bool internalCreate(CompactGroupEntry* groupEntry);

	virtual std::wstring getName() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName);

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid);

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups);

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances);

private:
	Ref< CompactContext > m_context;
	Ref< CompactGroupEntry > m_groupEntry;
	RefArray< CompactGroup > m_childGroups;
	RefArray< CompactInstance > m_childInstances;
};

	}
}

#endif	// traktor_db_CompactGroup_H
