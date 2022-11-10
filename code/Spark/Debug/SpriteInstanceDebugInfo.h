/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <utility>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Spark/Debug/InstanceDebugInfo.h"

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

class SpriteInstance;

/*!
 * \ingroup Spark
 */
class T_DLLCLASS SpriteInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	SpriteInstanceDebugInfo() = default;

	explicit SpriteInstanceDebugInfo(const SpriteInstance* instance, const std::string& className, bool mask, bool clipped, const RefArray< InstanceDebugInfo >& childrenDebugInfo);

	const std::string& getClassName() const { return m_className; }

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	uint16_t getFrames() const { return m_frames; }

	uint16_t getCurrentFrame() const { return m_currentFrame; }

	bool isPlaying() const { return m_playing; }

	const Aabb2& getScalingGrid() const { return m_scalingGrid; }

	const AlignedVector< std::pair< uint32_t, std::string > >& getLabels() const { return m_labels; }

	const RefArray< InstanceDebugInfo >& getChildrenDebugInfo() const { return m_childrenDebugInfo; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_className;
	bool m_mask = false;
	bool m_clipped = false;
	uint16_t m_frames = 0;
	uint16_t m_currentFrame = 0;
	bool m_playing = false;
	Aabb2 m_scalingGrid;
	AlignedVector< std::pair< uint32_t, std::string > > m_labels;
	RefArray< InstanceDebugInfo > m_childrenDebugInfo;
};

	}
}

