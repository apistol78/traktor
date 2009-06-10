#include "Physics/Editor/MeshAsset.h"
#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.physics.MeshAsset", MeshAsset, editor::Asset)

const Type* MeshAsset::getOutputType() const
{
	return &type_of< MeshResource >();
}

	}
}
