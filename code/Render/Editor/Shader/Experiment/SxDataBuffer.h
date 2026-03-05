/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Render/Editor/Shader/Experiment/SxData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS SxDataBuffer : public SxData
{
	T_RTTI_CLASS;

public:
    enum class Initial
	{
		Undefined,
		Zero,
		Random
	};

    const Guid& getStructDeclaration() const { return m_structDeclaration; }

    int32_t getCount() const { return m_count; }

    Initial getInitial() const { return m_initial; }

	virtual void serialize(ISerializer& s) override final;

private:
    Guid m_structDeclaration;
    int32_t m_count = 1;
    Initial m_initial = Initial::Undefined;
};

}
