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
#include "Core/Object.h"
#include "Core/Class/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Any;
class OutputStream;

/*! Runtime dispatch interface.
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeDispatch : public Object
{
	T_RTTI_CLASS;

public:
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const = 0;
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const = 0;
};

}

