#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/Editor/MaterialMaskAsset.h"
#include "Heightfield/Editor/MaterialMaskAssetLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.MaterialMaskAsset", 1, MaterialMaskAsset, editor::Asset)

const RefArray< MaterialMaskAssetLayer >& MaterialMaskAsset::getLayers() const
{
	return m_layers;
}

bool MaterialMaskAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	if (s.getVersion() >= 1)
		s >> MemberRefArray< MaterialMaskAssetLayer >(L"layers", m_layers);

	return true;
}

	}
}
