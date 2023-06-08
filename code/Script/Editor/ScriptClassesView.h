/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

namespace traktor::ui
{

class TreeView;

}

namespace traktor::script
{

/*! Runtime classes view.
 * \ingroup Script
 */
class ScriptClassesView : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual void destroy() override final;

private:
	Ref< ui::TreeView > m_treeClasses;
};

}
