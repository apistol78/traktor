/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	namespace flash
	{

class IActionVMImage;
class SwfReader;

/*! \brief ActionScript virtual machine interface.
 * \ingroup Flash
 */
class T_DLLCLASS IActionVM : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Load executable image from binary representation of bytecode. */
	virtual Ref< const IActionVMImage > load(SwfReader& swf) const = 0;
};

	}
}

#endif	// traktor_flash_IActionVM_H
