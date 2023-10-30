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
#include "Spark/Character.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Frame;

/*! Sprite
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	Sprite() = default;

	explicit Sprite(uint16_t frameRate);

	uint16_t getFrameRate() const;

	void addFrame(Frame* frame);

	uint32_t getFrameCount() const;

	Frame* getFrame(uint32_t frameId) const;

	int findFrame(const std::string& frameLabel) const;

	void setScalingGrid(const Aabb2& scalingGrid);

	const Aabb2& getScalingGrid() const;

	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	uint16_t m_frameRate = 0;
	RefArray< Frame > m_frames;
	Aabb2 m_scalingGrid;
};

}
