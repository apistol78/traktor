/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/GltfSkeletonConverter.h"

#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Model.h"
#include "Model/Pose.h"

namespace traktor::model
{
namespace
{

std::wstring getNodeName(const cgltf_node* node)
{
	if (node->name)
		return mbstows(node->name);
	else
		return L"Joint_" + toString((uintptr_t)node);
}

float sampleAnimation(const cgltf_accessor* timeAccessor, const cgltf_accessor* valueAccessor, float time, cgltf_size componentIndex, cgltf_interpolation_type interpolation)
{
	if (!timeAccessor || !valueAccessor || timeAccessor->count == 0)
		return 0.0f;

	// Find the time frame
	cgltf_size frameIndex = 0;
	float frameTime0 = 0.0f, frameTime1 = 0.0f;

	// Simple linear search for now
	for (cgltf_size i = 0; i < timeAccessor->count - 1; ++i)
	{
		cgltf_accessor_read_float(timeAccessor, i, &frameTime0, 1);
		cgltf_accessor_read_float(timeAccessor, i + 1, &frameTime1, 1);

		if (time >= frameTime0 && time <= frameTime1)
		{
			frameIndex = i;
			break;
		}
	}

	// Handle edge cases
	if (time <= frameTime0)
	{
		// Before first frame
		cgltf_float value;
		if (!cgltf_accessor_read_float(valueAccessor, 0, &value, 1))
			return 0.0f;
		return value;
	}

	if (frameIndex >= timeAccessor->count - 1)
	{
		// After last frame
		cgltf_float value;
		if (!cgltf_accessor_read_float(valueAccessor, timeAccessor->count - 1, &value, 1))
			return 0.0f;
		return value;
	}

	// Interpolate between frames
	const float t = (time - frameTime0) / (frameTime1 - frameTime0);

	cgltf_float value0, value1;
	if (!cgltf_accessor_read_float(valueAccessor, frameIndex, &value0, 1) ||
		!cgltf_accessor_read_float(valueAccessor, frameIndex + 1, &value1, 1))
		return 0.0f;

	switch (interpolation)
	{
	case cgltf_interpolation_type_step:
		return value0;

	case cgltf_interpolation_type_linear:
		return value0 + t * (value1 - value0);

	case cgltf_interpolation_type_cubic_spline:
		// TODO: Implement cubic spline interpolation
		return value0 + t * (value1 - value0);

	default:
		return value0;
	}
}

Vector4 sampleVector(const cgltf_accessor* timeAccessor, const cgltf_accessor* valueAccessor, float time, cgltf_interpolation_type interpolation, cgltf_size componentCount)
{
	Vector4 result(0.0f, 0.0f, 0.0f, componentCount == 4 ? 1.0f : 0.0f);

	if (!timeAccessor || !valueAccessor || timeAccessor->count == 0)
		return result;

	// Find the time frame
	cgltf_size frameIndex = 0;
	float frameTime0 = 0.0f, frameTime1 = 0.0f;

	for (cgltf_size i = 0; i < timeAccessor->count - 1; ++i)
	{
		cgltf_accessor_read_float(timeAccessor, i, &frameTime0, 1);
		cgltf_accessor_read_float(timeAccessor, i + 1, &frameTime1, 1);

		if (time >= frameTime0 && time <= frameTime1)
		{
			frameIndex = i;
			break;
		}
	}

	// Handle edge cases
	if (time <= frameTime0)
	{
		cgltf_float values[4];
		if (cgltf_accessor_read_float(valueAccessor, 0, values, componentCount))
			for (cgltf_size i = 0; i < componentCount && i < 4; ++i)
				result.set(i, Scalar(values[i]));
		return result;
	}

	if (frameIndex >= timeAccessor->count - 1)
	{
		cgltf_float values[4];
		if (cgltf_accessor_read_float(valueAccessor, timeAccessor->count - 1, values, componentCount))
			for (cgltf_size i = 0; i < componentCount && i < 4; ++i)
				result.set(i, Scalar(values[i]));
		return result;
	}

	// Interpolate between frames
	const float t = (time - frameTime0) / (frameTime1 - frameTime0);

	cgltf_float values0[4], values1[4];
	if (cgltf_accessor_read_float(valueAccessor, frameIndex, values0, componentCount) &&
		cgltf_accessor_read_float(valueAccessor, frameIndex + 1, values1, componentCount))
	{
		for (cgltf_size i = 0; i < componentCount && i < 4; ++i)
		{
			switch (interpolation)
			{
			case cgltf_interpolation_type_step:
				result.set(i, Scalar(values0[i]));
				break;

			case cgltf_interpolation_type_linear:
				result.set(i, Scalar(values0[i] + t * (values1[i] - values0[i])));
				break;

			case cgltf_interpolation_type_cubic_spline:
				// TODO: Implement cubic spline interpolation
				result.set(i, Scalar(values0[i] + t * (values1[i] - values0[i])));
				break;

			default:
				result.set(i, Scalar(values0[i]));
				break;
			}
		}
	}

	return result;
}

}

bool convertSkeleton(
	Model& outModel,
	const cgltf_data* data,
	const cgltf_skin* skin,
	const Matrix44& axisTransform)
{
	if (!skin || skin->joints_count == 0)
		return false;

	AlignedVector< Joint > joints;
	joints.reserve(skin->joints_count);

	// First pass: create all joints
	for (cgltf_size i = 0; i < skin->joints_count; ++i)
	{
		const cgltf_node* jointNode = skin->joints[i];
		if (!jointNode)
		{
			log::error << L"Invalid joint node in skin." << Endl;
			return false;
		}

		Joint joint;
		joint.setName(getNodeName(jointNode));
		joint.setParent(c_InvalidIndex); // Will be set in second pass

		// Get node transform
		Matrix44 nodeTransform = getNodeTransform(jointNode);

		// Apply inverse bind matrix if available
		if (skin->inverse_bind_matrices && i < skin->inverse_bind_matrices->count)
		{
			cgltf_float invBindMatrix[16];
			if (cgltf_accessor_read_float(skin->inverse_bind_matrices, i, invBindMatrix, 16))
			{
				Matrix44 invBind = convertMatrix(invBindMatrix);
				// The joint transform should incorporate the inverse bind matrix
				// For now, we'll use the node transform directly
			}
		}

		// Apply axis transform
		nodeTransform = axisTransform * nodeTransform;
		joint.setTransform(Transform(nodeTransform));

		joints.push_back(joint);
	}

	// Second pass: set up parent relationships
	for (cgltf_size i = 0; i < skin->joints_count; ++i)
	{
		const cgltf_node* jointNode = skin->joints[i];

		if (jointNode->parent)
		{
			// Find parent in joints array
			for (cgltf_size j = 0; j < skin->joints_count; ++j)
			{
				if (skin->joints[j] == jointNode->parent)
				{
					joints[i].setParent((uint32_t)j);

					// Convert to relative transform
					const Matrix44 parentTransform = joints[j].getTransform().toMatrix44();
					const Matrix44 currentTransform = joints[i].getTransform().toMatrix44();
					const Matrix44 relativeTransform = parentTransform.inverse() * currentTransform;
					joints[i].setTransform(Transform(relativeTransform));
					break;
				}
			}
		}
	}

	// Add joints to model
	outModel.setJoints(joints);
	return true;
}

Ref< Pose > convertPose(
	const Model& model,
	const cgltf_data* data,
	const cgltf_animation* animation,
	float time,
	const Matrix44& axisTransform)
{
	if (!animation || animation->channels_count == 0)
		return nullptr;

	Ref< Pose > pose = new Pose();

	// Process each animation channel
	for (cgltf_size i = 0; i < animation->channels_count; ++i)
	{
		const cgltf_animation_channel* channel = &animation->channels[i];

		if (!channel->target_node || !channel->sampler)
			continue;

		// Find joint index in model
		const std::wstring jointName = getNodeName(channel->target_node);
		const uint32_t jointIndex = model.findJointIndex(jointName);

		if (jointIndex == c_InvalidIndex)
		{
			log::warning << L"Joint \"" << jointName << L"\" not found in skeleton." << Endl;
			continue;
		}

		const cgltf_animation_sampler* sampler = channel->sampler;

		// Get current joint transform
		Transform currentTransform = pose->getJointTransform(jointIndex);
		Matrix44 transformMatrix = currentTransform.toMatrix44();

		// Sample animation data based on target path
		switch (channel->target_path)
		{
		case cgltf_animation_path_type_translation:
			{
				Vector4 translation = sampleVector(
					sampler->input,
					sampler->output,
					time,
					sampler->interpolation,
					3);
				Vector4 transformedTranslation = axisTransform * translation;
				// Create new transform matrix with updated translation
				transformMatrix = Matrix44(
					transformMatrix.axisX(),
					transformMatrix.axisY(),
					transformMatrix.axisZ(),
					transformedTranslation);
			}
			break;

		case cgltf_animation_path_type_rotation:
			{
				Vector4 rotation = sampleVector(
					sampler->input,
					sampler->output,
					time,
					sampler->interpolation,
					4);
				Quaternion quat(rotation);
				Matrix44 rotMatrix = quat.toMatrix44();
				// Apply to existing transform
				transformMatrix = transformMatrix * (axisTransform * rotMatrix * axisTransform.inverse());
			}
			break;

		case cgltf_animation_path_type_scale:
			{
				Vector4 scaleVec = sampleVector(
					sampler->input,
					sampler->output,
					time,
					sampler->interpolation,
					3);
				Matrix44 scaleMatrix = traktor::scale(scaleVec.x(), scaleVec.y(), scaleVec.z());
				transformMatrix = transformMatrix * scaleMatrix;
			}
			break;

		case cgltf_animation_path_type_weights:
			// Morph target weights - not supported in basic joint animation
			break;

		default:
			break;
		}

		pose->setJointTransform(jointIndex, Transform(transformMatrix));
	}

	return pose;
}

}