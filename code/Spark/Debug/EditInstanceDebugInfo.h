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
#include "Spark/Swf/SwfTypes.h"

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

class EditInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS EditInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	EditInstanceDebugInfo() = default;

	explicit EditInstanceDebugInfo(const EditInstance* instance);

	const std::wstring& getText() const { return m_text; }

	const Color4f& getTextColor() const { return m_textColor; }

	SwfTextAlignType getTextAlign() const { return m_textAlign; }

	const Vector2& getTextSize() const { return m_textSize; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_text;
	Color4f m_textColor;
	SwfTextAlignType m_textAlign = StaLeft;
	Vector2 m_textSize;
};

	}
}
