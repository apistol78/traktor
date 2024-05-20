/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "ufbx.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/MeshConverter.h"
#include "Model/Formats/Fbx/ModelFormatFbx.h"
#include "Model/Formats/Fbx/SkeletonConverter.h"

namespace traktor::model
{
	namespace
	{

bool include(ufbx_node* node, const std::wstring& filter)
{
	if (!node)
		return false;

	// Always accept root nodes.
	if (node->is_root)
		return true;

	// Only filter on first level of child nodes.
	if (!node->parent->is_root)
		return true;

	// Empty filter means everything is included.
	if (filter.empty())
		return true;

	// Check if node's name match filter tag.
	const std::wstring name = mbstows(node->name.data);
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

ufbx_node* search(ufbx_node* node, const std::wstring& filter, const std::function< bool (ufbx_node* node) >& predicate)
{
	if (!include(node, filter))
		return nullptr;

	if (predicate(node))
		return node;

	for (size_t i = 0; i < node->children.count; ++i)
	{
		ufbx_node* child = node->children.data[i];
		if (child)
		{
			ufbx_node* foundNode = search(child, filter, predicate);
			if (foundNode)
				return foundNode;
		}
	}

	return nullptr;
}

bool traverse(ufbx_node* node, const std::wstring& filter, const std::function< bool (ufbx_node* node, int32_t) >& visitor, int32_t depth = 0)
{
	if (!include(node, filter))
		return true;

	if (!visitor(node, depth))
		return false;

	for (size_t i = 0; i < node->children.count; ++i)
	{
		ufbx_node* child = node->children.data[i];
		if (child)
		{
			if (!traverse(child, filter, visitor, depth + 1))
				return false;
		}
	}

	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatFbx", 0, ModelFormatFbx, ModelFormat)

void ModelFormatFbx::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"FBX";
	outExtensions.push_back(L"fbx");
}

bool ModelFormatFbx::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"fbx") == 0;
}

Ref< Model > ModelFormatFbx::read(const Path& filePath, const std::wstring& filter) const
{
	ufbx_load_opts opts = {};
	opts.load_external_files = true;
	ufbx_scene* scene = ufbx_load_file(wstombs(filePath.getPathNameOS()).c_str(), &opts, nullptr);
	if (!scene)
		return nullptr;

#if defined(_DEBUG)
	traverse(scene->root_node, L"", [](ufbx_node* node, int32_t depth) {
		for (int32_t i = 0; i < depth; ++i)
			log::info << L" ";
		log::info << mbstows(node->name.data) << L" " << (node->bind_pose != nullptr ? L"Y" : L"N"
		) << Endl;
		return true;
	});
#endif

	const Matrix44 axisTransform = calculateAxisTransform(scene->settings.axes);

	Ref< Model > model = new Model();
	bool result = true;

	// First convert skeleton since it's used to figure out correct weight index
	// when converting meshes.
	ufbx_node* skeletonNode = search(scene->root_node, filter, [&](ufbx_node* node) {
		return (node->children.count > 0 && node->bind_pose != nullptr);
	});
	if (skeletonNode)
	{
		if (!convertSkeleton(*model, scene, skeletonNode, axisTransform))
			return nullptr;
	}

	// Convert and merge all meshes.
	result &= traverse(scene->root_node, filter, [&](ufbx_node* node, int32_t) {
		if (node->visible)
		{
			if (node->mesh != nullptr)
			{
				if (!convertMesh(*model, scene, node, axisTransform))
					return false;
			}
		}
		return true;
	});

	// Convert animations.
	if (skeletonNode)
	{
		for (size_t i = 0; i < scene->anim_stacks.count; ++i)
		{
			ufbx_anim_stack* animStack = scene->anim_stacks.data[i];
			if (!animStack)
				continue;

			std::wstring takeName = mbstows(animStack->name.data);
			const size_t p = takeName.find_last_of(L'|');
			if (p != std::wstring::npos)
				takeName = takeName.substr(p + 1);

			Ref< Animation > anim = new Animation();
			anim->setName(takeName);

			const int32_t startFrame = int32_t(animStack->time_begin * 30.0);
			const int32_t endFrame = int32_t(animStack->time_end * 30.0);

			for (int32_t frame = startFrame; frame <= endFrame; ++frame)
			{
				const double time = double(frame) / 30.0;

				Ref< Pose > pose = convertPose(*model, scene, skeletonNode, animStack->anim, time, axisTransform);
				if (!pose)
					return nullptr;

				anim->insertKeyFrame(frame / 30.0f, pose);
			}

			model->addAnimation(anim);
		}
	}

	ufbx_free_scene(scene);

	// Create and assign default material if anonymous faces has been created.
	uint32_t defaultMaterialIndex = c_InvalidIndex;
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
	{
		const Polygon& polygon = model->getPolygon(i);
		if (polygon.getMaterial() == c_InvalidIndex)
		{
			if (defaultMaterialIndex == c_InvalidIndex)
			{
				Material material;
				material.setName(L"FBX_Default");
				material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				material.setDiffuseTerm(1.0f);
				material.setSpecularTerm(1.0f);
				defaultMaterialIndex = model->addMaterial(material);
			}

			Polygon replacement = polygon;
			replacement.setMaterial(defaultMaterialIndex);
			model->setPolygon(i, replacement);
		}
	}

	const auto& channels = model->getTexCoordChannels();
	if (!channels.empty())
	{
		const std::wstring& channel = channels[0];
		auto materials = model->getMaterials();
		for (auto& material : materials)
		{
			{
				auto map = material.getDiffuseMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setDiffuseMap(map);
			}
			{
				auto map = material.getSpecularMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setSpecularMap(map);
			}
			{
				auto map = material.getRoughnessMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setRoughnessMap(map);
			}
			{
				auto map = material.getMetalnessMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setMetalnessMap(map);
			}
			{
				auto map = material.getTransparencyMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setTransparencyMap(map);
			}
			{
				auto map = material.getEmissiveMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setEmissiveMap(map);
			}
			{
				auto map = material.getReflectiveMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setReflectiveMap(map);
			}
			{
				auto map = material.getNormalMap();
				if (map.channel == L"")
					map.channel = channel;
				material.setNormalMap(map);
			}
		}
		model->setMaterials(materials);
	}

	return model;
}

bool ModelFormatFbx::write(const Path& filePath, const Model* model) const
{
	return false;
}

}
