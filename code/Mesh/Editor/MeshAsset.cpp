#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Editor/MeshAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshAsset", 6, MeshAsset, editor::Asset)

MeshAsset::MeshAsset()
:	m_meshType(MtInvalid)
,	m_bakeOcclusion(false)
,	m_cullDistantFaces(false)
,	m_generateOccluder(false)
{
}

void MeshAsset::serialize(ISerializer& s)
{
	const MemberEnum< MeshType >::Key c_MeshType_Keys[] =
	{
		{ L"MtInvalid", MtInvalid },
		{ L"MtBlend", MtBlend },
		{ L"MtIndoor", MtIndoor },
		{ L"MtInstance", MtInstance },
		{ L"MtPartition", MtPartition },
		{ L"MtSkinned", MtSkinned },
		{ L"MtStatic", MtStatic },
		{ L"MtStream", MtStream },
		{ 0 }
	};

	editor::Asset::serialize(s);

	s >> MemberEnum< MeshType >(L"meshType", m_meshType, c_MeshType_Keys);

	if (s.getVersion() >= 6)
		s >> MemberStlMap< std::wstring, Guid >(L"materialTemplates", m_materialTemplates);

	if (s.getVersion() >= 1)
		s >> MemberStlMap< std::wstring, Guid >(L"materialShaders", m_materialShaders);

	if (s.getVersion() >= 4)
		s >> MemberStlMap< std::wstring, Guid >(L"materialTextures", m_materialTextures);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"bakeOcclusion", m_bakeOcclusion);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"cullDistantFaces", m_cullDistantFaces);

	if (s.getVersion() >= 5)
		s >> Member< bool >(L"generateOccluder", m_generateOccluder);
}

	}
}
