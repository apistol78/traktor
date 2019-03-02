#pragma once

#include <map>
#include <fbxsdk.h>

namespace traktor
{
	namespace model
	{

class Model;

bool convertMaterials(Model& outModel, std::map< int32_t, int32_t >& outMaterialMap, FbxNode* meshNode);

	}
}
