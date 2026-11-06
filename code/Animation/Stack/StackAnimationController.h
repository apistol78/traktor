/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseController.h"
#include "Core/Ref.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStack;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS StackAnimationController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit StackAnimationController(RtStack* stack);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms) override final;

	virtual Vector4 consumeRootMotion() override final;

	virtual void getPoseControllersOf(const TypeInfo& type, bool activeOnly, RefArray< IPoseController >& outControllers) override final;

	void setActive(render::Handle name, bool active);

	bool isActive(render::Handle name) const;

	void activateOnly(render::Handle name);

private:
	Ref< RtStack > m_stack;
};

}
