/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureOutput", 20, TextureOutput, ISerializable)

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
			{ L"TfBC6HU", TfBC6HU },
			{ L"TfBC6HS", TfBC6HS },
			{ L"TfPVRTC1", TfPVRTC1 },
			{ L"TfPVRTC2", TfPVRTC2 },
			{ L"TfPVRTC3", TfPVRTC3 },
			{ L"TfPVRTC4", TfPVRTC4 },
			{ L"TfETC1", TfETC1 },
			{ L"TfASTC4x4", TfASTC4x4 },
			{ L"TfASTC8x8", TfASTC8x8 },
			{ L"TfASTC10x10", TfASTC10x10 },
			{ L"TfASTC12x12", TfASTC12x12 },
			{ L"TfASTC4x4F", TfASTC4x4F },
			{ L"TfASTC8x8F", TfASTC8x8F },
			{ L"TfASTC10x10F", TfASTC10x10F },
			{ L"TfASTC12x12F", TfASTC12x12F },
			{ 0 }
		};
		s >> MemberEnum< TextureFormat >(L"textureFormat", m_textureFormat, c_TextureFormat_Keys);
	}

	if (s.getVersion() >= 20)
		s >> Member< bool >(L"normalMap", m_normalMap);

	if (s.getVersion() < 20)
		s >> ObsoleteMember< bool >(L"generateNormalMap");

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

	if (s.getVersion() >= 13)
		s >> Member< bool >(L"generateAlpha", m_generateAlpha);

	if (s.getVersion() >= 12)
		s >> Member< bool >(L"invertAlpha", m_invertAlpha);

	s >> Member< bool >(L"ignoreAlpha", m_ignoreAlpha);

	if (s.getVersion() >= 7)
		s >> Member< bool >(L"premultiplyAlpha", m_premultiplyAlpha);

	if (s.getVersion() >= 18)
		s >> Member< bool >(L"dilateImage", m_dilateImage);

	s >> Member< bool >(L"scaleImage", m_scaleImage);
	s >> Member< int32_t >(L"scaleWidth", m_scaleWidth, AttributeRange(0));
	s >> Member< int32_t >(L"scaleHeight", m_scaleHeight, AttributeRange(0));

	if (s.getVersion() >= 9)
	{
		s >> Member< bool >(L"flipX", m_flipX);
		s >> Member< bool >(L"flipY", m_flipY);
	}

	s >> Member< bool >(L"enableCompression", m_enableCompression);

	if (s.getVersion() >= 2 && s.getVersion() < 20)
	{
		bool enableNormalMapCompression;
		s >> Member< bool >(L"enableNormalMapCompression", enableNormalMapCompression);
		m_normalMap = enableNormalMapCompression;
	}

	if (s.getVersion() >= 16)
		s >> Member< bool >(L"encodeAsRGBM", m_encodeAsRGBM);

	if (s.getVersion() >= 15)
		s >> Member< bool >(L"inverseNormalMapX", m_inverseNormalMapX);
	if (s.getVersion() >= 3)
		s >> Member< bool >(L"inverseNormalMapY", m_inverseNormalMapY);

	if (s.getVersion() >= 17)
		s >> Member< float >(L"scaleNormalMap", m_scaleNormalMap, AttributeRange(-1.0f, 10.0f));

	if (s.getVersion() >= 19)
		s >> Member< bool >(L"assumeLinearGamma", m_assumeLinearGamma);
	else
		s >> Member< bool >(L"linearGamma", m_assumeLinearGamma);

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

	if (s.getVersion() >= 14)
		s >> Member< float >(L"noiseStrength", m_noiseStrength, AttributeRange(0.0f, 1.0f));

	if (s.getVersion() >= 10)
		s >> Member< bool >(L"systemTexture", m_systemTexture);
}

}
