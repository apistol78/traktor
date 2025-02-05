/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <functional>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
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
class Model;

}

namespace traktor::mesh
{

class T_DLLCLASS VertexShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	explicit VertexShaderGenerator(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve);

	Ref< render::ShaderGraph > generateMesh(
		const model::Model& model,
		const model::Material& material,
		const render::ShaderGraph* meshSurfaceShaderGraph,
		const Guid& vertexShaderGuid) const;

	static void addDependencies(editor::IPipelineDepends* pipelineDepends);

private:
	std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) > m_resolve;
};

}
