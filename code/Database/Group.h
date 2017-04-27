/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_Group_H
#define traktor_db_Group_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Database/IInstanceEventListener.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace db
	{

class IGroupEventListener;
class IInstanceEventListener;
class IProviderGroup;
class Instance;

/*! \brief Database group.
 * \ingroup Database
 *
 * A group is just a tool to split
 * the database into logical portions.
 */
class T_DLLCLASS Group
:	public Object
,	public IInstanceEventListener
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual Ref< Group > getGroup(const std::wstring& groupName);

	virtual Ref< Group > createGroup(const std::wstring& groupName);

	virtual Ref< Instance > getInstance(const std::wstring& instanceName, const TypeInfo* primaryType = 0);

	virtual Ref< Instance > createInstance(const std::wstring& instanceName, uint32_t flags = CifDefault, const Guid* guid = 0);

	virtual Ref< Group > getParent();

	virtual bool getChildGroups(RefArray< Group >& outChildGroups);

	virtual bool getChildInstances(RefArray< Instance >& outChildInstances);

private:
	friend class Database;

	mutable Semaphore m_lock;
	IGroupEventListener* m_groupEventListener;
	IInstanceEventListener* m_instanceEventListener;
	Ref< IProviderGroup > m_providerGroup;
	Group* m_parent;
	std::wstring m_name;
	RefArray< Group > m_childGroups;
	RefArray< Instance > m_childInstances;

	Group(IGroupEventListener* groupEventListener, IInstanceEventListener* instanceEventListener);

	bool internalCreate(IProviderGroup* providerGroup, Group* parent);

	void internalDestroy();

	bool internalFlushChildInstances();

	bool internalAddExtGroup(const std::wstring& groupName);

	bool internalAddExtInstance(const Guid& instanceGuid);

	// \name IInstanceEventListener
	// \{

	virtual void instanceEventCreated(Instance* instance) T_OVERRIDE T_FINAL;

	virtual void instanceEventRemoved(Instance* instance) T_OVERRIDE T_FINAL;

	virtual void instanceEventGuidChanged(Instance* instance, const Guid& previousGuid) T_OVERRIDE T_FINAL;

	virtual void instanceEventRenamed(Instance* instance, const std::wstring& previousName) T_OVERRIDE T_FINAL;

	virtual void instanceEventCommitted(Instance* instance) T_OVERRIDE T_FINAL;

	// \}
};

	}
}

#endif	// traktor_db_Group_H
