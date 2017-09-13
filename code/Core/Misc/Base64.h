/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Base64_H
#define traktor_Base64_H

#include <string>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Base64 encode binary data.
 * \ingroup Core
 */
class T_DLLCLASS Base64 : public Object
{
	T_RTTI_CLASS;

public:
	std::wstring encode(const uint8_t* data, uint32_t size, bool insertCrLf = false) const;

	std::wstring encode(const AlignedVector< uint8_t >& data, bool insertCrLf = false) const;

	AlignedVector< uint8_t > decode(const std::wstring& b64) const;
};

}

#endif	// traktor_Base64_H
