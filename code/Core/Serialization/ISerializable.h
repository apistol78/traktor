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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

/*! Base class of each serializable class.
 * \ingroup Core
 *
 * Any class supporting serialization must be
 * derived from this class.
 */
class T_DLLCLASS ISerializable : public Object
{
	T_RTTI_CLASS;

public:
	/*! Serialize object.
	 *
	 * \param s Serializer interface.
	 */
	virtual void serialize(ISerializer& s) = 0;
};

}
