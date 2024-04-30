/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/SkeletonConverter.h"

namespace traktor::model
{
	namespace
	{

ufbx_node* search(ufbx_node* node, const std::function< bool (ufbx_node* node) >& predicate)
{
	if (predicate(node))
		return node;

	for (size_t i = 0; i < node->children.count; ++i)
	{
		ufbx_node* child = node->children.data[i];
		if (child)
		{
			ufbx_node* foundNode = search(child, predicate);
			if (foundNode)
				return foundNode;
		}
	}

	return nullptr;
}

bool traverse(ufbx_node* parent, ufbx_node* node, const std::function< bool (ufbx_node* parent, ufbx_node* node) >& visitor)
{
	if (!node)
		return true;

	if (!visitor(parent, node))
		return false;

	for (size_t i = 0; i < node->children.count; ++i)
	{
		ufbx_node* child = node->children.data[i];
		if (child)
		{
			if (!traverse(node, child, visitor))
				return false;
		}
	}

	return true;
}

std::wstring getJointName(ufbx_node* node)
{
	std::wstring jointName = mbstows(node->name.data);

	const size_t p = jointName.find(L':');
	if (p != std::wstring::npos)
		jointName = jointName.substr(p + 1);

	return jointName;
}

ufbx_bone_pose* findBonePose(ufbx_pose* bindPose, ufbx_node* node)
{
	for (size_t i = 0; i < bindPose->bone_poses.count; ++i)
	{
		ufbx_bone_pose& pose = bindPose->bone_poses.data[i];
		if (pose.bone_node == node)
			return &pose;
	}
	return nullptr;	
}

	}

bool convertSkeleton(
	Model& outModel,
	ufbx_scene* scene,
	ufbx_node* skeletonNode,
	const Matrix44& axisTransform
)
{
	ufbx_pose* bindPose = skeletonNode->bind_pose;
	T_FATAL_ASSERT(bindPose != nullptr);

	const Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));

	const bool result = traverse(nullptr, skeletonNode, [&](ufbx_node* parent, ufbx_node* node) {
		const std::wstring jointName = getJointName(node);

		Matrix44 Mnode = Matrix44::identity();

		ufbx_bone_pose* pose = findBonePose(bindPose, node);
		if (pose)
		{
			Mnode = convertMatrix(pose->bone_to_world);
		}
		else
		{
			pose = findBonePose(bindPose, parent);
			if (pose)
				Mnode = convertMatrix(pose->bone_to_world) * convertMatrix(node->node_to_parent);
		}

		// Calculate joint transformation.
		Matrix44 Mjoint = Mnode * Mrx90;

		const Vector4 S(
			1.0f / Mjoint.axisX().length(),
			1.0f / Mjoint.axisY().length(),
			1.0f / Mjoint.axisZ().length()
		);
		Mjoint = Mjoint * scale(S);
		Mjoint = axisTransform * Mjoint * axisTransform.inverse();

		uint32_t parentId = c_InvalidIndex;
		if (parent != nullptr)
		{
			const std::wstring parentJointName = getJointName(parent);
			parentId = outModel.findJointIndex(parentJointName);
			if (parentId != c_InvalidIndex)
			{
				const Matrix44 Mparent = outModel.getJointGlobalTransform(parentId).toMatrix44();
				Mjoint = Mparent.inverse() * Mjoint;	// Cl = Bg-1 * Cg
			}
			else
				log::warning << L"Unable to bind parent joint; no such joint \"" << parentJointName << L"\"." << Endl;
		}

		Joint joint;
		joint.setParent(parentId);
		joint.setName(jointName);
		joint.setTransform(Transform(Mjoint));
		outModel.addJoint(joint);
		return true;
	});

	return true;
}

Ref< Pose > convertPose(
	const Model& model,
	ufbx_scene* scene,
	ufbx_node* skeletonNode,
	ufbx_anim* anim,
	double time,
	const Matrix44& axisTransform
)
{
	ufbx_evaluate_opts opts = {};
	ufbx_scene* escene = ufbx_evaluate_scene(scene, anim, time, &opts, nullptr);
	if (!escene)
		return nullptr;

	// Find evaluated skeleton.
	ufbx_node* eskeletonNode = search(escene->root_node, [&](ufbx_node* node) {
		return node->element_id == skeletonNode->element_id;
	});
	if (!eskeletonNode)
		return nullptr;

	const Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));

	Ref< Pose > pose = new Pose();
	const bool result = traverse(nullptr, eskeletonNode, [&](ufbx_node* parent, ufbx_node* node) {
		const std::wstring jointName = getJointName(node);
		const uint32_t jointId = model.findJointIndex(jointName);
		if (jointId == c_InvalidIndex)
		{
			log::warning << L"Unable to find joint \"" << jointName << L"\" in skeleton; unable to save pose for joint." << Endl;
			return true;
		}

		// Calculate joint transformation.
		const Matrix44 Mnode = convertMatrix(node->geometry_to_world);
		Matrix44 Mjoint = Mnode * Mrx90;

		const Vector4 S(
			1.0f / Mjoint.axisX().length(),
			1.0f / Mjoint.axisY().length(),
			1.0f / Mjoint.axisZ().length()
		);
		Mjoint = Mjoint * scale(S);
		Mjoint = axisTransform * Mjoint * axisTransform.inverse();

		if (parent != nullptr)
		{
			const std::wstring parentJointName = getJointName(parent);
			const uint32_t parentId = model.findJointIndex(parentJointName);
			if (parentId != c_InvalidIndex)
			{
				const Matrix44 Mparent = pose->getJointGlobalTransform(&model, parentId).toMatrix44();
				Mjoint = Mparent.inverse() * Mjoint;	// Cl = Bg-1 * Cg
			}
			else
				log::warning << L"Unable to bind parent joint; no such joint \"" << parentJointName << L"\"." << Endl;
		}

		pose->setJointTransform(jointId, Transform(Mjoint));
		return true;
	});

	ufbx_free_scene(escene);
	return pose;
}

}
