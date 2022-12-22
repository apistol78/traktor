/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Animation/StateNode.h"
#include "Resource/IdProxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Animation;

/*! Animation state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeAnimation : public StateNode
{
	T_RTTI_CLASS;

public:
	StateNodeAnimation() = default;

	explicit StateNodeAnimation(const std::wstring& name, const resource::IdProxy< Animation >& animation, bool linearInterpolation);

	virtual bool bind(resource::IResourceManager* resourceManager) override final;

	virtual bool prepareContext(StateContext& outContext) const override final;

	virtual void evaluate(StateContext& context, Pose& outPose) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::IdProxy< Animation >& getAnimation() const { return m_animation; }

	bool getLinearInterpolation() const { return m_linearInterpolation; }

private:
	resource::IdProxy< Animation > m_animation;
	bool m_linearInterpolation = false;
};

}
