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

namespace traktor
{
class Matrix44;
}

namespace traktor::model
{

class Model;

/*! Convert a GLTF mesh node to engine mesh data.
 *
 * \param outModel Output model to add mesh data to.
 * \param data GLTF data containing mesh information.
 * \param node GLTF node containing the mesh.
 * \param materialMap Map from GLTF material indices to engine material indices.
 * \param axisTransform Coordinate system transformation matrix.
 * \return True if conversion succeeded.
 */
bool convertMesh(
	Model& outModel,
	const cgltf_data* data,
	const cgltf_node* node,
	const SmallMap< cgltf_size, int32_t >& materialMap,
	const Matrix44& axisTransform);

}