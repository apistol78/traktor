/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStateData;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateTransitionData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class StateGraphCompiler;

	Ref< RtStateData > m_from;
	Ref< RtStateData > m_to;
};

}
