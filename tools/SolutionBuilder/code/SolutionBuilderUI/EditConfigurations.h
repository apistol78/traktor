#ifndef EditConfigurations_H
#define EditConfigurations_H

#include "CustomTool.h"

class EditConfigurations : public CustomTool
{
	T_RTTI_CLASS(EditConfigurations)

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution);
};

#endif	// EditConfigurations_H
