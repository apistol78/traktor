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

FbxMatrix getGeometricTransform(const FbxNode* fbxNode)
{
	FbxVector4 t = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxMatrix(t, r, s);
}

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

		}

bool convertSkeleton(
	Model& outModel,
	FbxScene* scene,
	FbxNode* meshNode,
	const Matrix44& axisTransform
)
{
	FbxMesh* mesh = static_cast< FbxMesh* >(meshNode->GetNodeAttribute());
	if (!mesh)
		return false;

	int32_t deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int32_t i = 0; i < deformerCount; ++i)
	{
		FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
		if (!skinDeformer)
			continue;

		int32_t clusterCount = skinDeformer->GetClusterCount();
		for (int32_t j = 0; j < clusterCount; ++j)
		{
			FbxCluster* cluster = skinDeformer->GetCluster(j);
			if (!cluster)
				continue;

			const FbxNode* jointNode = cluster->GetLink();
			T_ASSERT (jointNode);

			std::wstring jointName = mbstows(jointNode->GetName());

			size_t p = jointName.find(L':');
			if (p != std::wstring::npos)
				jointName = jointName.substr(p + 1);

			if (outModel.findJointIndex(jointName) != c_InvalidIndex)
				continue;

			FbxAMatrix jointRestTransform;
			cluster->GetTransformLinkMatrix(jointRestTransform);

			Matrix44 Mnode = convertMatrix(jointRestTransform);
			Matrix44 Mrx90 = rotateX(deg2rad(-90.0f));
			Matrix44 Mjoint = axisTransform * (Mnode * Mrx90) * axisTransform.inverse();

			Transform Tjoint(Mjoint);

			// Normalize transformation in case scaling involved.
			Tjoint = Transform(
				Tjoint.translation(),
				Tjoint.rotation().normalized()
			);

			Joint joint;
			joint.setName(jointName);
			joint.setTransform(Tjoint);
			outModel.addJoint(joint);	
		}
	}

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
			return true;

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

		Transform TjointRest = joint.getTransform();
		Transform TjointDelta = Tjoint * TjointRest.inverse();

		pose->setJointTransform(jointId, TjointDelta);

		return true;
	});
	
	return pose;
}

	}
}
