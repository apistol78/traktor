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
#include "Core/Math/Vector4.h"
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Model.h"
#include "Model/Pose.h"

#include <functional>

namespace traktor::model
{
namespace
{

std::wstring getJointName(cgltf_node* node)
{
	return node->name ? mbstows(node->name) : L"";
}

bool traverse(cgltf_node* parent, cgltf_node* node, const std::function< bool(cgltf_node* parent, cgltf_node* node) >& visitor)
{
	if (!node)
		return true;

	if (!visitor(parent, node))
		return false;

	for (cgltf_size i = 0; i < node->children_count; ++i)
	{
		cgltf_node* child = node->children[i];
		if (child)
		{
			if (!traverse(node, child, visitor))
				return false;
		}
	}

	return true;
}

void getNodeLocalTransform(cgltf_node* node, cgltf_float* outMatrix)
{
	cgltf_node_transform_local(node, outMatrix);
}

}

bool convertSkeleton(
	Model& outModel,
	cgltf_data* data,
	cgltf_skin* skin,
	const Matrix44& axisTransform)
{
	if (!skin || skin->joints_count == 0)
		return false;

	// Get inverse bind matrices
	AlignedVector< Matrix44 > inverseBindMatrices;
	if (skin->inverse_bind_matrices)
	{
		inverseBindMatrices.resize(skin->inverse_bind_matrices->count);
		for (cgltf_size i = 0; i < skin->inverse_bind_matrices->count; ++i)
		{
			cgltf_float matrix[16];
			cgltf_accessor_read_float(skin->inverse_bind_matrices, i, matrix, 16);
			inverseBindMatrices[i] = convertMatrix(matrix);
		}
	}

	// Process all joints in the skin
	for (cgltf_size i = 0; i < skin->joints_count; ++i)
	{
		cgltf_node* jointNode = skin->joints[i];
		const std::wstring jointName = getJointName(jointNode);

		// Get local transform
		cgltf_float localMatrix[16];
		getNodeLocalTransform(jointNode, localMatrix);
		Matrix44 Mlocal = convertMatrix(localMatrix);

		// Apply axis transform if this is a root joint (no parent or parent is not a joint)
		bool isRootJoint = true;
		if (jointNode->parent)
		{
			// Check if parent is also a joint in this skin
			for (cgltf_size j = 0; j < skin->joints_count; ++j)
			{
				if (skin->joints[j] == jointNode->parent)
				{
					isRootJoint = false;
					break;
				}
			}
		}

		Matrix44 Mjoint = Mlocal;
		if (isRootJoint)
		{
			// Apply axis transform for root joints
			Mjoint = axisTransform * Mlocal;
		}

		// Find parent joint index
		uint32_t parentId = c_InvalidIndex;
		if (jointNode->parent)
		{
			const std::wstring parentJointName = getJointName(jointNode->parent);
			parentId = outModel.findJointIndex(parentJointName);
		}

		Joint joint;
		joint.setParent(parentId);
		joint.setName(jointName);
		joint.setTransform(Transform(Mjoint));
		outModel.addJoint(joint);
	}

	return true;
}

Ref< Pose > convertPose(
	const Model& model,
	cgltf_data* data,
	cgltf_animation* anim,
	cgltf_skin* skin,
	float time,
	const Matrix44& axisTransform)
{
	if (!anim || !skin)
		return nullptr;

	Ref< Pose > pose = new Pose();

	// For each joint in the skeleton
	for (cgltf_size jointIdx = 0; jointIdx < skin->joints_count; ++jointIdx)
	{
		cgltf_node* jointNode = skin->joints[jointIdx];
		const std::wstring jointName = getJointName(jointNode);
		const uint32_t traktorJointIdx = model.findJointIndex(jointName);

		if (traktorJointIdx == c_InvalidIndex)
			continue;

		// Find animation channels that target this joint
		Vector4 translation(0.0f, 0.0f, 0.0f, 1.0f);
		Vector4 rotation(0.0f, 0.0f, 0.0f, 1.0f); // quaternion (x, y, z, w)
		Vector4 scale(1.0f, 1.0f, 1.0f, 0.0f);

		bool hasTranslation = false;
		bool hasRotation = false;
		bool hasScale = false;

		for (cgltf_size chanIdx = 0; chanIdx < anim->channels_count; ++chanIdx)
		{
			cgltf_animation_channel* channel = &anim->channels[chanIdx];
			if (channel->target_node != jointNode)
				continue;

			cgltf_animation_sampler* sampler = channel->sampler;
			if (!sampler)
				continue;

			// Find the appropriate keyframe time indices
			cgltf_accessor* timeAccessor = sampler->input;
			cgltf_accessor* valueAccessor = sampler->output;

			if (!timeAccessor || !valueAccessor)
				continue;

			// Simple linear interpolation between keyframes
			cgltf_size keyIndex = 0;
			for (cgltf_size i = 0; i < timeAccessor->count - 1; ++i)
			{
				float t0, t1;
				cgltf_accessor_read_float(timeAccessor, i, &t0, 1);
				cgltf_accessor_read_float(timeAccessor, i + 1, &t1, 1);
				if (time >= t0 && time <= t1)
				{
					keyIndex = i;
					break;
				}
			}

			float t0, t1;
			cgltf_accessor_read_float(timeAccessor, keyIndex, &t0, 1);
			cgltf_accessor_read_float(timeAccessor, keyIndex + 1 < timeAccessor->count ? keyIndex + 1 : keyIndex, &t1, 1);

			float alpha = (t1 - t0) > 0.0f ? (time - t0) / (t1 - t0) : 0.0f;
			alpha = clamp(alpha, 0.0f, 1.0f);

			switch (channel->target_path)
			{
			case cgltf_animation_path_type_translation:
				{
					cgltf_float v0[3], v1[3];
					cgltf_accessor_read_float(valueAccessor, keyIndex, v0, 3);
					cgltf_accessor_read_float(valueAccessor, keyIndex + 1 < valueAccessor->count ? keyIndex + 1 : keyIndex, v1, 3);
					translation = lerp(convertPosition(v0), convertPosition(v1), Scalar(alpha));
					hasTranslation = true;
				}
				break;

			case cgltf_animation_path_type_rotation:
				{
					cgltf_float q0[4], q1[4];
					cgltf_accessor_read_float(valueAccessor, keyIndex, q0, 4);
					cgltf_accessor_read_float(valueAccessor, keyIndex + 1 < valueAccessor->count ? keyIndex + 1 : keyIndex, q1, 4);
					// Quaternion slerp would be better but lerp is simpler
					rotation = lerp(convertVector4(q0), convertVector4(q1), Scalar(alpha)).normalized();
					hasRotation = true;
				}
				break;

			case cgltf_animation_path_type_scale:
				{
					cgltf_float s0[3], s1[3];
					cgltf_accessor_read_float(valueAccessor, keyIndex, s0, 3);
					cgltf_accessor_read_float(valueAccessor, keyIndex + 1 < valueAccessor->count ? keyIndex + 1 : keyIndex, s1, 3);
					scale = lerp(convertPosition(s0), convertPosition(s1), Scalar(alpha));
					hasScale = true;
				}
				break;

			default:
				break;
			}
		}

		// If no animation data, use bind pose
		if (!hasTranslation && !hasRotation && !hasScale)
		{
			if (jointNode->has_matrix)
			{
				Matrix44 m = convertMatrix(jointNode->matrix);
				pose->setJointTransform(traktorJointIdx, Transform(m));
			}
			else
			{
				if (jointNode->has_translation)
					translation = convertPosition(jointNode->translation);
				if (jointNode->has_rotation)
					rotation = convertVector4(jointNode->rotation);
				if (jointNode->has_scale)
					scale = convertPosition(jointNode->scale);

				// Build transform from TRS
				Matrix44 T = translate(translation.xyz1());
				Matrix44 R = Matrix44::identity(); // Convert quaternion to matrix
				{
					float x = rotation.x(), y = rotation.y(), z = rotation.z(), w = rotation.w();
					float xx = x * x, yy = y * y, zz = z * z;
					float xy = x * y, xz = x * z, yz = y * z;
					float wx = w * x, wy = w * y, wz = w * z;
					R = Matrix44(
						1.0f - 2.0f * (yy + zz),
						2.0f * (xy + wz),
						2.0f * (xz - wy),
						0.0f,
						2.0f * (xy - wz),
						1.0f - 2.0f * (xx + zz),
						2.0f * (yz + wx),
						0.0f,
						2.0f * (xz + wy),
						2.0f * (yz - wx),
						1.0f - 2.0f * (xx + yy),
						0.0f,
						0.0f,
						0.0f,
						0.0f,
						1.0f);
				}
				Matrix44 S = traktor::scale(scale.x(), scale.y(), scale.z());
				pose->setJointTransform(traktorJointIdx, Transform(T * R * S));
			}
		}
		else
		{
			// Build transform from TRS
			Matrix44 T = translate(translation.xyz1());
			Matrix44 R = Matrix44::identity(); // Convert quaternion to matrix
			{
				float x = rotation.x(), y = rotation.y(), z = rotation.z(), w = rotation.w();
				float xx = x * x, yy = y * y, zz = z * z;
				float xy = x * y, xz = x * z, yz = y * z;
				float wx = w * x, wy = w * y, wz = w * z;
				R = Matrix44(
					1.0f - 2.0f * (yy + zz),
					2.0f * (xy + wz),
					2.0f * (xz - wy),
					0.0f,
					2.0f * (xy - wz),
					1.0f - 2.0f * (xx + zz),
					2.0f * (yz + wx),
					0.0f,
					2.0f * (xz + wy),
					2.0f * (yz - wx),
					1.0f - 2.0f * (xx + yy),
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f);
			}
			Matrix44 S = traktor::scale(scale.x(), scale.y(), scale.z());
			pose->setJointTransform(traktorJointIdx, Transform(T * R * S));
		}
	}

	return pose;
}

}
