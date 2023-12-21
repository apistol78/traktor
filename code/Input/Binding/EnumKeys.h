/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/MemberEnum.h"
#include "Input/InputTypes.h"

namespace traktor::input
{

extern const MemberEnum< InputCategory >::Key g_InputCategory_Keys_Old[];
extern const MemberEnum< InputCategory >::Key g_InputCategory_Keys[];

extern const MemberEnum< DefaultControl >::Key g_DefaultControl_Keys_Old[];
extern const MemberEnum< DefaultControl >::Key g_DefaultControl_Keys[];

}
