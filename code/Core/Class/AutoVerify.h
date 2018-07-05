/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#define T_VERIFY_ENABLE 1

#if T_VERIFY_ENABLE
#	if !defined(__ANDROID__) && !defined(__PS3__) && !defined(__PS4__)
#		include "Core/Class/RuntimeException.h"
#		include "Core/Io/StringOutputStream.h"

#		define T_VERIFY_ARGUMENT_COUNT(required) \
			if (argc < (required)) \
			{ \
				StringOutputStream ss; \
				ss << L"Not enought arguments; expect " << (required) << L", got " << argc; \
				throw traktor::RuntimeException(ss.str()); \
			}

#		define T_VERIFY_ARGUMENT_TYPE(arg, type) \
			if (!CastAny< type >::accept(argv[arg])) \
			{ \
				StringOutputStream ss; \
				ss << L"Incorrect type of argument " << (arg + 1) << L"; expect "; CastAny< type >::typeName(ss); ss << L", got " << argv[arg].format(); \
				throw traktor::RuntimeException(ss.str()); \
			}

#		define T_VERIFY_USING_EXCEPTIONS 1
#	else
#		define T_VERIFY_ARGUMENT_COUNT(required) \
			T_FATAL_ASSERT_M (argc >= (required), L"Not enough arguments");
#		define T_VERIFY_ARGUMENT_TYPE(arg, type) \
			T_FATAL_ASSERT_M (CastAny< type >::accept(argv[arg]), L"Incorrect argument type");
#		define T_VERIFY_USING_EXCEPTIONS 0
#	endif
#else
#	define T_VERIFY_ARGUMENT_COUNT(required) 
#	define T_VERIFY_ARGUMENT_TYPE(arg, type)
#	define T_VERIFY_USING_EXCEPTIONS 0
#endif
