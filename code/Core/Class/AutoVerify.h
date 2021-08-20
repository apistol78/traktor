#pragma once

#if defined(__IOS__)
#	define T_VERIFY_ENABLE 0
#elif defined(__ANDROID__)
#	define T_VERIFY_ENABLE 0
#elif defined(__PS3__)
#	define T_VERIFY_ENABLE 0
#else
#	define T_VERIFY_ENABLE 1
#endif

#if T_VERIFY_ENABLE
#	if !defined(__ANDROID__) && !defined(__PS3__)
#		include "Core/Class/RuntimeException.h"
#		include "Core/Misc/String.h"
#		define T_VERIFY_ARGUMENT_COUNT(required) \
			if (argc < (required)) \
				throw traktor::RuntimeException(str(L"Not enought arguments; expect %d, got %d.", (required), argc));
#		define T_VERIFY_ARGUMENT_TYPE(arg, type) \
			if (!CastAny< type >::accept(argv[arg])) \
				throw traktor::RuntimeException(str(L"Incorrect type of argument %d; expect %s, got %ls.", (arg + 1), CastAny< type >::typeName().c_str(), argv[arg].format().c_str()));
#		define T_VERIFY_USING_EXCEPTIONS 1
#	else
#		define T_VERIFY_ARGUMENT_COUNT(required) \
			T_FATAL_ASSERT_M (argc >= (required), L"Not enough arguments");
#		define T_VERIFY_ARGUMENT_TYPE(arg, type) \
			T_FATAL_ASSERT_M (CastAny< type >::accept(argv[arg]), L"Incorrect argument type");
#		define T_VERIFY_USING_EXCEPTIONS 0
#	endif
#	define T_VERIFY_CAST_SELF(ClassType, self) mandatory_non_null_type_cast< ClassType* >(self)
#else
#	define T_VERIFY_ARGUMENT_COUNT(required)
#	define T_VERIFY_ARGUMENT_TYPE(arg, type)
#	define T_VERIFY_USING_EXCEPTIONS 0
#	define T_VERIFY_CAST_SELF(ClassType, self) static_cast< ClassType* >(self)
#endif
