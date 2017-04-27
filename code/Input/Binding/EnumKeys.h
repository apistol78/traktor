/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_EnumKeys_H
#define traktor_input_EnumKeys_H

#include "Core/Serialization/MemberEnum.h"
#include "Input/InputTypes.h"

namespace traktor
{
	namespace input
	{

extern const MemberEnum< InputCategory >::Key g_InputCategory_Keys[];
extern const MemberEnum< InputDefaultControlType >::Key g_InputDefaultControlType_Keys[];

	}
}

#endif	// traktor_input_EnumKeys_H
