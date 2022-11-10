/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Singleton base class.
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

	/*! Destroy singleton.
	 *
	 * Called from the SingletonManager when
	 * this singleton should be destroyed.
	 */
	virtual void destroy() = 0;
};

}

