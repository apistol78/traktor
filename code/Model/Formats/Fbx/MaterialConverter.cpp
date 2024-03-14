/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
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

std::wstring getTextureName(const ufbx_texture* texture)
{
	if (texture->type == UFBX_TEXTURE_FILE)
	{
		const Path texturePath(mbstows(texture->filename.data));
		return texturePath.getFileNameNoExtension();
	}
	else
		return std::wstring(mbstows(texture->name.data));
}

Ref< drawing::Image > getEmbeddedTexture(const ufbx_texture* texture)
{
	if (texture->type == UFBX_TEXTURE_FILE)
	{
		if (texture->content.data != nullptr)
			return drawing::Image::load(texture->content.data, texture->content.size, L"png");
		else
		{
			const Path fileName(mbstows(texture->absolute_filename.data));
			return drawing::Image::load(fileName);
		}
	}
	else
		return nullptr;
}

	}

bool convertMaterials(Model& outModel, SmallMap< int32_t, int32_t >& outMaterialMap, ufbx_node* meshNode)
{
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

		if (material->pbr.base_color.has_value)
		{
			mm.setColor(Color4f(
				material->pbr.base_color.value_vec3.x,
				material->pbr.base_color.value_vec3.y,
				material->pbr.base_color.value_vec3.z,
				1.0f
			));
		}

		if (material->pbr.base_factor.has_value)
			mm.setDiffuseTerm(material->pbr.base_factor.value_real);

		if (material->pbr.glossiness.has_value)
		{
			const float sf = material->pbr.glossiness.value_real;
			mm.setSpecularTerm(clamp(sf, 0.0f, 1.0f));
		}

		if (material->pbr.specular_ior.has_value)
		{
			const float ior = material->pbr.specular_ior.value_real;
			const float sf = std::pow((ior - 1.0f) / (ior + 1.0f), 2.0f) / 0.08f;
			mm.setSpecularTerm(clamp(sf, 0.0f, 1.0f));
		}

		if (material->pbr.roughness.has_value)
			mm.setRoughness(material->pbr.roughness.value_real);

		if (material->pbr.metalness.has_value)
			mm.setMetalness(material->pbr.metalness.value_real);

		if (material->pbr.base_color.texture)
		{
			Ref< drawing::Image > diffuseImage = getEmbeddedTexture(material->pbr.base_color.texture);
			mm.setDiffuseMap(Material::Map(
				getTextureName(material->pbr.base_color.texture),
				mbstows(material->pbr.base_color.texture->uv_set.data),
				true,
				Guid(),
				diffuseImage
			));
		}

		if (material->pbr.specular_color.texture)
		{
			Ref< drawing::Image > specularImage = getEmbeddedTexture(material->pbr.specular_color.texture);
			mm.setSpecularMap(Material::Map(
				getTextureName(material->pbr.specular_color.texture),
				mbstows(material->pbr.specular_color.texture->uv_set.data),
				false,
				Guid(),
				specularImage
			));
		}

		if (material->pbr.roughness.texture)
		{
			Ref< drawing::Image > roughnessImage = getEmbeddedTexture(material->pbr.roughness.texture);
			mm.setRoughnessMap(Material::Map(
				getTextureName(material->pbr.roughness.texture),
				mbstows(material->pbr.roughness.texture->uv_set.data),
				false,
				Guid(),
				roughnessImage
			));
			mm.setRoughness(1.0f);
		}

		if (material->pbr.metalness.texture)
		{
			Ref< drawing::Image > metalnessImage = getEmbeddedTexture(material->pbr.metalness.texture);
			mm.setMetalnessMap(Material::Map(
				getTextureName(material->pbr.metalness.texture),
				mbstows(material->pbr.metalness.texture->uv_set.data),
				false,
				Guid(),
				metalnessImage
			));
			mm.setMetalness(1.0f);
		}

		if (material->pbr.normal_map.texture)
		{
			Ref< drawing::Image > normalImage = getEmbeddedTexture(material->pbr.normal_map.texture);
			mm.setNormalMap(Material::Map(
				getTextureName(material->pbr.normal_map.texture),
				mbstows(material->pbr.normal_map.texture->uv_set.data),
				false,
				Guid(),
				normalImage
			));
		}

		if (material->pbr.opacity.texture)
		{
			Ref< drawing::Image > transparencyImage = getEmbeddedTexture(material->pbr.opacity.texture);
			mm.setTransparencyMap(Material::Map(
				getTextureName(material->pbr.opacity.texture),
				mbstows(material->pbr.opacity.texture->uv_set.data),
				false,
				Guid(),
				transparencyImage
			));
			mm.setBlendOperator(Material::BoAlpha);
		}

		if (material->pbr.emission_color.texture)
		{
			Ref< drawing::Image > emissiveImage = getEmbeddedTexture(material->pbr.emission_color.texture);
			mm.setEmissiveMap(Material::Map(
				getTextureName(material->pbr.emission_color.texture),
				mbstows(material->pbr.emission_color.texture->uv_set.data),
				false,
				Guid(),
				emissiveImage
			));
		}

		// Get custom properties on material.
		//scanCustomProperties(meshNode, mm);
		//scanCustomProperties(material, mm);

		outMaterialMap[i] = outModel.addUniqueMaterial(mm);
	}

	return true;
}

}
