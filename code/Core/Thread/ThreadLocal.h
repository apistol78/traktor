/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor
{

/*! Thread local storage.
 * \ingroup Core
 *
 * Provide thread unique storage of a pointer
 * using TLS (Thread Local Storage).
 */
class T_DLLCLASS ThreadLocal : public Object
{
	T_RTTI_CLASS;

public:
	ThreadLocal();

	virtual ~ThreadLocal();

	void set(void* ptr);

	void* get() const;

private:
	uint64_t m_handle;
};

}

