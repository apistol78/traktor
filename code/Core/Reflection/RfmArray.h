/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_RfmArray_H
#define traktor_RfmArray_H

#include "Core/RefArray.h"
#include "Core/Reflection/RfmCompound.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Reflected array member.
 * \ingroup Core
 */
class T_DLLCLASS RfmArray : public RfmCompound
{
	T_RTTI_CLASS;

public:
	RfmArray(const wchar_t* name);
};

}

#endif	// traktor_RfmArray_H
