#pragma once

#include <fbxsdk.h>

namespace traktor
{

class Matrix44;

	namespace model
	{

class Model;

bool convertMesh(
	Model& outModel,
	FbxScene* scene,
	FbxNode* meshNode,
	const Matrix44& axisTransform
);

	}
}
