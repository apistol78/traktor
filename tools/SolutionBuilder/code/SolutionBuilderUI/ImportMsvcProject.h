/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ImportMsvcProject_H
#define ImportMsvcProject_H

#include "CustomTool.h"

class ImportMsvcProject : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution, const std::wstring& solutionFileName);
};

#endif	// ImportMsvcProject_H
