/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Path.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Drawing/Image.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/MaterialConverter.h"

namespace traktor::model
{
	namespace
	{

//void scanCustomProperties(const FbxObject* node, Material& outMaterial)
//{
//	FbxProperty prop = node->GetFirstProperty();
//	while (prop.IsValid())
//	{
//		//const int userTag = prop.GetUserTag();
//		std::wstring propName = mbstows(prop.GetNameAsCStr());
//		if (startsWith(propName, L"Traktor_"))
//		{
//			propName = propName.substr(8);
//			if (propName == L"DoubleSided")
//			{
//				FbxPropertyT< FbxBool > propState = prop;
//				if (propState.IsValid())
//					outMaterial.setDoubleSided(propState.Get());
//			}
//			else if (propName == L"BlendMode")
//			{
//				FbxPropertyT< FbxInt32 > propMode = prop;
//				if (propMode.IsValid())
//					outMaterial.setBlendOperator((Material::BlendOperator)propMode.Get());
//			}
//			else if (propName == L"Transparency")
//			{
//				FbxPropertyT< FbxFloat > propTransparency = prop;
//				if (propTransparency.IsValid())
//					outMaterial.setTransparency(propTransparency.Get());
//			}
//			else if (propName == L"Emissive")
//				outMaterial.setEmissive(1.0f);
//			else
//			{
//				FbxPropertyT< FbxBool > propState = prop;
//				if (propState.IsValid())
//					outMaterial.setProperty< PropertyBoolean >(propName, propState.Get());
//			}
//
//			propName = replaceAll(propName, L"Traktor_", L"");
//			FbxPropertyT< FbxBool > propState = prop;
//			if (propState.IsValid())
//			{
//				bool propValue = propState.Get();
//				if (propName != L"DoubleSided")
//					outMaterial.setProperty< PropertyBoolean >(propName, propValue);
//				else
//					outMaterial.setDoubleSided(propValue);
//			}
//		}
//		prop = node->GetNextProperty(prop);
//	}
//}

//const FbxTexture* getTexture(const ufbx_material* material, const char* fbxPropertyName)
//{
//	if (!material || !fbxPropertyName)
//		return nullptr;

	//const FbxProperty prop = material->FindProperty(fbxPropertyName);
	//if (!prop.IsValid())
	//	return nullptr;

	//const int fileTextureCount = prop.GetSrcObjectCount< FbxFileTexture >();
	//for (int i = 0; i < fileTextureCount; ++i)
	//{
	//	FbxFileTexture* fileTexture = prop.GetSrcObject< FbxFileTexture >(i);
	//	if (fileTexture)
	//		return fileTexture;
	//}

