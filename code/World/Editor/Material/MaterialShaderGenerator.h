/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IPipelineDepends;

}

namespace traktor::render
{

class ShaderGraph;

}

namespace traktor::model
{

class Material;

}

namespace traktor::world
{

/*! Generate shader from material specification.
 * \ingroup World
 */
class T_DLLCLASS MaterialShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	explicit MaterialShaderGenerator(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve);

	Ref< render::ShaderGraph > generateSurface(
		const model::Material& material,
		bool vertexColor,
		bool decalResponse
	) const;

	Ref< render::ShaderGraph > combineSurface(
		const render::ShaderGraph* customSurfaceShaderGraph,
		const render::ShaderGraph* materialSurfaceShaderGraph
	) const;

	static void addDependencies(editor::IPipelineDepends* pipelineDepends);

private:
	std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) > m_resolve;
};

}
