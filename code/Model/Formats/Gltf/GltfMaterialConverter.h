/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "cgltf.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/Path.h"

namespace traktor::model
{

class Model;

/*! Convert GLTF materials to engine materials.
 *
 * \param outModel Output model to add materials to.
 * \param outMaterialMap Map from GLTF material indices to engine material indices.
 * \param data GLTF data containing materials and textures.
 * \param filePath Path to the GLTF file for resolving texture paths.
 * \return True if conversion succeeded.
 */
bool convertMaterials(
	Model& outModel,
	SmallMap< cgltf_size, int32_t >& outMaterialMap,
	const cgltf_data* data,
	const Path& filePath);

}