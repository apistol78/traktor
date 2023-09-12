/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class T_DLLCLASS MovieAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	struct Font
	{
		std::wstring name;
		traktor::Path fileName;

		void serialize(ISerializer& s);
	};

	const AlignedVector< Font >& getFonts() const { return m_fonts; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class Pipeline;

	bool m_staticMovie = false;
	AlignedVector< Font > m_fonts;

};

}
