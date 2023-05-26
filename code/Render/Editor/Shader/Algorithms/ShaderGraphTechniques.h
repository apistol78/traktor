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
#include <set>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ShaderGraph;
class Node;

/*! Shader graph technique generator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphTechniques : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphTechniques(const ShaderGraph* shaderGraph, const Guid& shaderGraphId, bool optimize = true);

	std::set< std::wstring > getNames() const;

	ShaderGraph* generate(const std::wstring& name) const;

private:
	std::map< std::wstring, Ref< ShaderGraph > > m_techniques;
};

}
