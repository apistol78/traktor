#include "Physics/Editor/MeshAsset.h"
#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.MeshAsset", 0, MeshAsset, editor::Asset)

const TypeInfo* MeshAsset::getOutputType() const
{
	return &type_of< MeshResource >();
}

	}
}
