/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4ub.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class T_DLLCLASS EmptyMovieAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	EmptyMovieAsset();

	virtual void serialize(ISerializer& s) override final;

	int32_t getStageWidth() const { return m_stageWidth; }

	int32_t getStageHeight() const { return m_stageHeight; }

	int32_t getFrameRate() const { return m_frameRate; }

	const Color4ub& getBackgroundColor() const { return m_backgroundColor; }

private:
	int32_t m_stageWidth;
	int32_t m_stageHeight;
	int32_t m_frameRate;
	Color4ub m_backgroundColor;
};

	}
}

