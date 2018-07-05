/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IRuntimeDispatch_H
#define traktor_IRuntimeDispatch_H

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
class OutputStream;

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeDispatch : public Object
{
	T_RTTI_CLASS;

public:
	virtual void signature(OutputStream& ss) const = 0;

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const = 0;
};

}

#endif	// traktor_IRuntimeDispatch_H
