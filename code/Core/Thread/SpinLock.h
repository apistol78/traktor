/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Blocking spin lock primitive.
 * \ingroup Core
 */
class T_DLLCLASS SpinLock : public IWaitable
{
public:
	SpinLock();

	virtual ~SpinLock();

	virtual bool wait(int32_t timeout = -1);

	void release();

private:
	int32_t m_lock;
};

}

