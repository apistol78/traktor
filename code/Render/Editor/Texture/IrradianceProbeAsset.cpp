#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/IrradianceProbeAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.IrradianceProbeAsset", 0, IrradianceProbeAsset, editor::Asset)

IrradianceProbeAsset::IrradianceProbeAsset()
:	m_factor(1.0f)
{
}

void IrradianceProbeAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> Member< float >(L"factor", m_factor);
}

	}
}
