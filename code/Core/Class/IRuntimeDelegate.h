/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IRuntimeDelegate_H
#define traktor_IRuntimeDelegate_H

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

class Any;

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeDelegate : public Object
{
	T_RTTI_CLASS;

public:
	virtual Any call(int32_t argc, const Any* argv) = 0;
};

}

#endif	// traktor_IRuntimeDelegate_H
