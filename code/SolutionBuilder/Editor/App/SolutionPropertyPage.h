/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

namespace traktor::ui
{

class ButtonClickEvent;
class Edit;
class GridColumnClickEvent;
class GridItemContentChangeEvent;
class GridView;
class IBitmap;

}

namespace traktor::sb
{

class Solution;

class SolutionPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Solution* solution);

private:
	Ref< ui::Edit > m_rootPath;
	Ref< ui::Edit > m_aggregateOutputPath;
	Ref< ui::GridView > m_gridVariables;
	Ref< ui::GridView > m_environmentPreview;
	Ref< ui::IBitmap > m_imageUnchecked;
	Ref< ui::IBitmap > m_imageChecked;
	Ref< Solution > m_solution;

	void updateVariables();

	void updateReferencedEnvironment();

	void eventEditFocus(ui::FocusEvent* event);

	void eventVariableChange(ui::GridItemContentChangeEvent* event);

	void eventVariableClick(ui::GridColumnClickEvent* event);

	void eventClickAddVariable(ui::ButtonClickEvent* event);

	void eventClickRemoveVariable(ui::ButtonClickEvent* event);
};

}
