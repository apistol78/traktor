/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <string>
#include <vector>

namespace traktor::model
{

class Model;

}

namespace traktor::mcp
{

/*! In-memory store of working models for the MCP model tools.
 * \ingroup MCP
 *
 * A traktor::model::Model is a transient object (not a database instance), so
 * editing one across several MCP calls needs a place for it to live between
 * calls. This store keeps opened/created models in editor memory keyed by an
 * integer handle; tools reference "the model I'm working on" by that handle
 * until it is explicitly saved and/or closed.
 *
 * All access happens on the single MCP server worker thread, so no locking is
 * required.
 */
class ModelSession : public Object
{
	T_RTTI_CLASS;

public:
	enum class SourceKind
	{
		Blank = 0,		//!< Created empty, from scratch.
		File = 1,		//!< Read from a model file via ModelFormat.
		MeshAsset = 2	//!< Read from a MeshAsset's referenced source file.
	};

	struct Entry
	{
		Ref< model::Model > model;
		SourceKind sourceKind = SourceKind::Blank;
		std::wstring filePath;		//!< Absolute source/target file (File/MeshAsset), empty for Blank.
		Guid meshAssetGuid;			//!< Originating MeshAsset instance guid (MeshAsset only).
		std::wstring importFilter;	//!< Import filter used to read the source (MeshAsset only).
	};

	/*! Register a model and return its new handle (always >= 1). */
	int32_t open(model::Model* model, SourceKind sourceKind, const std::wstring& filePath, const Guid& meshAssetGuid, const std::wstring& importFilter);

	/*! Resolve a model by handle; null if unknown. */
	model::Model* get(int32_t handle) const;

	/*! Resolve full entry metadata by handle; null if unknown. */
	const Entry* entry(int32_t handle) const;

	/*! Release a handle; false if it was not open. */
	bool close(int32_t handle);

	/*! Release every open handle; returns the number closed. */
	int32_t closeAll();

	/*! Currently open handles, ascending. */
	std::vector< int32_t > handles() const;

private:
	SmallMap< int32_t, Entry > m_entries;
	int32_t m_nextHandle = 1;
};

}
