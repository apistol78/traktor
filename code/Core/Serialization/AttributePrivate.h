/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_AttributePrivate_H
#define traktor_AttributePrivate_H

#include "Core/Serialization/Attribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Member private attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributePrivate : public Attribute
{
	T_RTTI_CLASS;
};

}

#endif	// traktor_AttributePrivate_H
