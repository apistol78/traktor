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
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Image.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Model.h"

namespace traktor::model
{
namespace
{

std::wstring getTextureName(const cgltf_texture* texture, const std::wstring& defaultName)
{
	if (texture->name)
		return mbstows(texture->name);
	else if (texture->image && texture->image->name)
		return mbstows(texture->image->name);
	else if (texture->image && texture->image->uri)
	{
		const Path texturePath(mbstows(texture->image->uri));
		return texturePath.getFileNameNoExtension();
	}
	else
		return defaultName;
}

Ref< drawing::Image > loadTextureImage(const cgltf_texture* texture, const Path& basePath)
{
	if (!texture || !texture->image)
		return nullptr;

	const cgltf_image* image = texture->image;

	// Try to load from buffer view (embedded)
	if (image->buffer_view)
	{
		const cgltf_buffer_view* bufferView = image->buffer_view;
		if (bufferView->buffer && bufferView->buffer->data)
		{
			const uint8_t* data = (const uint8_t*)bufferView->buffer->data + bufferView->offset;
			return drawing::Image::load(data, bufferView->size, L"png");
		}
	}

	// Try to load from URI
	if (image->uri)
	{
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

					// Use cgltf's base64 decoder (more reliable)
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
			const Path imagePath = basePath + Path(mbstows(uri));
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

bool convertMaterials(
	Model& outModel,
	SmallMap< cgltf_size, int32_t >& outMaterialMap,
	const cgltf_data* data,
	const Path& filePath)
{
	const Path basePath = filePath.getPathOnly();

	// Setup default texture coordinate channel
	const uint32_t defaultChannel = outModel.addUniqueTexCoordChannel(L"UV0");

	for (cgltf_size i = 0; i < data->materials_count; ++i)
	{
		const cgltf_material* gltfMaterial = &data->materials[i];

		Material material;

		// Set material name
		if (gltfMaterial->name)
			material.setName(mbstows(gltfMaterial->name));
		else
			material.setName(L"Material_" + toString(i));

		// Handle PBR Metallic Roughness workflow (default)
		if (gltfMaterial->has_pbr_metallic_roughness)
		{
			const cgltf_pbr_metallic_roughness* pbr = &gltfMaterial->pbr_metallic_roughness;

			// Base color factor
			material.setColor(convertColor(pbr->base_color_factor, 4));

			// Metallic and roughness factors
			material.setMetalness(pbr->metallic_factor);
			material.setRoughness(pbr->roughness_factor);

			// Base color texture (diffuse)
			if (pbr->base_color_texture.texture)
			{
				const std::wstring textureName = getTextureName(pbr->base_color_texture.texture, L"base_color_texture");
				const uint32_t channel = getTextureCoordChannel(&pbr->base_color_texture, outModel);
				Ref< drawing::Image > image = loadTextureImage(pbr->base_color_texture.texture, basePath);

				material.setDiffuseMap(Material::Map(textureName, channel, true, Guid(), image));
			}

			// Metallic-roughness texture
			if (pbr->metallic_roughness_texture.texture)
			{
				const std::wstring textureName = getTextureName(pbr->metallic_roughness_texture.texture, L"metallic_roughness_texture");
				const uint32_t channel = getTextureCoordChannel(&pbr->metallic_roughness_texture, outModel);
				Ref< drawing::Image > metallicRoughnessImage = loadTextureImage(pbr->metallic_roughness_texture.texture, basePath);

				if (metallicRoughnessImage)
				{
					// GLTF stores metallic in B channel, roughness in G channel
					const drawing::SwizzleFilter bFilter(L"BBBB");
					const drawing::SwizzleFilter gFilter(L"GGGG");

					// Extract metallic (blue channel)
					Ref< drawing::Image > metallicImage = metallicRoughnessImage->clone();
					metallicImage->apply(&bFilter);
					metallicImage->convert(drawing::PixelFormat::getR8());
					material.setMetalnessMap(Material::Map(textureName + L"_M", channel, false, Guid(), metallicImage));

					// Extract roughness (green channel)
					Ref< drawing::Image > roughnessImage = metallicRoughnessImage->clone();
					roughnessImage->apply(&gFilter);
					roughnessImage->convert(drawing::PixelFormat::getR8());
					material.setRoughnessMap(Material::Map(textureName + L"_R", channel, false, Guid(), roughnessImage));

					// When textures are present, use them at full strength
					material.setMetalness(1.0f);
					material.setRoughness(1.0f);
				}
			}
		}

		// Normal texture
		if (gltfMaterial->normal_texture.texture)
		{
			const std::wstring textureName = getTextureName(gltfMaterial->normal_texture.texture, L"normal_texture");
			const uint32_t channel = getTextureCoordChannel(&gltfMaterial->normal_texture, outModel);
			Ref< drawing::Image > image = loadTextureImage(gltfMaterial->normal_texture.texture, basePath);

			material.setNormalMap(Material::Map(textureName, channel, false, Guid(), image));
		}

		// Occlusion texture
		if (gltfMaterial->occlusion_texture.texture)
		{
			const std::wstring textureName = getTextureName(gltfMaterial->occlusion_texture.texture, L"occlusion_texture");
			const uint32_t channel = getTextureCoordChannel(&gltfMaterial->occlusion_texture, outModel);
			Ref< drawing::Image > image = loadTextureImage(gltfMaterial->occlusion_texture.texture, basePath);

			// Map occlusion to ambient occlusion if your engine supports it
			// For now, we'll skip it as the base Material class doesn't have AO
		}

		// Emissive
		if (gltfMaterial->has_emissive_strength)
		{
			float emissive = gltfMaterial->emissive_strength.emissive_strength;
			emissive *= std::max({ gltfMaterial->emissive_factor[0],
				gltfMaterial->emissive_factor[1],
				gltfMaterial->emissive_factor[2] });
			material.setEmissive(emissive);
		}

		if (gltfMaterial->emissive_texture.texture)
		{
			const std::wstring textureName = getTextureName(gltfMaterial->emissive_texture.texture, L"emissive_texture");
			const uint32_t channel = getTextureCoordChannel(&gltfMaterial->emissive_texture, outModel);
			Ref< drawing::Image > image = loadTextureImage(gltfMaterial->emissive_texture.texture, basePath);

			material.setEmissiveMap(Material::Map(textureName, channel, false, Guid(), image));
		}

		// Alpha mode and cutoff
		if (gltfMaterial->alpha_mode == cgltf_alpha_mode_blend)
		{
			material.setBlendOperator(Material::BoAlpha);
		}
		else if (gltfMaterial->alpha_mode == cgltf_alpha_mode_mask)
		{
			material.setBlendOperator(Material::BoAlpha);
			// Alpha cutoff could be stored in transparency if needed
		}

		// Double sided
		material.setDoubleSided(gltfMaterial->double_sided);

		// Handle extensions if needed (PBR Specular Glossiness, etc.)
		if (gltfMaterial->has_pbr_specular_glossiness)
		{
			const cgltf_pbr_specular_glossiness* specGloss = &gltfMaterial->pbr_specular_glossiness;

			// Convert specular-glossiness to metallic-roughness approximation
			material.setColor(convertColor(specGloss->diffuse_factor, 4));
			material.setSpecularTerm(std::max({ specGloss->specular_factor[0],
				specGloss->specular_factor[1],
				specGloss->specular_factor[2] }));
			material.setRoughness(1.0f - specGloss->glossiness_factor);

			if (specGloss->diffuse_texture.texture)
			{
				const std::wstring textureName = getTextureName(specGloss->diffuse_texture.texture, L"diffuse_texture");
				const uint32_t channel = getTextureCoordChannel(&specGloss->diffuse_texture, outModel);
				Ref< drawing::Image > image = loadTextureImage(specGloss->diffuse_texture.texture, basePath);

				material.setDiffuseMap(Material::Map(textureName, channel, true, Guid(), image));
			}
		}

		// Add material to model
		outMaterialMap[i] = outModel.addUniqueMaterial(material);
	}

	return true;
}

}