/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual ~ISingleton() {}

	/*! \brief Destroy singleton.
	 *
	 * Called from the SingletonManager when
	 * this singleton should be destroyed.
	 */
	virtual void destroy() = 0;
};

}

#endif	// traktor_ISingleton_H
