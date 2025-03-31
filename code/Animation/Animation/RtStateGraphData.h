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
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::animation
{

class RtStateData;
class RtStateGraph;
class RtStateTransitionData;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateGraphData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< RtStateGraph > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StateGraphCompiler;

	RefArray< RtStateData > m_states;
	RefArray< RtStateTransitionData > m_transitions;
	Ref< RtStateData > m_root;
};

}
