#ifndef traktor_render_ShaderDependencyTracker_H
#define traktor_render_ShaderDependencyTracker_H

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
	bool m_scanThreadActive;
	Semaphore m_lock;
	std::vector< IListener* > m_listeners;
	std::set< Dependency > m_dependencies;
};

	}
}

#endif	// traktor_render_ShaderDependencyTracker_H
