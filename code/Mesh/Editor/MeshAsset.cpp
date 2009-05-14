#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/MeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.MeshAsset", MeshAsset, editor::Asset)

MeshAsset::MeshAsset()
:	m_meshType(MtInvalid)
{
}

const Type* MeshAsset::getOutputType() const
{
	return &type_of< MeshResource >();
}

bool MeshAsset::serialize(Serializer& s)
{
	const MemberEnum< MeshType >::Key c_MeshType_Keys[] =
	{
		{ L"MtInvalid", MtInvalid },
		{ L"MtBlend", MtBlend },
		{ L"MtIndoor", MtIndoor },
		{ L"MtInstance", MtInstance },
		{ L"MtSkinned", MtSkinned },
		{ L"MtStatic", MtStatic },
		{ 0 }
	};

	if (!editor::Asset::serialize(s))
		return false;

	return s >> MemberEnum< MeshType >(L"meshType", m_meshType, c_MeshType_Keys);
}

	}
}
