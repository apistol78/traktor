#include <functional>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/SkeletonConverter.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

bool traverse(FbxNode* parent, FbxNode* node, const std::function< bool (FbxNode* parent, FbxNode* node) >& visitor)
{
	if (!node)
		return true;

	if (!visitor(parent, node))
		return false;

	int32_t childCount = node->GetChildCount();
	for (int32_t i = 0; i < childCount; ++i)
	{
		FbxNode* childNode = node->GetChild(i);
		if (childNode)
		{
			if (!traverse(node, childNode, visitor))
				return false;
		}
	}

	return true;
}

std::wstring getJointName(FbxNode* node)
{
	std::wstring jointName = mbstows(node->GetName());

	size_t p = jointName.find(L':');
	if (p != std::wstring::npos)
		jointName = jointName.substr(p + 1);

	return jointName;
}

		}

bool convertSkeleton(
	Model& outModel,
	FbxScene* scene,
	FbxNode* skeletonNode,
	const Matrix44& axisTransform
)
{
	FbxSkeleton* skeleton = skeletonNode->GetSkeleton();
	if (!skeleton)
		return false;

	// Find bind pose.
	FbxPose* bindPose = nullptr;
	int32_t poseCount = scene->GetPoseCount();
    for (int32_t i = 0; i < poseCount; i++)
    {
        FbxPose* pose = scene->GetPose(i);
		if (pose->IsBindPose())
		{
			bindPose = pose;
			break;
		}
	}
	if (!bindPose)
		return false;

	bool result = traverse(nullptr, skeletonNode, [&](FbxNode* parent, FbxNode* node) {
		std::wstring jointName = getJointName(node);

		// Calculate joint transformation.
		FbxMatrix nodeTransform;

		int32_t id = bindPose->Find(node);
		if (id >= 0)
			nodeTransform = bindPose->GetMatrix(id);
		else
		{
			id = bindPose->Find(parent);
			if (id < 0)
			{
				log::error << L"Parent of \"" << jointName << L"\" doesn't have a bind pose matrix, cannot synthesize bind pose matrix." << Endl;
				return true;
			}
			nodeTransform = bindPose->GetMatrix(id) * node->EvaluateLocalTransform();
		}
		
		Matrix44 Mnode = convertMatrix(nodeTransform);
		Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));
		Matrix44 Mjoint = axisTransform * (Mnode * Mrx90) * axisTransform.inverse();

		const Vector4 S(
			1.0f / Mjoint.axisX().length(),
			1.0f / Mjoint.axisY().length(),
			1.0f / Mjoint.axisZ().length()
		);
		Transform Tjoint(Mjoint * scale(S));

		// Normalize transformation in case scaling involved.
		Tjoint = Transform(
			Tjoint.translation(),
			Tjoint.rotation().normalized()
		);

		uint32_t parentId = c_InvalidIndex;
		if (parent != nullptr)
		{
			std::wstring parentJointName = getJointName(parent);
			parentId = outModel.findJointIndex(parentJointName);
			if (parentId != c_InvalidIndex)
			{
				Transform Tparent = outModel.getJointGlobalTransform(parentId);
				Tjoint = Tparent.inverse() * Tjoint;	// Cl = Bg-1 * Cg
			}
			else
				log::warning << L"Unable to bind parent joint; no such joint \"" << parentJointName << L"\"." << Endl;
		}

		Joint joint;
		joint.setParent(parentId);
		joint.setName(jointName);
		joint.setTransform(Tjoint);
		outModel.addJoint(joint);	
		return true;
	});	

	return true;
}

Ref< Pose > convertPose(
	const Model& model,
	FbxScene* scene,
	FbxNode* skeletonNode,
	const FbxTime& time,
	const Matrix44& axisTransform
)
{
	FbxSkeleton* skeleton = skeletonNode->GetSkeleton();
	if (!skeleton)
		return nullptr;

	Ref< Pose > pose = new Pose();

	bool result = traverse(nullptr, skeletonNode, [&](FbxNode* parent, FbxNode* node) {
		FbxAMatrix nodeTransform = node->EvaluateGlobalTransform(time);

		Matrix44 Mnode = convertMatrix(nodeTransform);
		Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));
		Matrix44 Mjoint = axisTransform * (Mnode * Mrx90) * axisTransform.inverse();

		std::wstring jointName = mbstows(node->GetName());

		size_t p = jointName.find(L':');
		if (p != std::wstring::npos)
			jointName = jointName.substr(p + 1);

		uint32_t jointId = model.findJointIndex(jointName);
		if (jointId == c_InvalidIndex)
		{
			log::warning << L"Unable to find joint \"" << jointName << L"\" in skeleton; unable to save pose for joint." << Endl;
			return true;
		}

		const Joint& joint = model.getJoint(jointId);

		const Vector4 S(
			1.0f / Mjoint.axisX().length(),
			1.0f / Mjoint.axisY().length(),
			1.0f / Mjoint.axisZ().length()
		);
		Transform Tjoint(Mjoint * scale(S));

		// Normalize transformation in case scaling involved.
		Tjoint = Transform(
			Tjoint.translation(),
			Tjoint.rotation().normalized()
		);

		// Calculate pose delta transformation matrix.
		Transform Tglobal = Transform::identity();
		for (
			uint32_t parentId = joint.getParent();
			parentId != c_InvalidIndex;
			parentId = model.getJoint(parentId).getParent()
		)
		{
			Tglobal = model.getJoint(parentId).getTransform() * pose->getJointTransform(parentId) * Tglobal;	// ABC order (A root)
		}
		
		Tglobal = Tglobal * joint.getTransform();
		Tglobal = Tglobal.inverse();
		Tglobal = Tglobal * Tjoint;

		pose->setJointTransform(jointId, Tglobal);
		return true;
	});
	
	return pose;
}

	}
}
