/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseController.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStateGraph;
class ITransformTime;

/*! Animation pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationGraphPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit AnimationGraphPoseController(const resource::Proxy< RtStateGraph >& stateGraph, ITransformTime* transformTime);

	bool setState(const std::wstring& stateName);

	bool setParameterValue(const render::Handle& parameter, bool value);

	void setTime(float time);

	float getTime() const;

	void setTimeFactor(float timeFactor);

	float getTimeFactor() const;

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms) override final;

private:
	resource::Proxy< RtStateGraph > m_stateGraph;
};

}
