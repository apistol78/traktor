/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "ufbx.h"
#include "Core/Ref.h"

namespace traktor
{

class Matrix44;

}

namespace traktor::model
{

class Model;
class Pose;

bool convertSkeleton(
	Model& outModel,
	ufbx_scene* scene,
	ufbx_node* skeletonNode,
	const Matrix44& axisTransform
);

//Ref< Pose > convertPose(
//	const Model& model,
//	FbxScene* scene,
//	FbxNode* skeletonNode,
//	const FbxTime& time,
//	const Matrix44& axisTransform
//);

}
