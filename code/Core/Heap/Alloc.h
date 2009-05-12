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

/*! \ingroup Core */
//@{

T_DLLCLASS void* alloc(size_t size);

T_DLLCLASS void free(void* ptr);

T_DLLCLASS void* allocAlign(size_t size, size_t align);

T_DLLCLASS void freeAlign(void* ptr);

//!\}

}

#endif	// traktor_Alloc_H
