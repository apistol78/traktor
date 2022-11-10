/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace spray
	{

/*! Point set
 * \ingroup Spray
 */
class T_DLLCLASS PointSet : public Object
{
	T_RTTI_CLASS;

public:
	struct Point
	{
		Vector4 position;
		Vector4 normal;
		Vector4 color;
	};

	void add(const Point& point);

	const AlignedVector< Point >& get() const { return m_points; }

	bool read(IStream* stream);

	bool write(IStream* stream) const;

private:
	AlignedVector< Point > m_points;
};

	}
}

