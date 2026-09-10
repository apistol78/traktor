/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Unit.h"

namespace traktor::ui
{

/*! Corner radius of controls; buttons, fields and drop downs.
 * \ingroup UI
 */
const Unit c_controlRadius = 4_ut;

/*! Corner radius of surfaces; tabs and the page they sit on.
 *
 * Tab and TabPage must agree on this, since the tab strip is indented by
 * the radius so the first tab clears the page's rounded corner.
 *
 * \ingroup UI
 */
const Unit c_surfaceRadius = 5_ut;

/*! Height added to the font size to form a row of a list.
 *
 * PropertyList, GridView and TreeView share this so their rows line up.
 *
 * \ingroup UI
 */
const Unit c_listRowPadding = 14_ut;

/*! Height of the column header of a list.
 * \ingroup UI
 */
const Unit c_listHeaderHeight = 28_ut;

/*! */
const Unit c_propertyButtonMargin = 1_ut;

}
