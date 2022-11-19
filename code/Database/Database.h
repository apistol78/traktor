/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"
#include "Database/ConnectionString.h"
#include "Database/IGroupEventListener.h"
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

class ISerializable;

}

namespace traktor::db
{

class IEvent;
class IProviderBus;
class IProviderDatabase;
class Group;
class Instance;

/*! Database engine.
 * \ingroup Database
 *
 * The Database class manages a local view of
 * a database provider.
 */
class T_DLLCLASS Database
:	public Object
,	public IInstanceEventListener
,	public IGroupEventListener
{
	T_RTTI_CLASS;

public:
	/*! Open database from provider instance.
	 *
	 * \param providerDatabase Concrete provider database implementation.
	 * \return True if database opened successfully.
	 */
	virtual bool open(IProviderDatabase* providerDatabase);

	/*! Open database from connection string. */
	virtual bool open(const ConnectionString& connectionString);

	/*! Create database from connection string. */
	virtual bool create(const ConnectionString& connectionString);

	/*! Close database. */
	virtual void close();

	/*! Root group. */
	virtual Ref< Group > getRootGroup();

	/*! Get group by path.
	 *
	 * \param groupPath Path to group.
	 * \return Group.
	 */
	virtual Ref< Group > getGroup(const std::wstring& groupPath);

	/*! Create new group.
	 *
	 * \param groupPath Path to new group.
	 * \return New group.
	 */
	virtual Ref< Group > createGroup(const std::wstring& groupPath);

	/*! Get instance by guid.
	 *
	 * \param instanceGuid Instance guid.
	 * \return Instance; null if no instance with given guid exist.
	 */
	virtual Ref< Instance > getInstance(const Guid& instanceGuid);

	/*! Get instance by path.
	 *
	 * \param instancePath Path to instance.
	 * \param primaryType Optional primary type; if instance isn't of type then null is returned.
	 * \return Instance.
	 */
	virtual Ref< Instance > getInstance(const std::wstring& instancePath, const TypeInfo* primaryType = nullptr);

	/*! Create new instance.
	 *
	 * \param instancePath Path to new instance.
	 * \param flags Create flags.
	 * \param guid Instance guid; null then guid is automatically created.
	 * \return New instance.
	 */
	virtual Ref< Instance > createInstance(const std::wstring& instancePath, uint32_t flags = CifDefault, const Guid* guid = nullptr);

	/*! Get instance object by guid.
	 *
	 * \param guid Instance guid.
	 * \return Instance's object; null if no instance found.
	 */
	virtual Ref< ISerializable > getObjectReadOnly(const Guid& guid) const;

	/*! Get instance object by guid.
	 *
	 * \param guid Instance guid.
	 * \return Instance's object; null if no instance found.
	 */
	template < typename T >
	Ref< T > getObjectReadOnly(const Guid& guid) const
	{
		Ref< ISerializable > object = getObjectReadOnly(guid);
		return dynamic_type_cast< T* >(object);
	}

	/*! Get event from bus.
	 *
	 * \note
	 * This method may flush database tree and should not be called
	 * from another thread if any instance or group is temporarily accessed.
	 *
	 * \param outEvent Event object.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(Ref< const IEvent >& outEvent, bool& outRemote);

private:
	Ref< IProviderDatabase > m_providerDatabase;
	Ref< IProviderBus > m_providerBus;
	Ref< Group > m_rootGroup;
	mutable Semaphore m_lock;
	SmallMap< Guid, Ref< Instance > > m_instanceMap;
	uint64_t m_lastEntrySqnr = 0;

	// \name IInstanceEventListener
	// \{

	virtual void instanceEventCreated(Instance* instance) override final;

	virtual void instanceEventRemoved(Instance* instance) override final;

	virtual void instanceEventGuidChanged(Instance* instance, const Guid& previousGuid) override final;

	virtual void instanceEventRenamed(Instance* instance, const std::wstring& previousName) override final;

	virtual void instanceEventCommitted(Instance* instance) override final;

	// \}

	// \name IGroupEventListener
	// \{

	virtual void groupEventRenamed(Group* group, const std::wstring& previousName) override final;

	// \}
};

}
