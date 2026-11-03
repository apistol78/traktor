/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStackData;

/*! Stack animation pose controller data.
 * \ingroup Animation
 */
class T_DLLCLASS StackAnimationControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	StackAnimationControllerData() = default;

	explicit StackAnimationControllerData(const resource::Id< RtStackData >& stack);

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		bool editor) const override final;

	virtual void serialize(ISerializer& s) override final;

	inline const resource::Id< RtStackData >& getStack() const { return m_stack; }

private:
	resource::Id< RtStackData > m_stack;
};

}
