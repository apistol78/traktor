/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IHash_H
#define traktor_IHash_H

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

/*! \brief Hash function interface.
 * \ingroup Core
 */
class T_DLLCLASS IHash : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Begin feeding data for hash calculation. */
	virtual void begin() = 0;

	/*! \brief Feed data to hash calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feed(const void* buffer, uint64_t bufferSize) = 0;

	/*! \brief End feeding data for hash calculation. */
	virtual void end() = 0;
};

}

#endif	// traktor_IHash_H
