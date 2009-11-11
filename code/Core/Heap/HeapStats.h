#ifndef traktor_HeapStats_H
#define traktor_HeapStats_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Heap monitoring statistics.
 * \ingroup Core
 */
struct HeapStats
{
	int32_t objects;			//!< Number of objects.
	int32_t references;			//!< Number of references.
	int32_t rootReferences;		//!< Number of root references.
	int32_t collects;			//!< Number of collections issued.
};

}

#endif	// traktor_HeapStats_H
