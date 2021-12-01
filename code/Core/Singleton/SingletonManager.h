#pragma once

#include "Core/Containers/StaticVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISingleton;

/*! Singleton manager.
 * \ingroup Core
 *
 * The singleton manager is responsible of maintaining a list
 * of singletons and which order they should be brought down
 * when the application terminates.
 */
class T_DLLCLASS SingletonManager
{
public:
	static SingletonManager& getInstance();

	/*! Add singleton. */
	void add(ISingleton* singleton);

	/*! Add singleton before a dependent singleton. */
	void addBefore(ISingleton* singleton, ISingleton* dependency);

	/*! Add singleton after a dependent singleton. */
	void addAfter(ISingleton* singleton, ISingleton* dependency);

	/*! Explicit destroy all singletons. */
	void destroy();

private:
	StaticVector< ISingleton*, 64 > m_singletons;

	virtual ~SingletonManager();
};

}

