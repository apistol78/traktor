/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(__IOS__)
#	define T_VERIFY_ENABLE 0
#	define T_VERIFY_USING_EXCEPTIONS 0
#elif defined(__ANDROID__)
#	define T_VERIFY_ENABLE 0
#	define T_VERIFY_USING_EXCEPTIONS 0
#else
#	define T_VERIFY_ENABLE 1
#	define T_VERIFY_USING_EXCEPTIONS 1
#endif

#if T_VERIFY_ENABLE

#	if T_VERIFY_USING_EXCEPTIONS
#		include "Core/Class/RuntimeException.h"
#		include "Core/Misc/String.h"

#		define T_VERIFY_ARGUMENT_COUNT(required) \
			if (argc < (required)) \
				throw traktor::RuntimeException(str(L"Not enought arguments; expect %d, got %d.", (required), argc));
#	else
#		define T_VERIFY_ARGUMENT_COUNT(required) \
			T_FATAL_ASSERT_M (argc >= (required), L"Not enough arguments.");
#	endif

#include "Core/Class/CastAny.h"

namespace traktor
{
	template < typename ... ArgumentTypes >
	struct Verify
	{
		template < typename ArgumentType >
		static void __check__(int32_t index, const class Any& arg)
		{
#	if T_VERIFY_USING_EXCEPTIONS
			if (!CastAny< ArgumentType >::accept(arg))
				throw traktor::RuntimeException(str(
					L"Incorrect type of argument %d; expect %s, got %ls.",
					index + 1,
					CastAny< ArgumentType >::typeName().c_str(),
					arg.format().c_str()
				));
#	else
			T_FATAL_ASSERT_M (CastAny< ArgumentType >::accept(arg), L"Incorrect argument type.");
#	endif
		}

		template < std::size_t ... Is >
		static void check(const Any* argv, std::index_sequence< Is ... >)
		{
			int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = {
				( __check__< ArgumentTypes >(Is, argv[Is]), 0 ) ...
			};
            (void)__dummy__;
		}
	};
}

#	define T_VERIFY_ARGUMENT_TYPES \
	Verify< ArgumentTypes ... >::check(argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());

#	define T_VERIFY_CAST_SELF(ClassType, self) mandatory_non_null_type_cast< ClassType* >(self)

#else

#	define T_VERIFY_ARGUMENT_COUNT(required)
#	define T_VERIFY_ARGUMENT_TYPES
#	define T_VERIFY_CAST_SELF(ClassType, self) static_cast< ClassType* >(self)

#endif
