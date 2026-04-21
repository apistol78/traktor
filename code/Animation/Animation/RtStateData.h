/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Animation/Animation.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Member.h"

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

class IPoseControllerData;
class RtState;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< RtState > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StateGraphCompiler;

	resource::Id< Animation > m_animation;
	Ref< const IPoseControllerData > m_poseController;
};

}
