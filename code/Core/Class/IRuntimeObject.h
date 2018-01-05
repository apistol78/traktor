/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IRuntimeObject_H
#define traktor_IRuntimeObject_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeObject : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< const IRuntimeClass > getRuntimeClass() const = 0;
};

}

#endif	// traktor_IRuntimeObject_H
