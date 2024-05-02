/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Atomic.h"

namespace traktor
{

/*! Invoke method on pointer to object once then set pointer to null.
 * \ingroup Core
 */
template < typename Type, typename ... ArgumentTypes >
void invokeOnce(Type*& ref, void (Type::*M)(ArgumentTypes...), ArgumentTypes... args)
{
	Type* ptr = Atomic::exchange< Type* >(ref, nullptr);
	if (ptr)
		(ptr->*M)(std::forward< ArgumentTypes >(args) ...);
}

}
