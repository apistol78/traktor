#ifndef traktor_flash_IActionVM_H
#define traktor_flash_IActionVM_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BitReader;

	namespace flash
	{

class IActionVMImage;

/*! \brief ActionScript virtual machine interface.
 * \ingroup Flash
 */
class T_DLLCLASS IActionVM : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Load executable image from binary representation of bytecode. */
	virtual Ref< const IActionVMImage > load(BitReader& br) const = 0;
};

	}
}

#endif	// traktor_flash_IActionVM_H
