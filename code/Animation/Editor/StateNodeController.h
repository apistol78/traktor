/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Editor/StateNode.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class IPoseControllerData;

/*! Pose controller state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeController : public StateNode
{
	T_RTTI_CLASS;

public:
	StateNodeController() = default;

	explicit StateNodeController(const std::wstring& name, const IPoseControllerData* poseController);

	virtual void serialize(ISerializer& s) override;

	const IPoseControllerData* getPoseController() const { return m_poseController; }

private:
	Ref< const IPoseControllerData > m_poseController;
};

}
