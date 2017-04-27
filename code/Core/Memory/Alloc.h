/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Alloc_H
#define traktor_Alloc_H

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

/*! \brief System memory allocators.
 * \ingroup Core
 */
class T_DLLCLASS Alloc
{
public:
	/*! \brief Allocate chunk of memory. */
	static void* acquire(size_t size, const char* tag);

	/*! \brief Free chunk of memory. */
	static void free(void* ptr);

	/*! \brief Allocate aligned chunk of memory. */
	static void* acquireAlign(size_t size, size_t align, const char* tag);

	/*! \brief Free aligned chunk of memory. */
	static void freeAlign(void* ptr);

	/*! \brief Return number of allocations. */
	static size_t count();
	
	/*! \brief Return amount of memory currently allocated. */
	static size_t allocated();
};

}

#endif	// traktor_Alloc_H
