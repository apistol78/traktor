#ifndef AddMultipleConfigurations_H
#define AddMultipleConfigurations_H

#include "CustomTool.h"

class AddMultipleConfigurations : public CustomTool
{
	T_RTTI_CLASS(AddMultipleConfigurations)

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution);
};

#endif	// AddMultipleConfigurations_H
