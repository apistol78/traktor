/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/ModelFormatGltf.h"

#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Formats/Gltf/GltfMaterialConverter.h"
#include "Model/Formats/Gltf/GltfMeshConverter.h"
#include "Model/Formats/Gltf/GltfSkeletonConverter.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"

#include <functional>
#include <limits>

namespace traktor::model
{
namespace
{

bool include(const cgltf_node* node, const std::wstring& filter)
{
	if (!node)
		return false;

	// Empty filter means everything is included
	if (filter.empty())
		return true;

	// Check if node's name matches filter tag
	const std::wstring name = node->name ? mbstows(node->name) : L"";

	// Simple include/exclude logic based on comma-separated list
	for (auto s : StringSplit< std::wstring >(filter, L",;"))
	{
		s = trim(s);
		if (s.front() != L'!')
		{
			if (s == name)
				return true;
		}
		else
		{
			if (s.substr(1) != name)
				return true;
		}
	}

	return false;
}

bool traverseNodes(const cgltf_data* data, const cgltf_node* node, const std::wstring& filter, const std::function< bool(const cgltf_node* node) >& visitor)
{
	if (!include(node, filter))
		return true;

	if (!visitor(node))
		return false;

	for (cgltf_size i = 0; i < node->children_count; ++i)
	{
		const cgltf_node* child = node->children[i];
		if (child)
		{
			if (!traverseNodes(data, child, filter, visitor))
				return false;
		}
	}

	return true;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatGltf", 0, ModelFormatGltf, ModelFormat)

void ModelFormatGltf::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"glTF 2.0";
	outExtensions.push_back(L"gltf");
	outExtensions.push_back(L"glb");
}

bool ModelFormatGltf::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"gltf") == 0 ||
		compareIgnoreCase(extension, L"glb") == 0;
}

