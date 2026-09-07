/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/GltfSkeletonConverter.h"

#include "Core/Containers/SmallMap.h"
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

	for (cgltf_size i = 0; i < skin->joints_count; ++i)
	{
		const cgltf_node* jointNode = skin->joints[i];
		if (!jointNode)
		{
			log::error << L"Invalid joint node in skin." << Endl;
			return false;
		}

		// Find parent joint within the skin.
		uint32_t parentId = c_InvalidIndex;
		if (jointNode->parent)
		{
			for (cgltf_size j = 0; j < skin->joints_count; ++j)
			{
				if (skin->joints[j] == jointNode->parent)
				{
					parentId = (uint32_t)j;
					break;
				}
			}
		}

		// glTF node transforms are already parent relative; a root joint instead
		// composes any non-joint ancestor transforms so the skeleton lands
		// correctly in object space.
		Matrix44 Mjoint;
		if (parentId != c_InvalidIndex)
			Mjoint = getNodeTransform(jointNode);
		else
		{
			cgltf_float world[16];
			cgltf_node_transform_world(jointNode, world);
			Mjoint = convertMatrix(world);
		}

		// Engine joint transforms are rigid; normalize any scale away.
		const Vector4 S(
			1.0f / Mjoint.axisX().length(),
			1.0f / Mjoint.axisY().length(),
			1.0f / Mjoint.axisZ().length());
		Mjoint = Mjoint * scale(S);

		// Convert into engine space by conjugation so joints stay consistent
		// with the reflected vertex positions. Renormalize the extracted
		// rotation; near 180 degrees the extraction amplifies matrix noise.
		Mjoint = axisTransform * Mjoint * axisTransform.inverse();

		Joint joint;
		joint.setName(getNodeName(jointNode));
		joint.setParent(parentId);
		joint.setTransform(Transform(Mjoint.translation().xyz0(), Quaternion(Mjoint).normalized()));
		joints.push_back(joint);
	}

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

	// Gather sampled channel values per joint; components without a channel fall
	// back to the target node's static local TRS (cgltf pre-fills the glTF
	// defaults, and animated nodes cannot use the matrix form).
	struct JointSample
	{
		Vector4 translation = Vector4::zero();
		Quaternion rotation = Quaternion::identity();
	};
	SmallMap< uint32_t, JointSample > samples;

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

		if (samples.find(jointIndex) == samples.end())
		{
			const cgltf_node* node = channel->target_node;
			JointSample initial;
			initial.translation = Vector4(node->translation[0], node->translation[1], node->translation[2], 0.0f);
			initial.rotation = Quaternion(Vector4(node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]));
			samples[jointIndex] = initial;
		}
		JointSample& sample = samples[jointIndex];

		const cgltf_animation_sampler* sampler = channel->sampler;
		switch (channel->target_path)
		{
		case cgltf_animation_path_type_translation:
			sample.translation = sampleVector(
				sampler->input,
				sampler->output,
				time,
				sampler->interpolation,
				3);
			break;

		case cgltf_animation_path_type_rotation:
			sample.rotation = Quaternion(sampleVector(
				sampler->input,
				sampler->output,
				time,
				sampler->interpolation,
				4));
			break;

		case cgltf_animation_path_type_scale:
			// Engine joint transforms are rigid; scale is ignored.
			break;

		case cgltf_animation_path_type_weights:
			// Morph target weights - not supported in basic joint animation
			break;

		default:
			break;
		}
	}

	Ref< Pose > pose = new Pose();

	// Pose transforms replace each joint's bind transform, so give every joint
	// an explicit value; unanimated joints hold their bind pose.
	for (uint32_t i = 0; i < model.getJointCount(); ++i)
		pose->setJointTransform(i, model.getJoint(i).getTransform());

	// Compose sampled locals and convert into engine space by the same
	// conjugation used for the skeleton's bind transforms.
	for (const auto& it : samples)
	{
		const Matrix44 local = translate(it.second.translation) * it.second.rotation.normalized().toMatrix44();
		const Matrix44 Mjoint = axisTransform * local * axisTransform.inverse();
		pose->setJointTransform(it.first, Transform(Mjoint.translation().xyz0(), Quaternion(Mjoint).normalized()));
	}

	return pose;
}

}