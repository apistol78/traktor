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

class MorphShapeInstance;

/*!
 * \ingroup Spark
 */
class T_DLLCLASS MorphShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	MorphShapeInstanceDebugInfo() = default;

	explicit MorphShapeInstanceDebugInfo(const MorphShapeInstance* instance, bool mask, bool clipped);

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_mask = false;
	bool m_clipped = false;
};

	}
}
