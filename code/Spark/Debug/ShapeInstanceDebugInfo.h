/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

class Shape;
class ShapeInstance;

/*!
 * \ingroup Spark
 */
class T_DLLCLASS ShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ShapeInstanceDebugInfo() = default;

	explicit ShapeInstanceDebugInfo(const ShapeInstance* instance, bool mask, bool clipped);

	const Shape* getShape() const { return m_shape; }

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< const Shape > m_shape;
	bool m_mask = false;
	bool m_clipped = false;
};

	}
}
