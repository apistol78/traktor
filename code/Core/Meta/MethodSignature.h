/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor
{

template<
	bool Const,
	typename ClassType,
	typename ReturnType,
	typename... ArgumentTypes
>
struct MethodSignature
{
	typedef ReturnType (ClassType::*method_t)(ArgumentTypes...);
	typedef ReturnType (*static_method_t)(ArgumentTypes...);
};

template<
	typename ClassType,
	typename ReturnType,
	typename... ArgumentTypes
>
struct MethodSignature< true, ClassType, ReturnType, ArgumentTypes... >
{
	typedef ReturnType (ClassType::*method_t)(ArgumentTypes...) const;
};

}