	//const int layeredTextureCount = prop.GetSrcObjectCount< FbxLayeredTexture >();
	//if (layeredTextureCount)
	//{
	//	for (int i = 0; i < layeredTextureCount; ++i)
	//	{
	//		FbxLayeredTexture* layeredTexture = prop.GetSrcObject< FbxLayeredTexture >(i);
	//		if (layeredTexture)
	//			return layeredTexture;
	//	}
	//}

//	return nullptr;
//}

//std::wstring getTextureName(const FbxTexture* texture)
//{
//	const FbxFileTexture* fileTexture = FbxCast< const FbxFileTexture >(texture);
//	if (fileTexture)
//	{
//		const Path texturePath(mbstows(fileTexture->GetFileName()));
//		return texturePath.getFileNameNoExtension();
//	}
//	else
//		return std::wstring(mbstows(texture->GetName()));
//}
//
//Ref< drawing::Image > getEmbeddedTexture(const FbxTexture* texture)
//{
//	const FbxFileTexture* fileTexture = FbxCast< const FbxFileTexture >(texture);
//	if (fileTexture)
//	{
//		const Path texturePath(mbstows(fileTexture->GetFileName()));
//		return drawing::Image::load(texturePath);
//	}
//	else
//		return nullptr;
//}
//
//std::wstring uvChannel(Model& outModel, const std::string& name)
//{
//	return mbstows(name);
//}

	}

bool convertMaterials(Model& outModel, SmallMap< int32_t, int32_t >& outMaterialMap, ufbx_node* meshNode)
{
	//int32_t materialCount = meshNode->GetMaterialCount();
	for (size_t i = 0; i < meshNode->materials.count; ++i)
	{
		ufbx_material* material = meshNode->materials.data[i];
		if (!material)
			continue;

		T_FATAL_ASSERT(material->features.pbr.enabled);

		const std::wstring name = mbstows(material->name.data);

		// Check if material has already been added.
		const auto& materials = outModel.getMaterials();
		auto it = std::find_if(materials.begin(), materials.end(), [&](const Material& m) {
			return m.getName() == name;
		});
		if (it != materials.end())
		{
			outMaterialMap[i] = std::distance(materials.begin(), it);
			continue;
		}

		Material mm;
		mm.setName(name);

		if (material->fbx.diffuse_color.texture)
		{
			//Ref< drawing::Image > diffuseImage = getEmbeddedTexture(diffuseTexture);
			//mm.setDiffuseMap(Material::Map(
			//	getTextureName(diffuseTexture),
			//	uvChannel(outModel, diffuseTexture->UVSet.Get().Buffer()),
			//	true,
			//	Guid(),
			//	diffuseImage
			//));
		}

		if (material->fbx.specular_color.texture)
		{
			//Ref< drawing::Image > specularImage = getEmbeddedTexture(specularTexture);
			//mm.setSpecularMap(Material::Map(
			//	getTextureName(specularTexture),
			//	uvChannel(outModel, specularTexture->UVSet.Get().Buffer()),
			//	false,
			//	Guid(),
			//	specularImage
			//));
		}

		//const FbxTexture* shininessTexture = getTexture(material, FbxSurfaceMaterial::sShininess);
		//if (shininessTexture)
		//{
		//	Ref< drawing::Image > shininessImage = getEmbeddedTexture(shininessTexture);
		//	mm.setRoughnessMap(Material::Map(
		//		getTextureName(shininessTexture),
		//		uvChannel(outModel, shininessTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		shininessImage
		//	));
		//	mm.setRoughness(1.0f);
		//}

		//const FbxTexture* reflectionFactorTexture = getTexture(material, FbxSurfaceMaterial::sReflectionFactor);
		//if (reflectionFactorTexture)
		//{
		//	Ref< drawing::Image > reflectionFactorImage = getEmbeddedTexture(reflectionFactorTexture);
		//	mm.setMetalnessMap(Material::Map(
		//		getTextureName(reflectionFactorTexture),
		//		uvChannel(outModel, reflectionFactorTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		reflectionFactorImage
		//	));
		//	mm.setMetalness(1.0f);
		//}

		//const FbxTexture* normalTexture = getTexture(material, FbxSurfaceMaterial::sNormalMap);
		//if (normalTexture)
		//{
		//	Ref< drawing::Image > normalImage = getEmbeddedTexture(normalTexture);
		//	mm.setNormalMap(Material::Map(
		//		getTextureName(normalTexture),
		//		uvChannel(outModel, normalTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		normalImage
		//	));
		//}

		//const FbxTexture* transparencyTexture = getTexture(material, FbxSurfaceMaterial::sTransparentColor);
		//if (transparencyTexture)
		//{
		//	Ref< drawing::Image > transparencyImage = getEmbeddedTexture(transparencyTexture);
		//	mm.setTransparencyMap(Material::Map(
		//		getTextureName(transparencyTexture),
		//		uvChannel(outModel, transparencyTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		transparencyImage
		//	));
		//	mm.setBlendOperator(Material::BoAlpha);
		//}

		//const FbxTexture* transparencyFactorTexture = getTexture(material, FbxSurfaceMaterial::sTransparencyFactor);
		//if (transparencyFactorTexture)
		//{
		//	Ref< drawing::Image > transparencyFactorImage = getEmbeddedTexture(transparencyFactorTexture);
		//	mm.setTransparencyMap(Material::Map(
		//		getTextureName(transparencyFactorTexture),
		//		uvChannel(outModel, transparencyFactorTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		transparencyFactorImage
		//	));
		//	mm.setBlendOperator(Material::BoAlphaTest);
		//}

		//const FbxTexture* emissiveTexture = getTexture(material, /*mayaExported ? FbxSurfaceMaterial::sAmbient :*/ FbxSurfaceMaterial::sEmissive);
		//if (emissiveTexture)
		//{
		//	Ref< drawing::Image > emissiveImage = getEmbeddedTexture(emissiveTexture);
		//	mm.setEmissiveMap(Material::Map(
		//		getTextureName(emissiveTexture),
		//		uvChannel(outModel, emissiveTexture->UVSet.Get().Buffer()),
		//		false,
		//		Guid(),
		//		emissiveImage
		//	));
		//}

		mm.setColor(Color4f(
			material->pbr.base_color.value_vec3.x,
			material->pbr.base_color.value_vec3.y,
			material->pbr.base_color.value_vec3.z,
			1.0f
		));
		mm.setDiffuseTerm(material->pbr.base_factor.has_value ? material->pbr.base_factor.value_real : 1.0f);
		mm.setSpecularTerm(material->pbr.specular_factor.has_value ? material->pbr.specular_factor.value_real : 1.0f);

		//if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		//{
		//	FbxSurfacePhong* phongMaterial = (FbxSurfacePhong*)material;

		//	if (diffuseTexture == nullptr)
		//	{
		//		FbxPropertyT< FbxDouble3 > phongDiffuse = phongMaterial->Diffuse;
		//		if (phongDiffuse.IsValid())
		//		{
		//			FbxDouble3 diffuse = phongDiffuse.Get();
		//			mm.setColor(Color4f(
		//				(float)diffuse[0],
		//				(float)diffuse[1],
		//				(float)diffuse[2],
		//				1.0f
		//			));
		//		}
		//	}

		//	FbxPropertyT< FbxDouble > phongDiffuseFactor = phongMaterial->DiffuseFactor;
		//	if (phongDiffuseFactor.IsValid())
		//	{
		//		FbxDouble diffuseFactor = phongDiffuseFactor.Get();
		//		mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
		//	}

		//	FbxPropertyT< FbxDouble > phongSpecularFactor = phongMaterial->SpecularFactor;
		//	if (phongSpecularFactor.IsValid())
		//	{
		//		FbxDouble specularFactor = phongSpecularFactor.Get() * 2.0f;
		//		mm.setSpecularTerm(clamp(float(specularFactor), 0.0f, 1.0f));
		//	}

		//	// Do not modulate roughness in case a roughness map already exist.
		//	if (shininessTexture == nullptr)
		//	{
		//		FbxPropertyT< FbxDouble > phongShininess = phongMaterial->Shininess;
		//		if (phongShininess.IsValid())
		//		{
		//			FbxDouble shininess = phongShininess.Get();
		//			float roughness = std::pow(1.0f - shininess / 100.0f, 2.0f);
		//			mm.setRoughness(clamp(roughness, 0.0f, 1.0f));
		//		}
		//	}

		//	// Do not modulate metalness in case a roughness map already exist.
		//	if (reflectionFactorTexture == nullptr)
		//	{
		//		FbxPropertyT< FbxDouble > reflectionFactor = phongMaterial->ReflectionFactor;
		//		if (reflectionFactor.IsValid())
		//		{
		//			FbxDouble reflection = reflectionFactor.Get();
		//			mm.setMetalness(clamp(float(reflection), 0.0f, 1.0f));
		//		}
		//	}

		//	FbxPropertyT< FbxDouble3 > phongEmissive = /*mayaExported ? phongMaterial->Ambient :*/ phongMaterial->Emissive;
		//	if (phongEmissive.IsValid())
		//	{
		//		FbxDouble3 emissive = phongEmissive.Get();
		//		FbxDouble emissiveFactor = /*mayaExported ? phongMaterial->AmbientFactor.Get() :*/ phongMaterial->EmissiveFactor.Get();

		//		emissive[0] *= emissiveFactor;
		//		emissive[1] *= emissiveFactor;
		//		emissive[2] *= emissiveFactor;

		//		mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
		//	}
		//}
		//else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
		//{
		//	FbxSurfaceLambert* lambertMaterial = (FbxSurfaceLambert*)material;

		//	if (diffuseTexture == nullptr)
		//	{
		//		FbxPropertyT< FbxDouble3 > lambertDiffuse = lambertMaterial->Diffuse;
		//		if (lambertDiffuse.IsValid())
		//		{
		//			FbxDouble3 diffuse = lambertDiffuse.Get();
		//			mm.setColor(Color4f(
		//				(float)diffuse[0],
		//				(float)diffuse[1],
		//				(float)diffuse[2],
		//				1.0f
		//			));
		//		}
		//	}
		//	
		//	FbxPropertyT< FbxDouble > lambertDiffuseFactor = lambertMaterial->DiffuseFactor;
		//	if (lambertDiffuseFactor.IsValid())
		//	{
		//		FbxDouble diffuseFactor = lambertDiffuseFactor.Get();
		//		mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
		//	}

		//	FbxPropertyT< FbxDouble3 > lambertEmissive = /*mayaExported ? lambertMaterial->Ambient :*/ lambertMaterial->Emissive;
		//	if (lambertEmissive.IsValid())
		//	{
		//		FbxDouble3 emissive = lambertEmissive.Get();
		//		FbxDouble emissiveFactor = /*mayaExported ? lambertMaterial->AmbientFactor.Get() :*/ lambertMaterial->EmissiveFactor.Get();

		//		emissive[0] *= emissiveFactor;
		//		emissive[1] *= emissiveFactor;
		//		emissive[2] *= emissiveFactor;

		//		mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
		//	}

		//	mm.setSpecularTerm(0.0f);
		//}

		// Get custom properties on material.
		//scanCustomProperties(meshNode, mm);
		//scanCustomProperties(material, mm);

		outMaterialMap[i] = outModel.addUniqueMaterial(mm);
	}

	return true;
}

//void fixMaterialUvSets(Model& outModel)
//{
//	// Since FBX sometimes reference "default" UV set we need to patch
//	// this after everything has been extracted.
//
//	const auto& channels = outModel.getTexCoordChannels();
//	if (channels.empty())
//		return;
//
//	const std::wstring& channel = channels[0];
//
//	auto materials = outModel.getMaterials();
//	for (auto& material : materials)
//	{
//		{
//			auto map = material.getDiffuseMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setDiffuseMap(map);
//		}
//		{
//			auto map = material.getSpecularMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setSpecularMap(map);
//		}
//		{
//			auto map = material.getRoughnessMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setRoughnessMap(map);
//		}
//		{
//			auto map = material.getMetalnessMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setMetalnessMap(map);
//		}
//		{
//			auto map = material.getTransparencyMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setTransparencyMap(map);
//		}
//		{
//			auto map = material.getEmissiveMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setEmissiveMap(map);
//		}
//		{
//			auto map = material.getReflectiveMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setReflectiveMap(map);
//		}
//		{
//			auto map = material.getNormalMap();
//			if (map.channel == L"default")
//				map.channel = channel;
//			material.setNormalMap(map);
//		}
//	}
//	outModel.setMaterials(materials);
//}

}
