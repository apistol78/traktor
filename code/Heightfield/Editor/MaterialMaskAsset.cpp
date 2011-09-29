#include "Heightfield/MaterialMaskResource.h"
#include "Heightfield/Editor/MaterialMaskAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.MaterialMaskAsset", 0, MaterialMaskAsset, editor::Asset)

const TypeInfo* MaterialMaskAsset::getOutputType() const
{
	return &type_of< MaterialMaskResource >();
}

	}
}
