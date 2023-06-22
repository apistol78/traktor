/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! Clone object.
 * \ingroup Core
 *
 * Creates a clone of an object through
 * serialization.
 */
class T_DLLCLASS DeepClone : public Object
{
	T_RTTI_CLASS;

public:
	explicit DeepClone(const ISerializable* source);

	/*! Create new instance of source object. */
	Ref< ISerializable > create() const;

	/*! Create new instance of source object. */
	template < typename T >
	Ref< T > create() const
	{
		return dynamic_type_cast< T* >(create());
	}

private:
	AlignedVector< uint8_t > m_copy;
};

}

