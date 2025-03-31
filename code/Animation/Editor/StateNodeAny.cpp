/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateNodeAny.h"

#include "Animation/Animation/Animation.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeAny", 0, StateNodeAny, StateNode)

StateNodeAny::StateNodeAny(const std::wstring& name)
	: StateNode(name)
{
}

}
