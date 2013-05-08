#ifndef traktor_flash_ActionVMImage2_H
#define traktor_flash_ActionVMImage2_H

#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

class ActionVMImage2 : public IActionVMImage
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class ActionVM2;
};

	}
}

#endif	// traktor_flash_ActionVMImage2_H
