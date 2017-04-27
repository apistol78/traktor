/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_IActionVMImage_H
#define traktor_flash_IActionVMImage_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace flash
	{

class ActionFrame;

/*! \brief ActionScript virtual machine image interface.
 * \ingroup Flash
 */
class IActionVMImage : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Execute image. */
	virtual void execute(ActionFrame* frame) const = 0;
};

	}
}

#endif	// traktor_flash_IActionVMImage_H
