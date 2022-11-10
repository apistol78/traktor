/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class Thread;

	namespace db
	{

class Database;

	}

	namespace render
	{

class ShaderDependencyTracker : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual void dependencyAdded(const Guid& fromShader, const Guid& toShader) = 0;

		virtual void dependencyRemoved(const Guid& fromShader, const Guid& toShader) = 0;
	};

	ShaderDependencyTracker();

	virtual ~ShaderDependencyTracker();

	void destroy();

	void scan(db::Database* database);

	void scan(db::Database* database, const Guid& shader);

	void addListener(IListener* listener);

	void removeListener(IListener* listener);

	void addDependency(const Guid& fromShader, const Guid& toShader);

	void removeDependency(const Guid& fromShader, const Guid& toShader);

	void removeDependencies(const Guid& fromShader);

private:
	struct Dependency
	{
		Guid from;
		Guid to;

		bool operator < (const Dependency& rh) const;
	};

	Ref< db::Database > m_database;
	Thread* m_scanThread;
	Semaphore m_lock;
	std::vector< IListener* > m_listeners;
	std::set< Dependency > m_dependencies;
};

	}
}

