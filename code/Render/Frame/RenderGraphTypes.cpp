/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraphTypes.h"

namespace traktor::render
{

const RGTargetSet RGTargetSet::Invalid(~0U);

const RGTargetSet RGTargetSet::Output(0);

}
