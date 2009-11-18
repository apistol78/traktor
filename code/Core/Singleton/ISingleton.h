#ifndef traktor_ISingleton_H
#define traktor_ISingleton_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Singleton base class.
 * \ingroup Core
 *
 * Singletons should be derived from
 * this class as the SingletonManager will
 * then be able to control order of destruction
 * of all singletons.
 */
class ISingleton
{
protected:
	friend class SingletonManager;

	/*! \brief Destroy singleton.
	 *
	 * Called from the SingletonManager when
	 * this singleton should be destroyed.
	 */
	virtual void destroy() = 0;
};

}

#endif	// traktor_ISingleton_H
