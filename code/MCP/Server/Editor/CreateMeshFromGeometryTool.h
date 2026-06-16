/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "MCP/Server/IMcpTool.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mcp
{

class Json;

/*! Tool: generate a mesh from raw geometry data.
 * \ingroup MCP
 *
 * Builds a model::Model from supplied positions and polygons (with optional
 * per-position normals and texture coordinates), computes normals if absent,
 * triangulates, writes it to a model file under the asset path, and creates a
 * traktor.mesh.MeshAsset referencing it. The asset must be built before use.
 */
class CreateMeshFromGeometryTool : public IMcpTool
{
	T_RTTI_CLASS;

public:
	explicit CreateMeshFromGeometryTool(editor::IEditor* editor);

	virtual std::wstring getName() const override final;

	virtual std::wstring getDescription() const override final;

	virtual Ref< Json > getInputSchema() const override final;

	virtual Ref< Json > invoke(const Json* arguments, std::wstring& outError) override final;

private:
	editor::IEditor* m_editor;
};

}
