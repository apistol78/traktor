#ifndef traktor_db_LocalGroup_H
#define traktor_db_LocalGroup_H

#include "Database/Provider.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Context;

/*! \brief Local group.
 * \ingroup Database
 */
class T_DLLCLASS LocalGroup : public IProviderGroup
{
	T_RTTI_CLASS(LocalGroup)

public:
	LocalGroup(Context* contex, const Path& groupPath);

	virtual std::wstring getName() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual IProviderGroup* createGroup(const std::wstring& groupName);

	virtual IProviderInstance* createInstance(const std::wstring& instanceName, const Guid& instanceGuid);

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups);

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances);

private:
	Ref< Context > m_context;
	Path m_groupPath;
};

	}
}

#endif	// traktor_db_LocalGroup_H
