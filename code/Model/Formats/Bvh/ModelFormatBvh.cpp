/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/String.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Formats/Bvh/BvhDocument.h"
#include "Model/Formats/Bvh/BvhJoint.h"
#include "Model/Formats/Bvh/ModelFormatBvh.h"

namespace traktor::model
{
	namespace
	{

std::wstring getJointName(const BvhJoint* bvhJoint)
{
	std::wstring jointName = bvhJoint->getName();

	const size_t p = jointName.find(L':');
	if (p != std::wstring::npos)
		jointName = jointName.substr(p + 1);

	if (jointName.empty() && bvhJoint->getParent() != nullptr)
		jointName = getJointName(bvhJoint->getParent()) + L"_END";

	return jointName;
}

void createJoints(
	Model* model,
	const BvhJoint* bvhJoint,
	uint32_t parent
)
{
	const std::wstring jointName = getJointName(bvhJoint);

	Joint joint;
	joint.setParent(parent);
	joint.setName(jointName);
	joint.setTransform(Transform(
		bvhJoint->getOffset() * Vector4(1.0f, 1.0f, -1.0f, 1.0f)
	));

	const int32_t jointIndex = model->addJoint(joint);

	for (const auto childBvhJoint : bvhJoint->getChildren())
		createJoints(model, childBvhJoint, jointIndex);
}

bool convertKeyPose(
	Pose* outPose,
	const Model* model,
	const BvhJoint* bvhJoint,
	const BvhDocument::cv_t& cv
)
{
	const std::wstring jointName = getJointName(bvhJoint);

	const uint32_t jointId = model->findJointIndex(jointName);
	if (jointId == c_InvalidIndex)
	{
		log::error << L"Unable to find joint \"" << jointName << L"\" in skeleton when parsing pose." << Endl;
		return false;
	}

	Vector4 P = bvhJoint->getOffset();
	Matrix44 R = Matrix44::identity();

	int32_t offset = bvhJoint->getChannelOffset();
	for (const auto& channel : bvhJoint->getChannels())
	{
		float c = cv[offset++];

		if (channel == L"Xposition")
			P.set(0, Scalar(c));
		else if (channel == L"Yposition")
			P.set(1, Scalar(c));
		else if (channel == L"Zposition")
			P.set(2, Scalar(c));

		else if (channel == L"Xrotation")
			R = R * rotateX(-deg2rad(c));
		else if (channel == L"Yrotation")
			R = R * rotateY(-deg2rad(c));
		else if (channel == L"Zrotation")
			R = R * rotateZ(deg2rad(c));
	}

	if (bvhJoint->getParent() == nullptr)
		P = bvhJoint->getOffset();

	const Transform Tpose(
		P * Vector4(1.0f, 1.0f, -1.0f, 1.0f),
		Quaternion(R)
	);

	outPose->setJointTransform(
		jointId,
		Tpose
	);

	for (const auto childBvhJoint : bvhJoint->getChildren())
	{
		if (!convertKeyPose(
			outPose,
			model,
			childBvhJoint,
			cv
		))
			return false;
	}

	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatBvh", 0, ModelFormatBvh, ModelFormat)

void ModelFormatBvh::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"BVH";
	outExtensions.push_back(L"bvh");
}

bool ModelFormatBvh::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"bvh") == 0;
}

Ref< Model > ModelFormatBvh::read(const Path& filePath, const std::wstring& filter) const
{
	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return nullptr;

	Ref< BvhDocument > document = BvhDocument::parse(stream);
	if (!document)
		return nullptr;

	Ref< Model > model = new Model();

	createJoints(
		model,
		document->getRootJoint(),
		c_InvalidIndex
	);

	Ref< Animation > anim = new Animation();
	anim->setName(L"Animation");

	float at = 0.0f;
	for (const auto& channelValues : document->getChannelValues())
	{
		Ref< Pose > pose = new Pose();

		convertKeyPose(
			pose,
			model,
			document->getRootJoint(),
			channelValues
		);

		anim->insertKeyFrame(at, pose);
		at += document->getFrameTime();
	}

	model->addAnimation(anim);

	return model;
}

bool ModelFormatBvh::write(const Path& filePath, const Model* model) const
{
	return false;
}

}
