/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fbxsdk.h>

namespace traktor
{

class Matrix44;

	namespace model
	{

class Model;

bool convertMesh(
	Model& outModel,
	FbxScene* scene,
	FbxNode* meshNode,
	const Matrix44& axisTransform
);

	}
}
