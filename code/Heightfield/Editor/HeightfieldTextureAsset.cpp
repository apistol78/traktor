#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldTextureAsset", 0, HeightfieldTextureAsset, ISerializable)

HeightfieldTextureAsset::HeightfieldTextureAsset()
:	m_output(OtHeights)
,	m_scale(1.0f)
{
}

void HeightfieldTextureAsset::serialize(ISerializer& s)
{
	const MemberEnum< OutputType >::Key c_OutputType_Keys[] =
	{
		{ L"OtHeights", OtHeights },
		{ L"OtNormals", OtNormals },
		{ L"OtCuts", OtCuts },
		{ L"OtUnwrap", OtUnwrap },
		{ 0, 0 }
	};

	s >> resource::Member< Heightfield >(L"heightfield", m_heightfield);
	s >> MemberEnum< OutputType >(L"output", m_output, c_OutputType_Keys);
	s >> Member< float >(L"scale", m_scale);
}

	}
}
