/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/GltfMaterialConverter.h"

#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Model/Model.h"

namespace traktor::model
{
namespace
{

std::wstring getTextureName(cgltf_texture* texture)
{
	if (texture->image)
	{
		if (texture->image->uri)
		{
			const Path texturePath(mbstows(texture->image->uri));
			return texturePath.getFileNameNoExtension();
		}
		else if (texture->image->name)
		{
			return mbstows(texture->image->name);
		}
	}
	if (texture->name)
		return mbstows(texture->name);

	return L"";
}

Ref< drawing::Image > getEmbeddedTexture(cgltf_texture* texture, const std::wstring& basePath)
{
	if (!texture->image)
		return nullptr;

	const cgltf_image* image = texture->image;

	if (image->buffer_view)
	{
		// Embedded texture in buffer
		const cgltf_buffer_view* bufferView = image->buffer_view;
		if (bufferView->buffer && bufferView->buffer->data)
		{
			const uint8_t* data = static_cast< const uint8_t* >(bufferView->buffer->data) + bufferView->offset;
			std::wstring mimeType = L"png";// image->mime_type ? mbstows(image->mime_type) : L"png";
			return drawing::Image::load(data, uint32_t(bufferView->size), mimeType);
		}
	}
	else if (image->uri)
	{
		// External texture file

		std::string uri = image->uri;

		// Handle data URIs
		if (uri.find("data:") == 0)
		{
			// Find the comma that separates metadata from data
			size_t commaPos = uri.find(',');
			if (commaPos != std::string::npos)
			{
				// Check if it's base64 encoded
				if (uri.find("base64") != std::string::npos)
				{
					// Extract the base64 data part
					std::string base64Data = uri.substr(commaPos + 1);

					// Calculate the expected output size
					size_t base64Size = base64Data.length();
					// Remove padding characters for size calculation
					while (!base64Data.empty() && base64Data.back() == '=')
					{
						base64Data.pop_back();
						base64Size--;
					}
					size_t numberOfEncodedBits = base64Size * 6 - (base64Size * 6) % 8;
					size_t outSize = numberOfEncodedBits / 8;

					// Use cgltf's base64 decoder
					void* decodedData = nullptr;
					cgltf_options options = {};
					cgltf_result result = cgltf_load_buffer_base64(&options, outSize, (uri.substr(commaPos + 1)).c_str(), &decodedData);

					if (result == cgltf_result_success && decodedData)
					{
						// Load image from decoded data
						Ref< drawing::Image > loadedImage = drawing::Image::load((const uint8_t*)decodedData, outSize, L"png");

						// Free the decoded data
						if (options.memory.free_func)
							options.memory.free_func(options.memory.user_data, decodedData);
						else
							free(decodedData);

						return loadedImage;
					}
					else
					{
						log::warning << L"Failed to decode base64 image data using cgltf." << Endl;
					}
				}
				else
				{
					log::warning << L"Non-base64 data URIs not supported." << Endl;
				}
			}
		}
		else
		{
			// External file
			const Path imagePath = Path(basePath) + Path(mbstows(uri));
			return drawing::Image::load(imagePath);
		}
	}

	return nullptr;
}

uint32_t getTextureCoordChannel(const cgltf_texture_view* textureView, Model& model)
{
	if (textureView->has_transform && textureView->transform.has_texcoord)
	{
		// Use the transform's texcoord if available
		std::wstring channelName = L"UV" + toString(textureView->transform.texcoord);
		return model.getTexCoordChannel(channelName);
	}
	else if (textureView->texcoord >= 0)
	{
		// Use the texture view's texcoord
		std::wstring channelName = L"UV" + toString(textureView->texcoord);
		return model.getTexCoordChannel(channelName);
	}
	else
	{
		// Default to TEXCOORD_0
		return model.getTexCoordChannel(L"UV0");
	}
}

}

bool convertMaterials(Model& outModel, SmallMap< int32_t, int32_t >& outMaterialMap, cgltf_data* data, cgltf_primitive* primitive, const std::wstring& basePath)
{
	if (!primitive->material)
	{
		// No material assigned, create a default one
		outMaterialMap[0] = c_InvalidIndex;
		return true;
	}

	cgltf_material* material = primitive->material;
	const std::wstring name = material->name ? mbstows(material->name) : L"default";

	// Check if material has already been added
	const auto& materials = outModel.getMaterials();
	auto it = std::find_if(materials.begin(), materials.end(), [&](const Material& m) {
		return m.getName() == name;
	});
	if (it != materials.end())
	{
		outMaterialMap[0] = std::distance(materials.begin(), it);
		return true;
	}

	Material mm;
	mm.setName(name);

	// Handle PBR metallic-roughness workflow (glTF standard)
	if (material->has_pbr_metallic_roughness)
	{
		const auto& pbr = material->pbr_metallic_roughness;

		// Base color
		mm.setColor(Color4f(
			pbr.base_color_factor[0],
			pbr.base_color_factor[1],
			pbr.base_color_factor[2],
			pbr.base_color_factor[3]));

		// Metalness and roughness
		mm.setMetalness(pbr.metallic_factor);
		mm.setRoughness(pbr.roughness_factor);

		// Base color texture (diffuse)
		if (pbr.base_color_texture.texture)
		{
			const uint32_t channel = getTextureCoordChannel(&pbr.base_color_texture, outModel);
			Ref< drawing::Image > diffuseImage = getEmbeddedTexture(pbr.base_color_texture.texture, basePath);
			mm.setDiffuseMap(Material::Map(
				getTextureName(pbr.base_color_texture.texture),
				channel,
				true,
				Guid(),
				diffuseImage));
		}

		// Metallic-roughness texture
		if (pbr.metallic_roughness_texture.texture)
		{
			const uint32_t channel = getTextureCoordChannel(&pbr.metallic_roughness_texture, outModel);
			Ref< drawing::Image > metallicRoughnessImage = getEmbeddedTexture(pbr.metallic_roughness_texture.texture, basePath);

			// In glTF, metallic is in B channel and roughness is in G channel
			mm.setMetalnessMap(Material::Map(
				getTextureName(pbr.metallic_roughness_texture.texture),
				channel,
				false,
				Guid(),
				metallicRoughnessImage));
			mm.setRoughnessMap(Material::Map(
				getTextureName(pbr.metallic_roughness_texture.texture),
				channel,
				false,
				Guid(),
				metallicRoughnessImage));
			mm.setMetalness(1.0f);
			mm.setRoughness(1.0f);
		}
	}

	// Normal map
	if (material->normal_texture.texture)
	{
		const uint32_t channel = getTextureCoordChannel(&material->normal_texture, outModel);
		Ref< drawing::Image > normalImage = getEmbeddedTexture(material->normal_texture.texture, basePath);
		mm.setNormalMap(Material::Map(
			getTextureName(material->normal_texture.texture),
			channel,
			false,
			Guid(),
			normalImage));
	}

	// Emissive
	if (material->emissive_texture.texture)
	{
		const uint32_t channel = getTextureCoordChannel(&material->emissive_texture, outModel);
		Ref< drawing::Image > emissiveImage = getEmbeddedTexture(material->emissive_texture.texture, basePath);
		mm.setEmissiveMap(Material::Map(
			getTextureName(material->emissive_texture.texture),
			channel,
			false,
			Guid(),
			emissiveImage));
	}

	// Emissive factor
	float emissiveFactor = std::max({ material->emissive_factor[0],
		material->emissive_factor[1],
		material->emissive_factor[2] });
	if (emissiveFactor > 0.0f)
		mm.setEmissive(emissiveFactor);

	// Occlusion map - currently Traktor doesn't have direct support, could map to a custom property
	// if (material->occlusion_texture.texture) { ... }

	// Alpha mode
	if (material->alpha_mode == cgltf_alpha_mode_blend)
	{
		mm.setBlendOperator(Material::BoAlpha);
		if (material->has_pbr_metallic_roughness)
			mm.setTransparency(material->pbr_metallic_roughness.base_color_factor[3]);
	}
	else if (material->alpha_mode == cgltf_alpha_mode_mask)
	{
		mm.setBlendOperator(Material::BoAlpha);
		// Alpha cutoff handled by material->alpha_cutoff
	}

	// Double-sided rendering
	mm.setDoubleSided(material->double_sided);

	outMaterialMap[0] = outModel.addUniqueMaterial(mm);
	return true;
}

}
