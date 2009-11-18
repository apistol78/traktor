#include "Terrain/Editor/MaterialMaskAsset.h"
#include "Terrain/MaterialMaskResource.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.MaterialMaskAsset", 0, MaterialMaskAsset, editor::Asset)

const TypeInfo* MaterialMaskAsset::getOutputType() const
{
	return &type_of< MaterialMaskResource >();
}

	}
}
