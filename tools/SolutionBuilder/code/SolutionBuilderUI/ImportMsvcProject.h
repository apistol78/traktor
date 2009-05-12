#ifndef ImportMsvcProject_H
#define ImportMsvcProject_H

#include "CustomTool.h"

class ImportMsvcProject : public CustomTool
{
	T_RTTI_CLASS(ImportMsvcProject)

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution, const std::wstring& solutionFileName);
};

#endif	// ImportMsvcProject_H
