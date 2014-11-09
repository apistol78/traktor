#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureOutput", 11, TextureOutput, ISerializable)

TextureOutput::TextureOutput()
:	m_textureFormat(TfInvalid)
,	m_generateNormalMap(false)
,	m_scaleDepth(0.0f)
,	m_generateMips(true)
,	m_keepZeroAlpha(true)
,	m_textureType(Tt2D)
,	m_hasAlpha(false)
,	m_ignoreAlpha(false)
,	m_premultiplyAlpha(false)
,	m_scaleImage(false)
,	m_scaleWidth(0)
,	m_scaleHeight(0)
,	m_flipX(false)
,	m_flipY(false)
,	m_enableCompression(true)
,	m_enableNormalMapCompression(false)
,	m_inverseNormalMapY(false)
,	m_linearGamma(false)
,	m_generateSphereMap(false)
,	m_preserveAlphaCoverage(false)
,	m_alphaCoverageReference(0.5f)
,	m_sharpenRadius(5)
,	m_sharpenStrength(1.0f)
,	m_systemTexture(false)
{
}

void TextureOutput::serialize(ISerializer& s)
{
	if (s.getVersion() >= 6)
	{
		const MemberEnum< TextureFormat >::Key c_TextureFormat_Keys[] =
		{
			{ L"TfInvalid", TfInvalid },
			{ L"TfR8", TfR8 },
			{ L"TfR8G8B8A8", TfR8G8B8A8 },
			{ L"TfR5G6B5", TfR5G6B5 },
			{ L"TfR5G5B5A1", TfR5G5B5A1 },
			{ L"TfR4G4B4A4", TfR4G4B4A4 },
			{ L"TfR16G16B16A16F", TfR16G16B16A16F },
			{ L"TfR32G32B32A32F", TfR32G32B32A32F },
			{ L"TfR16G16F", TfR16G16F },
			{ L"TfR32G32F", TfR32G32F },
			{ L"TfR16F", TfR16F },
			{ L"TfR32F", TfR32F },
			{ L"TfR11G11B10F", TfR11G11B10F },
			{ L"TfDXT1", TfDXT1 },
			{ L"TfDXT2", TfDXT2 },
			{ L"TfDXT3", TfDXT3 },
			{ L"TfDXT4", TfDXT4 },
			{ L"TfDXT5", TfDXT5 },
			{ L"TfPVRTC1", TfPVRTC1 },
			{ L"TfPVRTC2", TfPVRTC2 },
			{ L"TfPVRTC3", TfPVRTC3 },
			{ L"TfPVRTC4", TfPVRTC4 },
			{ L"TfETC1", TfETC1 },
			{ 0 }
		};
		s >> MemberEnum< TextureFormat >(L"textureFormat", m_textureFormat, c_TextureFormat_Keys);
	}

	s >> Member< bool >(L"generateNormalMap", m_generateNormalMap);
	s >> Member< float >(L"scaleDepth", m_scaleDepth, AttributeRange(0.0f));
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> Member< bool >(L"keepZeroAlpha", m_keepZeroAlpha);

	if (s.getVersion() >= 8)
	{
		const MemberEnum< TextureType >::Key c_TextureType_Keys[] =
		{
			{ L"Tt2D", Tt2D },
			{ L"Tt3D", Tt3D },
			{ L"TtCube", TtCube },
			{ 0 }
		};
		s >> MemberEnum< TextureType >(L"textureType", m_textureType, c_TextureType_Keys);
	}
	else
	{
		bool isCubeMap = false;
		s >> Member< bool >(L"isCubeMap", isCubeMap);
		m_textureType = isCubeMap ? TtCube : Tt2D;
	}

	s >> Member< bool >(L"hasAlpha", m_hasAlpha);
	s >> Member< bool >(L"ignoreAlpha", m_ignoreAlpha);

	if (s.getVersion() >= 7)
		s >> Member< bool >(L"premultiplyAlpha", m_premultiplyAlpha);

	s >> Member< bool >(L"scaleImage", m_scaleImage);
	s >> Member< int32_t >(L"scaleWidth", m_scaleWidth, AttributeRange(0));
	s >> Member< int32_t >(L"scaleHeight", m_scaleHeight, AttributeRange(0));

	if (s.getVersion() >= 9)
	{
		s >> Member< bool >(L"flipX", m_flipX);
		s >> Member< bool >(L"flipY", m_flipY);
	}

	s >> Member< bool >(L"enableCompression", m_enableCompression);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"enableNormalMapCompression", m_enableNormalMapCompression);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"inverseNormalMapY", m_inverseNormalMapY);

	s >> Member< bool >(L"linearGamma", m_linearGamma);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"generateSphereMap", m_generateSphereMap);

	if (s.getVersion() >= 5)
	{
		s >> Member< bool >(L"preserveAlphaCoverage", m_preserveAlphaCoverage);
		s >> Member< float >(L"alphaCoverageReference", m_alphaCoverageReference, AttributeRange(0.0f, 1.0f));
	}

	if (s.getVersion() >= 11)
	{
		s >> Member< int32_t >(L"sharpenRadius", m_sharpenRadius);
		s >> Member< float >(L"sharpenStrength", m_sharpenStrength, AttributeRange(0.0f));
	}

	if (s.getVersion() >= 10)
		s >> Member< bool >(L"systemTexture", m_systemTexture);
}

	}
}
