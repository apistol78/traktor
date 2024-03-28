/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/Ds8/AudioDriverDs8.h"

namespace traktor::sound
{

extern "C" void __module__Traktor_Sound_Ds8()
{
	T_FORCE_LINK_REF(AudioDriverDs8);
}

}

#endif
