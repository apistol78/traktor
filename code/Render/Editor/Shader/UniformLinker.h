/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <string>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

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
class UniformDeclaration;

class T_DLLCLASS UniformLinker : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::pair< std::wstring, Ref< const UniformDeclaration > > named_decl_t;
	typedef std::function< named_decl_t (const Guid&) > fn_reader_t;

	explicit UniformLinker(const fn_reader_t& declarationReader);

	Ref< ShaderGraph > resolve(const ShaderGraph* shaderGraph) const;

private:
	fn_reader_t m_declarationReader;
};

}
