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

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual bool rename(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual bool remove() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) T_OVERRIDE T_FINAL;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups) T_OVERRIDE T_FINAL;

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances) T_OVERRIDE T_FINAL;

private:
	Ref< CompactContext > m_context;
	Ref< CompactGroupEntry > m_groupEntry;
	RefArray< CompactGroup > m_childGroups;
	RefArray< CompactInstance > m_childInstances;
};

	}
}

#endif	// traktor_db_CompactGroup_H
