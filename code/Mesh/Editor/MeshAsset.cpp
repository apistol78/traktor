#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Editor/MeshAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshAsset", 12, MeshAsset, editor::Asset)

MeshAsset::MeshAsset()
:	m_meshType(MtInvalid)
,	m_scaleFactor(1.0f)
,	m_bakeOcclusion(false)
,	m_cullDistantFaces(false)
,	m_lodSteps(8)
,	m_lodMaxDistance(100.0f)
,	m_lodCullDistance(200.0f)
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
		{ L"MtLod", MtLod },
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

	if (s.getVersion() >= 11)
		s >> Member< float >(L"scaleFactor", m_scaleFactor);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"bakeOcclusion", m_bakeOcclusion);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"cullDistantFaces", m_cullDistantFaces);

	if (s.getVersion() >= 5 && s.getVersion() < 12)
	{
		bool generateOccluder = false;
		s >> Member< bool >(L"generateOccluder", generateOccluder);
	}

	if (s.getVersion() >= 10)
	{
		s >> Member< int32_t >(L"lodSteps", m_lodSteps);
		s >> Member< float >(L"lodMaxDistance", m_lodMaxDistance);
		s >> Member< float >(L"lodCullDistance", m_lodCullDistance);
	}

	if (s.getVersion() >= 8 && s.getVersion() < 12)
	{
		Path occluderModel;
		s >> Member< Path >(L"occluderModel", occluderModel);
	}

	if (s.getVersion() >= 7 && s.getVersion() <= 8)
	{
		float autoDetailLevel = 0.0f;
		s >> Member< float >(L"autoDetailLevel", autoDetailLevel, AttributeRange(0.0f, 1.0f));
	}
}

	}
}
