#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class GridView;

	}

	namespace render
	{

class Switch;

class SwitchNodeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, Switch* switchNode);

private:
	Ref< ui::GridView > m_caseList;
};

	}
}

