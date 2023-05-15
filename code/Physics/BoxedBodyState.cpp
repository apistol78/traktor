/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Physics/BoxedBodyState.h"

namespace traktor::physics
{
	namespace
	{
	
BoxedAllocator< BoxedBodyState, 64 > s_allocBoxedBodyState;
	
	}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyState", BoxedBodyState, Boxed)

BoxedBodyState::BoxedBodyState(const BodyState& value)
:	m_value(value)
{
}

std::wstring BoxedBodyState::toString() const
{
	return L"(body state)";
}

void* BoxedBodyState::operator new (size_t size)
{
	return s_allocBoxedBodyState.alloc();
}

void BoxedBodyState::operator delete (void* ptr)
{
	s_allocBoxedBodyState.free(ptr);
}

}
