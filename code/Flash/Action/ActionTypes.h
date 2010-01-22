#ifndef traktor_flash_ActionTypes_H
#define traktor_flash_ActionTypes_H

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

	}
}

#endif	// traktor_flash_ActionTypes_H
