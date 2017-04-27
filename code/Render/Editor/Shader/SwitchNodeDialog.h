/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SwitchNodeDialog_H
#define traktor_render_SwitchNodeDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GridView;

		}
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
	Ref< ui::custom::GridView > m_caseList;
};

	}
}

#endif	// traktor_render_SwitchNodeDialog_H
