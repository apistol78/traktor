/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Debugger_H
#define traktor_Debugger_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Platform specific debugger path.
 * \ingroup Core
 *
 * Used by the framework in order to invoke platform specific
 * actions when, for example, there is an assertion.
 */
class T_DLLCLASS Debugger
{
public:
	static Debugger& getInstance();

	/*! \brief Show assert failed dialog.
	 *
	 * \param expression Failed expression.
	 * \param file Source file where assert triggered.
	 * \param line Line in source file.
	 * \param message Custom message.
	 */
	void assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message = L"");

	/*! \brief Check if debugger is attached. */
	bool isDebuggerAttached() const;

	/*! \brief Break application into debugger. */
	void breakDebugger();

	/*! \brief Report profiling event to debugger. */
	void reportEvent(const wchar_t* const text, ...);
};

}

#endif	// traktor_Debugger_H
