/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Guid.h"
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

/*! Mutually exclusive primitive.
 * \ingroup Core
 */
class T_DLLCLASS Mutex : public IWaitable
{
public:
	Mutex();

	Mutex(const Guid& id);

	virtual ~Mutex();

	virtual bool wait(int32_t timeout = -1);

	void release();

	bool existing() const;

private:
	void* m_handle;
	bool m_existing;
};

}

