/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once
#ifndef traktor_Assert_H
#define traktor_Assert_H

#include "Core/Debug/Debugger.h"

/*! \ingroup Core */
//@{

#if defined(_DEBUG)
#	define T_ASSERT(expression) \
		(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__), 0));

#	define T_ASSERT_M(expression, message) \
		(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__, message), 0));

#	define T_BREAKPOINT \
		traktor::Debugger::getInstance().breakDebugger();
#else
#	define T_ASSERT(expression)
#	define T_ASSERT_M(expression, message)
#	define T_BREAKPOINT
#endif

#define T_FATAL_ASSERT(expression) \
	(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__), 0));

#define T_FATAL_ASSERT_M(expression, message) \
	(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__, message), 0));

#define T_FATAL_ERROR \
	traktor::Debugger::getInstance().breakDebugger();

//@}

#endif	// traktor_Assert_H
