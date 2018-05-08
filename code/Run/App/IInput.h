/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_run_IInput_H
#define traktor_run_IInput_H

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

/*! \brief Scriptable input reader interface.
 * \ingroup Run
 */
class IInput : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool endOfFile() = 0;

	virtual std::wstring readChar() = 0;

	virtual std::wstring readLn() = 0;
};

	}
}

#endif	// traktor_run_IInput_H
