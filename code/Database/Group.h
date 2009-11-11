#ifndef traktor_db_Group_H
#define traktor_db_Group_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace db
	{

class IProviderBus;
class IProviderGroup;

class Instance;

/*! \brief Database group.
 * \ingroup Database
 *
 * A group is just a tool to split
 * the database into logical portions.
 */
class T_DLLCLASS Group : public Object
{
	T_RTTI_CLASS(Group)

public:
	Group(IProviderBus* providerBus);

	bool internalCreate(IProviderGroup* providerGroup, Group* parent);

	void internalDestroy();

	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual Ref< Group > getGroup(const std::wstring& groupName);

	virtual Ref< Group > createGroup(const std::wstring& groupName);

	virtual Ref< Instance > getInstance(const std::wstring& instanceName, const Type* primaryType = 0);

	virtual Ref< Instance > createInstance(const std::wstring& instanceName, uint32_t flags = CifDefault, const Guid* guid = 0);

	virtual Ref< Group > getParent();

	virtual RefArray< Group >::iterator getBeginChildGroup();

	virtual RefArray< Group >::iterator getEndChildGroup();

	virtual RefArray< Instance >::iterator getBeginChildInstance();

	virtual RefArray< Instance >::iterator getEndChildInstance();

	void removeChildInstance(Instance* childInstance);

	void removeChildGroup(Group* childGroup);

private:
	Ref< IProviderBus > m_providerBus;
	Ref< IProviderGroup > m_providerGroup;
	Ref< Group > m_parent;
	std::wstring m_name;
	RefArray< Group > m_childGroups;
	RefArray< Instance > m_childInstances;
};

	}
}

#endif	// traktor_db_Group_H
