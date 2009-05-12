#ifndef traktor_SingletonManager_H
#define traktor_SingletonManager_H

#include <vector>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Singleton;

/*! \brief Singleton manager.
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

	/*! \brief Add singleton. */
	void add(Singleton* singleton);

	/*! \brief Add singleton before a dependent singleton. */
	void addBefore(Singleton* singleton, Singleton* dependency);

	/*! \brief Add singleton after a dependent singleton. */
	void addAfter(Singleton* singleton, Singleton* dependency);

private:
	std::vector< Singleton* > m_singletons;

	virtual ~SingletonManager();
};

}

#endif	// traktor_SingletonManager_H
