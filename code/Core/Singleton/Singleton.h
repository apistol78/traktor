#ifndef traktor_Singleton_H
#define traktor_Singleton_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS Singleton : public Object
{
	T_RTTI_CLASS(Singleton)

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

#endif	// traktor_Singleton_H
