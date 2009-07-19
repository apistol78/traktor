#ifndef traktor_Alloc_H
#define traktor_Alloc_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief System memory allocators.
 * \ingroup Core
 */
class T_DLLCLASS Alloc
{
public:
	static void* acquire(size_t size);

	static void free(void* ptr);

	static void* acquireAlign(size_t size, size_t align);

	static void freeAlign(void* ptr);
};

}

#endif	// traktor_Alloc_H
