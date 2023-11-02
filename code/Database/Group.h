/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
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

}

namespace traktor::db
{

class IGroupEventListener;
class IInstanceEventListener;
class IProviderGroup;
class Instance;

/*! Database group.
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

	virtual uint32_t getFlags() const;

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

	void internalRemoveGroup(Group* childGroup);

	// \name IInstanceEventListener
	// \{

	virtual void instanceEventCreated(Instance* instance) override final;

	virtual void instanceEventRemoved(Instance* instance) override final;

	virtual void instanceEventGuidChanged(Instance* instance, const Guid& previousGuid) override final;

	virtual void instanceEventRenamed(Instance* instance, const std::wstring& previousName) override final;

	virtual void instanceEventCommitted(Instance* instance) override final;

	// \}
};

}
