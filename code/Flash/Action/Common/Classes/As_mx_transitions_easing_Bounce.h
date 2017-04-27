/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_mx_transitions_easing_Bounce_H
#define traktor_flash_As_mx_transitions_easing_Bounce_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Bounce : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Bounce(ActionContext* context);

private:
	void Bounce_easeIn(CallArgs& ca);

	void Bounce_easeInOut(CallArgs& ca);

	void Bounce_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Bounce_H
