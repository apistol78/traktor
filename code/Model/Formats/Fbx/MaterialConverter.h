#pragma once

#include <fbxsdk.h>

namespace traktor
{
	namespace model
	{

class Model;

bool convertMaterials(Model& outModel, FbxNode* meshNode);

	}
}