/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef EditConfigurations_H
#define EditConfigurations_H

#include "CustomTool.h"

namespace traktor
{
	namespace ui
	{
	
class Widget;

	}

	namespace sb
	{

class Solution;

class EditConfigurations : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(ui::Widget* parent, Solution* solution);
};

	}
}

#endif	// EditConfigurations_H