Ref< Model > ModelFormatGltf::read(const Path& filePath, const std::wstring& filter) const
{
	// Setup cgltf options
	cgltf_options options = {};

	// Parse the GLTF file
	cgltf_data* data = nullptr;
	const std::string filePathStr = wstombs(filePath.getPathNameOS());
	cgltf_result result = cgltf_parse_file(&options, filePathStr.c_str(), &data);

	if (result != cgltf_result_success)
	{
		log::error << L"Failed to parse GLTF file: " << filePath.getPathNameOS() << L" (error " << (int)result << L")" << Endl;
		return nullptr;
	}

	// Load buffers
	result = cgltf_load_buffers(&options, data, filePathStr.c_str());
	if (result != cgltf_result_success)
	{
		log::error << L"Failed to load GLTF buffers: " << filePath.getPathNameOS() << L" (error " << (int)result << L")" << Endl;
		cgltf_free(data);
		return nullptr;
	}

	// Validate the data
	result = cgltf_validate(data);
	if (result != cgltf_result_success)
	{
		log::warning << L"GLTF validation failed: " << filePath.getPathNameOS() << L" (error " << (int)result << L")" << Endl;
		// Continue anyway - validation failures might not be critical
	}

#if defined(_DEBUG)
	// Debug output
	log::info << L"GLTF file loaded successfully:" << Endl;
	log::info << L"  Meshes: " << data->meshes_count << Endl;
	log::info << L"  Materials: " << data->materials_count << Endl;
	log::info << L"  Nodes: " << data->nodes_count << Endl;
	log::info << L"  Animations: " << data->animations_count << Endl;
	log::info << L"  Skins: " << data->skins_count << Endl;
#endif

	// Calculate axis transform
	const Matrix44 axisTransform = calculateGltfAxisTransform();

	Ref< Model > model = new Model();
	bool success = true;

	// Convert materials first
	SmallMap< cgltf_size, int32_t > materialMap;
	if (data->materials_count > 0)
	{
		if (!convertMaterials(*model, materialMap, data, filePath))
		{
			log::error << L"Failed to convert materials." << Endl;
			success = false;
		}
	}

	// Convert skeleton if present
	if (success && data->skins_count > 0)
	{
		// Use the first skin for now
		const cgltf_skin* skin = &data->skins[0];
		if (!convertSkeleton(*model, data, skin, axisTransform))
		{
			log::error << L"Failed to convert skeleton." << Endl;
			success = false;
		}
	}

	// Convert meshes
	if (success)
	{
		// Process scene nodes or all nodes if no scene is specified
		const cgltf_scene* scene = data->scene ? data->scene : (data->scenes_count > 0 ? &data->scenes[0] : nullptr);

		if (scene)
		{
			// Process scene nodes
			for (cgltf_size i = 0; i < scene->nodes_count; ++i)
			{
				const cgltf_node* node = scene->nodes[i];
				if (!traverseNodes(data, node, filter, [&](const cgltf_node* n) {
					if (n->mesh)
					{
						if (!convertMesh(*model, data, n, materialMap, axisTransform))
						{
							log::error << L"Failed to convert mesh for node." << Endl;
							success = false;
							return false;
						}
					}
					return true;
				}))
					break;
			}
		}
		else
		{
			// Process all nodes
			for (cgltf_size i = 0; i < data->nodes_count; ++i)
			{
				const cgltf_node* node = &data->nodes[i];
				if (include(node, filter) && node->mesh)
				{
					if (!convertMesh(*model, data, node, materialMap, axisTransform))
					{
						log::error << L"Failed to convert mesh for node " << i << Endl;
						success = false;
						break;
					}
				}
			}
		}
	}

	// Convert animations
	if (success && data->animations_count > 0 && model->getJointCount() > 0)
	{
		for (cgltf_size i = 0; i < data->animations_count; ++i)
		{
			const cgltf_animation* animation = &data->animations[i];

			Ref< Animation > anim = new Animation();
			anim->setName(animation->name ? mbstows(animation->name) : (L"Animation_" + toString(i)));

			// Sample animation at 30 FPS
			// Find animation duration
			float duration = 0.0f;
			for (cgltf_size j = 0; j < animation->channels_count; ++j)
			{
				const cgltf_animation_channel* channel = &animation->channels[j];
				if (channel->sampler && channel->sampler->input)
				{
					const cgltf_accessor* timeAccessor = channel->sampler->input;
					if (timeAccessor->count > 0)
					{
						cgltf_float maxTime;
						if (cgltf_accessor_read_float(timeAccessor, timeAccessor->count - 1, &maxTime, 1))
							duration = std::max(duration, maxTime);
					}
				}
			}

			if (duration > 0.0f)
			{
				const float frameRate = 30.0f;
				const int32_t frameCount = (int32_t)(duration * frameRate) + 1;

				for (int32_t frame = 0; frame < frameCount; ++frame)
				{
					const float time = frame / frameRate;

					Ref< Pose > pose = convertPose(*model, data, animation, time, axisTransform);
					if (pose)
						anim->insertKeyFrame(time, pose);
				}

				model->addAnimation(anim);
			}
		}
	}

	// Cleanup
	cgltf_free(data);

	if (!success)
		return nullptr;

	// Create default material if needed
	uint32_t defaultMaterialIndex = c_InvalidIndex;
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
	{
		const Polygon& polygon = model->getPolygon(i);
		if (polygon.getMaterial() == c_InvalidIndex)
		{
			if (defaultMaterialIndex == c_InvalidIndex)
			{
				Material material;
				material.setName(L"GLTF_Default");
				material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				material.setDiffuseTerm(1.0f);
				material.setSpecularTerm(1.0f);
				material.setRoughness(0.5f);
				material.setMetalness(0.0f);
				defaultMaterialIndex = model->addMaterial(material);
			}

			Polygon replacement = polygon;
			replacement.setMaterial(defaultMaterialIndex);
			model->setPolygon(i, replacement);
		}
	}

	// Ensure texture coordinate channels are properly assigned
	const auto& channels = model->getTexCoordChannels();
	if (!channels.empty())
	{
		auto materials = model->getMaterials();
		for (auto& material : materials)
		{
			// Set default texture coordinate channel for maps that don't have one assigned
			{
				auto map = material.getDiffuseMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setDiffuseMap(map);
			}
			{
				auto map = material.getSpecularMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setSpecularMap(map);
			}
			{
				auto map = material.getRoughnessMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setRoughnessMap(map);
			}
			{
				auto map = material.getMetalnessMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setMetalnessMap(map);
			}
			{
				auto map = material.getTransparencyMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setTransparencyMap(map);
			}
			{
				auto map = material.getEmissiveMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setEmissiveMap(map);
			}
			{
				auto map = material.getReflectiveMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setReflectiveMap(map);
			}
			{
				auto map = material.getNormalMap();
				if (map.channel == c_InvalidIndex)
					map.channel = 0;
				material.setNormalMap(map);
			}
		}
		model->setMaterials(materials);
	}

	// Clean up duplicate vertices and optimize the model
	model->apply(CleanDuplicates(std::numeric_limits< float >::min()));

	return model;
}

bool ModelFormatGltf::write(const Path& filePath, const Model* model) const
{
	// Writing GLTF files is not implemented yet
	return false;
}

}