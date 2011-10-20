#ifndef traktor_flash_ActionTypes_H
#define traktor_flash_ActionTypes_H

#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

// AVM number; original AVM use double.
#if !defined(T_AVM_USE_DOUBLE)
typedef float avm_number_t;
#else
typedef double avm_number_t;
#endif

// Verbose logging.
#define T_IF_VERBOSE(x) \
{ x }

	}
}

#endif	// traktor_flash_ActionTypes_H
