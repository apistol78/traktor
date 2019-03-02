#pragma once

#include <fbxsdk.h>
#include "Core/Ref.h"

namespace traktor
{

class Matrix44;

	namespace model
	{

class Model;
class Pose;

bool convertSkeleton(
	Model& outModel,
	FbxScene* scene,
	FbxNode* skeletonNode,
	const Matrix44& axisTransform
);

Ref< Pose > convertPose(
	const Model& model,
	FbxScene* scene,
	FbxNode* skeletonNode,
	const FbxTime& time,
	const Matrix44& axisTransform
);

	}
}
