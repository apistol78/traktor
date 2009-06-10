#include "Terrain/Editor/MaterialMaskAsset.h"
#include "Terrain/MaterialMaskResource.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.terrain.MaterialMaskAsset", MaterialMaskAsset, editor::Asset)

const Type* MaterialMaskAsset::getOutputType() const
{
	return &type_of< MaterialMaskResource >();
}

	}
}
