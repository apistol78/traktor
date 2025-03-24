/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/Misc/Preprocessor.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <functional>

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
class ShaderModule;

class T_DLLCLASS ShaderModuleResolve : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::pair< std::wstring, Ref< const Object > > named_decl_t;
	typedef std::function< named_decl_t(const Guid&) > fn_reader_t;

	explicit ShaderModuleResolve(const fn_reader_t& readObject);

	Ref< ShaderModule > resolve(const ShaderGraph* shaderGraph) const;

private:
	fn_reader_t m_readObject;
	Preprocessor m_preprocessor;

	bool collectModulesInDependencyOrder(const Guid& id, AlignedVector< Guid >& outIds) const;
};

}
