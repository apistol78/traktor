/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Editor/StateNode.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Animation;
class ITransformTimeData;

/*! Animation state node.
 *
 * A state can reference multiple animations; the compiler expands the
 * state into one runtime state per animation, so the graph randomly
 * select which animation to play each time the state is entered.
 *
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeAnimation : public StateNode
{
	T_RTTI_CLASS;

public:
	StateNodeAnimation() = default;

	explicit StateNodeAnimation(const std::wstring& name, const resource::Id< Animation >& animation);

	explicit StateNodeAnimation(const std::wstring& name, const AlignedVector< resource::Id< Animation > >& animations);

	virtual void serialize(ISerializer& s) override;

	const AlignedVector< resource::Id< Animation > >& getAnimations() const { return m_animations; }

	float getLoopDuration() const { return m_loopDuration; }

	const ITransformTimeData* getTransformTime() const { return m_transformTime; }

private:
	AlignedVector< resource::Id< Animation > > m_animations;
	float m_loopDuration = 0.0f;
	Ref< const ITransformTimeData > m_transformTime;
};

}
