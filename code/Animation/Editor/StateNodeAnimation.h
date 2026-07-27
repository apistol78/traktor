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
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeAnimation : public StateNode
{
	T_RTTI_CLASS;

public:
	StateNodeAnimation() = default;

	explicit StateNodeAnimation(const std::wstring& name, const resource::Id< Animation >& animation);

	virtual void serialize(ISerializer& s) override;

	const resource::Id< Animation >& getAnimation() const { return m_animation; }

	const ITransformTimeData* getTransformTime() const { return m_transformTime; }

private:
	resource::Id< Animation > m_animation;
	Ref< const ITransformTimeData > m_transformTime;
};

}
