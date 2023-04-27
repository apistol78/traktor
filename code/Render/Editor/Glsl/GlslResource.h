/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class GlslLayout;

class T_DLLCLASS GlslResource : public Object
{
	T_RTTI_CLASS;

public:
	enum BindStages
	{
		BsVertex = 1,
		BsFragment = 2,
		BsCompute = 4
	};

	int32_t getBinding() const { return m_binding; }

	void addStage(uint8_t stage) { m_stages |= stage; }

	uint8_t getStages() const { return m_stages; }

	const std::wstring& getName() const { return m_name; }

	virtual int32_t getOrdinal() const = 0;

	virtual bool isBindless() const = 0;

protected:
	explicit GlslResource(const std::wstring& name, uint8_t stages);

private:
	friend class GlslLayout;

	std::wstring m_name;
	uint8_t m_stages = 0;
	int32_t m_binding = 0;
};

}
