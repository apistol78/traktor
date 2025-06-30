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
#include "Core/Ref.h"

namespace traktor
{
class Matrix44;
}

namespace traktor::model
{

class Model;
class Pose;

/*! Convert GLTF skin to engine skeleton.
 *
 * \param outModel Output model to add skeleton to.
 * \param data GLTF data containing skin information.
 * \param skin GLTF skin to convert.
 * \param axisTransform Coordinate system transformation matrix.
 * \return True if conversion succeeded.
 */
bool convertSkeleton(
	Model& outModel,
	const cgltf_data* data,
	const cgltf_skin* skin,
	const Matrix44& axisTransform);

/*! Convert GLTF animation to engine pose at specific time.
 *
 * \param model Engine model containing skeleton.
 * \param data GLTF data containing animation.
 * \param animation GLTF animation to convert.
 * \param time Time in seconds to sample animation.
 * \param axisTransform Coordinate system transformation matrix.
 * \return Pose at specified time, or null if conversion failed.
 */
Ref< Pose > convertPose(
	const Model& model,
	const cgltf_data* data,
	const cgltf_animation* animation,
	float time,
	const Matrix44& axisTransform);

}