/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

//bool traverse(FbxNode* parent, FbxNode* node, const std::function< bool (FbxNode* parent, FbxNode* node) >& visitor)
//{
//	if (!node)
//		return true;
//
//	if (!visitor(parent, node))
//		return false;
//
//	const int32_t childCount = node->GetChildCount();
//	for (int32_t i = 0; i < childCount; ++i)
//	{
//		FbxNode* childNode = node->GetChild(i);
//		if (childNode)
//		{
//			if (!traverse(node, childNode, visitor))
//				return false;
//		}
//	}
//
//	return true;
//}
//
//std::wstring getJointName(FbxNode* node)
//{
//	std::wstring jointName = mbstows(node->GetName());
//
//	const size_t p = jointName.find(L':');
//	if (p != std::wstring::npos)
//		jointName = jointName.substr(p + 1);
//
//	return jointName;
//}

	}

//bool convertSkeleton(
//	Model& outModel,
//	FbxScene* scene,
//	FbxNode* skeletonNode,
//	const Matrix44& axisTransform
//)
//{
//	FbxSkeleton* skeleton = skeletonNode->GetSkeleton();
//	if (!skeleton)
//		return false;
//
//	// Find bind pose, if skeleton is animated then we cannot use
//	// global evaluation as it's the first frame of the animation.
//	FbxPose* bindPose = nullptr;
//	const int32_t poseCount = scene->GetPoseCount();
//    for (int32_t i = 0; i < poseCount; i++)
//    {
//        FbxPose* pose = scene->GetPose(i);
//		if (pose->IsBindPose())
//		{
//			bindPose = pose;
//			break;
//		}
//	}
//
//	if (!bindPose)
//		log::warning << L"No bind pose in FBX; skeleton might not be correct." << Endl;
//
//	const bool result = traverse(nullptr, skeletonNode, [&](FbxNode* parent, FbxNode* node) {
//		const std::wstring jointName = getJointName(node);
//
//		// Calculate joint transformation.
//		FbxMatrix nodeTransform;
//		if (bindPose)
//		{
//			int32_t id = bindPose->Find(node);
//			if (id >= 0)
//				nodeTransform = bindPose->GetMatrix(id);
//			else
//			{
//				id = bindPose->Find(parent);
//				if (id < 0)
//				{
//					log::error << L"Parent of \"" << jointName << L"\" doesn't have a bind pose matrix, cannot synthesize bind pose matrix." << Endl;
//					return true;
//				}
//				nodeTransform = bindPose->GetMatrix(id) * node->EvaluateLocalTransform();
//			}
//		}
//		else
//		{
//			// No bind pose available we use global transforms of each joint.
//			nodeTransform = node->EvaluateGlobalTransform();
//		}
//
//		const Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));
//
//		const Matrix44 Mnode = convertMatrix(nodeTransform);
//		Matrix44 Mjoint = Mnode * Mrx90;
//
//		const Vector4 S(
//			1.0f / Mjoint.axisX().length(),
//			1.0f / Mjoint.axisY().length(),
//			1.0f / Mjoint.axisZ().length()
//		);
//		Mjoint = Mjoint * scale(S);
//		Mjoint = axisTransform * Mjoint * axisTransform.inverse();
//
//		uint32_t parentId = c_InvalidIndex;
//		if (parent != nullptr)
//		{
//			const std::wstring parentJointName = getJointName(parent);
//			parentId = outModel.findJointIndex(parentJointName);
//			if (parentId != c_InvalidIndex)
//			{
//				const Matrix44 Mparent = outModel.getJointGlobalTransform(parentId).toMatrix44();
//				Mjoint = Mparent.inverse() * Mjoint;	// Cl = Bg-1 * Cg
//			}
//			else
//				log::warning << L"Unable to bind parent joint; no such joint \"" << parentJointName << L"\"." << Endl;
//		}
//
//		Joint joint;
//		joint.setParent(parentId);
//		joint.setName(jointName);
//		joint.setTransform(Transform(Mjoint));
//		outModel.addJoint(joint);
//		return true;
//	});
//
//	return true;
//}
//
//Ref< Pose > convertPose(
//	const Model& model,
//	FbxScene* scene,
//	FbxNode* skeletonNode,
//	const FbxTime& time,
//	const Matrix44& axisTransform
//)
//{
//	FbxSkeleton* skeleton = skeletonNode->GetSkeleton();
//	if (!skeleton)
//		return nullptr;
//
//	const Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));
//
//	Ref< Pose > pose = new Pose();
//	traverse(nullptr, skeletonNode, [&](FbxNode* parent, FbxNode* node) {
//		FbxAMatrix nodeTransform = node->EvaluateGlobalTransform(time, FbxNode::eSourcePivot, true, true);
//
//		const Matrix44 Mnode = convertMatrix(nodeTransform);
//		Matrix44 Mjoint = Mnode * Mrx90;
//
//		std::wstring jointName = mbstows(node->GetName());
//
//		const size_t p = jointName.find(L':');
//		if (p != std::wstring::npos)
//			jointName = jointName.substr(p + 1);
//
//		const uint32_t jointId = model.findJointIndex(jointName);
//		if (jointId == c_InvalidIndex)
//		{
//			log::warning << L"Unable to find joint \"" << jointName << L"\" in skeleton; unable to save pose for joint." << Endl;
//			return true;
//		}
//
//		const Vector4 S(
//			1.0f / Mjoint.axisX().length(),
//			1.0f / Mjoint.axisY().length(),
//			1.0f / Mjoint.axisZ().length()
//		);
//		Mjoint = Mjoint * scale(S);
//		Mjoint = axisTransform * Mjoint * axisTransform.inverse();
//
//		if (parent != nullptr)
//		{
//			const std::wstring parentJointName = getJointName(parent);
//			const uint32_t parentId = model.findJointIndex(parentJointName);
//			if (parentId != c_InvalidIndex)
//			{
//				Matrix44 Mparent = pose->getJointGlobalTransform(&model, parentId).toMatrix44();
//				Mjoint = Mparent.inverse() * Mjoint;	// Cl = Bg-1 * Cg
//			}
//			else
//				log::warning << L"Unable to bind parent joint; no such joint \"" << parentJointName << L"\"." << Endl;
//		}
//
//		pose->setJointTransform(jointId, Transform(Mjoint));
//		return true;
//	});
//
//	return pose;
//}

}
