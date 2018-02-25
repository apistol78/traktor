/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ExtractSolutionTool_H
#define ExtractSolutionTool_H

#include "CustomTool.h"

namespace traktor
{
	namespace ui
	{
	
class Widget;

	}

	namespace sb
	{

class ExtractSolutionTool : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(ui::Widget* parent, Solution* solution);
};

	}
}

#endif	// ExtractSolutionTool_H
