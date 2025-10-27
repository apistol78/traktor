/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "Model/Formats/Gltf/ModelFormatGltf.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Formats/Gltf/GltfMeshConverter.h"
#include "Model/Formats/Gltf/GltfSkeletonConverter.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"

#include <functional>

namespace traktor::model
{
namespace
{

bool include(cgltf_node* node, const std::wstring& filter)
{
	if (!node)
		return false;

	// Always accept root nodes
	if (!node->parent)
		return true;

	// Only filter on first level of child nodes
	if (node->parent && node->parent->parent)
		return true;

	// Empty filter means everything is included
	if (filter.empty())
		return true;

	// Check if node's name matches filter tag
	const std::wstring name = node->name ? mbstows(node->name) : L"";
	for (auto s : StringSplit< std::wstring >(filter, L",;"))
	{
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

cgltf_node* search(cgltf_node* node, const std::wstring& filter, const std::function< bool(cgltf_node* node) >& predicate)
{
	if (!include(node, filter))
		return nullptr;

	if (predicate(node))
		return node;

	for (cgltf_size i = 0; i < node->children_count; ++i)
	{
		cgltf_node* child = node->children[i];
		if (child)
		{
			cgltf_node* foundNode = search(child, filter, predicate);
			if (foundNode)
				return foundNode;
		}
	}

	return nullptr;
}

bool traverse(cgltf_node* node, const std::wstring& filter, const std::function< bool(cgltf_node* node, int32_t) >& visitor, int32_t depth = 0)
{
	if (!include(node, filter))
		return true;

	if (!visitor(node, depth))
		return false;

	for (cgltf_size i = 0; i < node->children_count; ++i)
	{
		cgltf_node* child = node->children[i];
		if (child)
		{
			if (!traverse(child, filter, visitor, depth + 1))
				return false;
		}
	}

	return true;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatGltf", 0, ModelFormatGltf, ModelFormat)

void ModelFormatGltf::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"GLTF";
	outExtensions.push_back(L"gltf");
	outExtensions.push_back(L"glb");
}

bool ModelFormatGltf::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"gltf") == 0 || compareIgnoreCase(extension, L"glb") == 0;
}

Ref< Model > ModelFormatGltf::read(const Path& filePath, const std::wstring& filter) const
{
	cgltf_options options = {};
	cgltf_data* data = nullptr;

	// Parse the glTF file
	cgltf_result result = cgltf_parse_file(&options, wstombs(filePath.getPathNameOS()).c_str(), &data);
	if (result != cgltf_result_success)
	{
		log::error << L"Failed to parse GLTF file: " << filePath.getPathNameOS() << Endl;
		return nullptr;
	}

	// Load buffers
	result = cgltf_load_buffers(&options, data, wstombs(filePath.getPathNameOS()).c_str());
	if (result != cgltf_result_success)
	{
		log::error << L"Failed to load GLTF buffers: " << filePath.getPathNameOS() << Endl;
		cgltf_free(data);
		return nullptr;
	}

	// Validate the data
	result = cgltf_validate(data);
	if (result != cgltf_result_success)
	{
		log::warning << L"GLTF validation warnings: " << filePath.getPathNameOS() << Endl;
	}

#if defined(_DEBUG)
	// Debug output scene hierarchy
	if (data->scene && data->scene->nodes_count > 0)
	{
		for (cgltf_size i = 0; i < data->scene->nodes_count; ++i)
		{
			traverse(data->scene->nodes[i], L"", [](cgltf_node* node, int32_t depth) {
				for (int32_t j = 0; j < depth; ++j)
					log::info << L" ";
				log::info << (node->name ? mbstows(node->name) : L"<unnamed>") << Endl;
				return true;
			});
		}
	}
#endif

	// glTF uses Y-up, right-handed coordinate system (+X right, +Y up, +Z toward viewer)
	// Traktor uses a different coordinate system - flip X axis
	const Matrix44 axisTransform = Matrix44(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Ref< Model > model = new Model();
	bool success = true;

	const std::wstring basePath = filePath.getPathOnly();

	// Convert skeleton if present
	cgltf_skin* skin = nullptr;
	if (data->skins_count > 0)
	{
		skin = &data->skins[0]; // Use first skin
		if (!convertSkeleton(*model, data, skin, axisTransform))
		{
			log::warning << L"Failed to convert skeleton." << Endl;
		}
	}

	// Convert meshes
	if (data->scene && data->scene->nodes_count > 0)
	{
		for (cgltf_size i = 0; i < data->scene->nodes_count; ++i)
		{
			success &= traverse(data->scene->nodes[i], filter, [&](cgltf_node* node, int32_t) {
				if (node->mesh != nullptr)
				{
					if (!convertMesh(*model, data, node, axisTransform, basePath))
						return false;
				}
				return true;
			});
		}
	}

	// Convert animations
	if (skin && data->animations_count > 0)
	{
		for (cgltf_size i = 0; i < data->animations_count; ++i)
		{
			cgltf_animation* anim = &data->animations[i];
			std::wstring animName = anim->name ? mbstows(anim->name) : L"Animation_" + toString(i);

			Ref< Animation > animation = new Animation();
			animation->setName(animName);

			// Find animation time range
			float minTime = std::numeric_limits<float>::max();
			float maxTime = std::numeric_limits<float>::min();

			for (cgltf_size j = 0; j < anim->samplers_count; ++j)
			{
				cgltf_animation_sampler* sampler = &anim->samplers[j];
				if (sampler->input)
				{
					for (cgltf_size k = 0; k < sampler->input->count; ++k)
					{
						float t;
						cgltf_accessor_read_float(sampler->input, k, &t, 1);
						minTime = std::min(minTime, t);
						maxTime = std::max(maxTime, t);
					}
				}
			}

			// Sample animation at 30 fps
			const float fps = 30.0f;
			const int32_t startFrame = int32_t(minTime * fps);
			const int32_t endFrame = int32_t(maxTime * fps);

			for (int32_t frame = startFrame; frame <= endFrame; ++frame)
			{
				const float time = float(frame) / fps;

				Ref< Pose > pose = convertPose(*model, data, anim, skin, time, axisTransform);
				if (!pose)
				{
					log::warning << L"Failed to convert pose at frame " << frame << Endl;
					continue;
				}

				animation->insertKeyFrame(time, pose);
			}

			model->addAnimation(animation);
		}
	}

	cgltf_free(data);

	if (!success)
		return nullptr;

	// Remove anonymous faces
	AlignedVector< Polygon > keepPolygons;
	keepPolygons.reserve(model->getPolygonCount());
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
	{
		const Polygon& polygon = model->getPolygon(i);
		if (polygon.getMaterial() != c_InvalidIndex)
			keepPolygons.push_back(polygon);
	}
	model->setPolygons(keepPolygons);

	// Set default texture channel for materials
	const auto& channels = model->getTexCoordChannels();
	if (!channels.empty())
	{
		auto materials = model->getMaterials();
		for (auto& material : materials)
		{
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

	model->apply(CleanDuplicates(std::numeric_limits< float >::min()));

	return model;
}

bool ModelFormatGltf::write(const Path& filePath, const Model* model) const
{
	// Writing glTF is not implemented
	return false;
}

}
