/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class InstanceDebugInfo;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS FrameDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	FrameDebugInfo() = default;

	explicit FrameDebugInfo(
		const Aabb2& frameBounds,
		const Vector4& stageTransform,
		int32_t viewWidth,
		int32_t viewHeight,
		const RefArray< InstanceDebugInfo >& instances
	);

	const Aabb2& getFrameBounds() const { return m_frameBounds; }

	const Vector4& getStageTransform() const { return m_stageTransform; }

	int32_t getViewWidth() const { return m_viewWidth; }

	int32_t getViewHeight() const { return m_viewHeight; }

	const RefArray< InstanceDebugInfo >& getInstances() const { return m_instances; }

	virtual void serialize(ISerializer& s) override final;

private:
	Aabb2 m_frameBounds;
	Vector4 m_stageTransform;
	int32_t m_viewWidth = 0;
	int32_t m_viewHeight = 0;
	RefArray< InstanceDebugInfo > m_instances;
};

	}
}
