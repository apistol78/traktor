/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Gltf/ModelFormatGltf.h"

#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Quaternion.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Formats/Gltf/GltfConversion.h"
#include "Model/Formats/Gltf/GltfMaterialConverter.h"
#include "Model/Formats/Gltf/GltfMeshConverter.h"
#include "Model/Formats/Gltf/GltfSkeletonConverter.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Operations/CleanDuplicates.h"

// The cgltf reader implementation (CGLTF_IMPLEMENTATION) lives in GltfConversion.cpp;
// this translation unit provides the writer implementation. cgltf_write.h is
// self-contained and does not depend on the reader implementation.
#define CGLTF_WRITE_IMPLEMENTATION
#include "Model/Formats/Gltf/cgltf_write.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <utility>
#include <vector>

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
	const uint32_t vertexCount = model->getVertexCount();
	const uint32_t polygonCount = model->getPolygonCount();

	if (vertexCount == 0 || polygonCount == 0)
	{
		log::error << L"Unable to write glTF; model contains no geometry." << Endl;
		return false;
	}

	// A ".glb" extension selects the binary container; anything else (".gltf")
	// produces a text file with the buffer embedded as a base64 data URI.
	const bool binary = compareIgnoreCase(filePath.getExtension(), L"glb") == 0;

	// glTF uses a Y-up, right-handed coordinate system. Convert from engine
	// space using the same axis transform used when reading; it is a reflection
	// about X and therefore its own inverse, so read/write round-trips exactly.
	// Transforms are converted by conjugation (A * M * A) so skinning stays
	// consistent with the reflected vertex positions.
	const Matrix44 axisTransform = calculateGltfAxisTransform();

	const uint32_t jointCount = model->getJointCount();
	const bool hasSkin = (jointCount > 0);

	// Detect which optional vertex attributes are present in the model.
	bool hasNormals = false;
	bool hasTexCoords = false;
	bool hasColors = false;
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const Vertex& v = model->getVertex(i);
		if (v.getNormal() != c_InvalidIndex)
			hasNormals = true;
		if (v.getTexCoord(0) != c_InvalidIndex)
			hasTexCoords = true;
		if (v.getColor() != c_InvalidIndex)
			hasColors = true;
	}

	// Build tightly-packed vertex attribute arrays, one entry per model vertex.
	// Because every model vertex is emitted in order, a polygon's vertex index
	// is also its glTF vertex index.
	std::vector< float > positions(vertexCount * 3);
	std::vector< float > normals(hasNormals ? vertexCount * 3 : 0);
	std::vector< float > texCoords(hasTexCoords ? vertexCount * 2 : 0);
	std::vector< float > colors(hasColors ? vertexCount * 4 : 0);
	std::vector< uint16_t > jointsData(hasSkin ? vertexCount * 4 : 0);
	std::vector< float > weightsData(hasSkin ? vertexCount * 4 : 0);

	float posMin[3] = { std::numeric_limits< float >::max(), std::numeric_limits< float >::max(), std::numeric_limits< float >::max() };
	float posMax[3] = { -std::numeric_limits< float >::max(), -std::numeric_limits< float >::max(), -std::numeric_limits< float >::max() };

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const Vertex& v = model->getVertex(i);

		const Vector4 position = axisTransform * model->getPosition(v.getPosition());
		position.storeUnaligned3(&positions[i * 3]);
		for (int32_t k = 0; k < 3; ++k)
		{
			posMin[k] = std::min(posMin[k], positions[i * 3 + k]);
			posMax[k] = std::max(posMax[k], positions[i * 3 + k]);
		}

		if (hasNormals)
		{
			Vector4 normal(0.0f, 1.0f, 0.0f, 0.0f);
			if (v.getNormal() != c_InvalidIndex)
				normal = (axisTransform * model->getNormal(v.getNormal())).normalized();
			normal.storeUnaligned3(&normals[i * 3]);
		}

		if (hasTexCoords)
		{
			Vector2 uv(0.0f, 0.0f);
			if (v.getTexCoord(0) != c_InvalidIndex)
				uv = model->getTexCoord(v.getTexCoord(0));
			texCoords[i * 2 + 0] = uv.x;
			texCoords[i * 2 + 1] = uv.y;
		}

		if (hasColors)
		{
			Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);
			if (v.getColor() != c_InvalidIndex)
				color = model->getColor(v.getColor());
			color.storeUnaligned(&colors[i * 4]);
		}

		if (hasSkin)
		{
			// Gather the (up to) four strongest joint influences, normalised.
			std::vector< std::pair< float, uint32_t > > influences;
			const uint32_t influenceCount = v.getJointInfluenceCount();
			for (uint32_t j = 0; j < influenceCount; ++j)
			{
				const float w = v.getJointInfluence(j);
				if (w > 0.0f)
					influences.push_back(std::make_pair(w, j));
			}
			std::sort(influences.begin(), influences.end(), [](const std::pair< float, uint32_t >& a, const std::pair< float, uint32_t >& b) {
				return a.first > b.first;
			});

			const size_t n = std::min< size_t >(4, influences.size());
			float sum = 0.0f;
			for (size_t k = 0; k < n; ++k)
				sum += influences[k].first;

			for (size_t k = 0; k < 4; ++k)
			{
				uint16_t jointIndex = 0;
				float weight = 0.0f;
				if (k < n)
				{
					jointIndex = (uint16_t)influences[k].second;
					weight = (sum > 0.0f) ? influences[k].first / sum : 0.0f;
				}
				jointsData[i * 4 + k] = jointIndex;
				weightsData[i * 4 + k] = weight;
			}

			// A vertex with no influence still needs weights summing to one.
			if (n == 0)
				weightsData[i * 4 + 0] = 1.0f;
		}
	}

	// Group triangles by material into one primitive each; the extra bucket at
	// index materialCount collects polygons without a valid material.
	const uint32_t materialCount = model->getMaterialCount();
	std::vector< std::vector< uint32_t > > bucketIndices(materialCount + 1);

	for (uint32_t i = 0; i < polygonCount; ++i)
	{
		const Polygon& polygon = model->getPolygon(i);
		const uint32_t vc = polygon.getVertexCount();
		if (vc < 3)
			continue;

		const uint32_t material = polygon.getMaterial();
		const uint32_t bucket = (material < materialCount) ? material : materialCount;
		std::vector< uint32_t >& indices = bucketIndices[bucket];

		// Fan triangulation; vertex order is preserved (see axis transform note).
		const uint32_t v0 = polygon.getVertex(0);
		for (uint32_t t = 1; t + 1 < vc; ++t)
		{
			indices.push_back(v0);
			indices.push_back(polygon.getVertex(t));
			indices.push_back(polygon.getVertex(t + 1));
		}
	}

	// Collect non-empty buckets in material order, with the no-material bucket last.
	struct Group { int32_t material; const std::vector< uint32_t >* indices; };
	std::vector< Group > groups;
	for (uint32_t m = 0; m < materialCount; ++m)
	{
		if (!bucketIndices[m].empty())
			groups.push_back({ (int32_t)m, &bucketIndices[m] });
	}
	if (!bucketIndices[materialCount].empty())
		groups.push_back({ -1, &bucketIndices[materialCount] });

	if (groups.empty())
	{
		log::error << L"Unable to write glTF; model contains no triangles." << Endl;
		return false;
	}

	// --- Texture pre-pass -------------------------------------------------
	// Collect the unique images referenced by material maps. Metallic and
	// roughness maps are recombined into a single glTF metallic-roughness
	// texture (roughness in G, metalness in B), mirroring how the reader
	// splits that texture apart on import.
	std::vector< Ref< drawing::Image > > imageStore;
	std::vector< drawing::Image* > uniqueImages;
	std::map< drawing::Image*, int32_t > imageIndex;

	const auto addImage = [&](drawing::Image* image) -> int32_t {
		if (!image)
			return -1;
		auto it = imageIndex.find(image);
		if (it != imageIndex.end())
			return it->second;
		const int32_t index = (int32_t)uniqueImages.size();
		uniqueImages.push_back(image);
		imageIndex.insert(std::make_pair(image, index));
		return index;
	};

	const auto buildMetallicRoughness = [&](drawing::Image* rough, drawing::Image* metal) -> Ref< drawing::Image > {
		const int32_t w = rough ? rough->getWidth() : metal->getWidth();
		const int32_t h = rough ? rough->getHeight() : metal->getHeight();
		Ref< drawing::Image > out = new drawing::Image(drawing::PixelFormat::getR8G8B8(), w, h);
		Color4f c;
		for (int32_t y = 0; y < h; ++y)
		{
			for (int32_t x = 0; x < w; ++x)
			{
				float g = 1.0f, b = 1.0f;
				if (rough && rough->getPixel(std::min(x, rough->getWidth() - 1), std::min(y, rough->getHeight() - 1), c))
					g = c.getRed();
				if (metal && metal->getPixel(std::min(x, metal->getWidth() - 1), std::min(y, metal->getHeight() - 1), c))
					b = c.getRed();
				out->setPixel(x, y, Color4f(0.0f, g, b, 1.0f));
			}
		}
		return out;
	};

	struct MaterialTextures { int32_t baseColor = -1, metallicRoughness = -1, normal = -1, emissive = -1; };
	std::vector< MaterialTextures > materialTextures(materialCount);

	for (uint32_t m = 0; m < materialCount; ++m)
	{
		const Material& src = model->getMaterial(m);
		MaterialTextures& mt = materialTextures[m];

		mt.baseColor = addImage(src.getDiffuseMap().image);
		mt.normal = addImage(src.getNormalMap().image);
		mt.emissive = addImage(src.getEmissiveMap().image);

		drawing::Image* rough = src.getRoughnessMap().image;
		drawing::Image* metal = src.getMetalnessMap().image;
		if (rough != nullptr || metal != nullptr)
		{
			Ref< drawing::Image > mr = buildMetallicRoughness(rough, metal);
			imageStore.push_back(mr);
			mt.metallicRoughness = addImage(mr);
		}
	}

	// Encode each unique image to PNG and embed it as a base64 data URI (works
	// for both text glTF and GLB, keeping the output self-contained).
	std::vector< std::string > imageUris(uniqueImages.size());
	for (size_t i = 0; i < uniqueImages.size(); ++i)
	{
		AlignedVector< uint8_t > pngBuffer;
		DynamicMemoryStream ms(pngBuffer, false, true);
		if (uniqueImages[i]->save(&ms, L"png"))
		{
			ms.close();
			Base64 base64;
			imageUris[i] = "data:image/png;base64," + wstombs(base64.encode(pngBuffer));
		}
		else
			log::warning << L"Failed to encode texture image " << (int32_t)i << L"; skipping." << Endl;
	}

	// --- Skeleton pre-pass ------------------------------------------------
	std::vector< std::vector< uint32_t > > jointChildren(hasSkin ? jointCount : 0);
	std::vector< uint32_t > rootJoints;
	uint32_t childLinkCount = 0;
	if (hasSkin)
	{
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			const uint32_t parent = model->getJoint(i).getParent();
			if (parent != c_InvalidIndex && parent < jointCount)
			{
				jointChildren[parent].push_back(i);
				++childLinkCount;
			}
			else
				rootJoints.push_back(i);
		}
	}

	// --- Animation pre-pass -----------------------------------------------
	// Bake each animation as translation/rotation channels for every joint
	// (engine joint transforms are rigid, so no scale channel is needed).
	struct AnimData
	{
		std::wstring name;
		std::vector< float > times;
		std::vector< std::vector< float > > translations; // per joint: keyframeCount * 3
		std::vector< std::vector< float > > rotations;     // per joint: keyframeCount * 4
	};
	std::vector< AnimData > anims;

	if (hasSkin)
	{
		for (uint32_t a = 0; a < model->getAnimationCount(); ++a)
		{
			const Animation* animation = model->getAnimation(a);
			const uint32_t keyFrameCount = animation->getKeyFrameCount();
			if (keyFrameCount == 0)
				continue;

			AnimData ad;
			ad.name = animation->getName();
			ad.times.resize(keyFrameCount);
			ad.translations.resize(jointCount);
			ad.rotations.resize(jointCount);
			for (uint32_t j = 0; j < jointCount; ++j)
			{
				ad.translations[j].resize(keyFrameCount * 3);
				ad.rotations[j].resize(keyFrameCount * 4);
			}

			for (uint32_t f = 0; f < keyFrameCount; ++f)
			{
				ad.times[f] = animation->getKeyFrameTime(f);
				const Pose* pose = animation->getKeyFramePose(f);
				const uint32_t poseCount = pose ? pose->getJointTransformCount() : 0;

				for (uint32_t j = 0; j < jointCount; ++j)
				{
					// Use the pose's local transform where stored, otherwise fall
					// back to the joint's bind transform so unposed joints hold.
					const Transform local = (pose != nullptr && j < poseCount) ? pose->getJointTransform(j) : model->getJoint(j).getTransform();
					const Matrix44 m = axisTransform * local.toMatrix44() * axisTransform;

					m.translation().storeUnaligned3(&ad.translations[j][f * 3]);

					const Quaternion q(m);
					q.e.storeUnaligned(&ad.rotations[j][f * 4]);
				}
			}

			anims.push_back(std::move(ad));
		}
	}

	// --- Count the cgltf objects ------------------------------------------
	const uint32_t numVertexAccessors = 1 + (hasNormals ? 1 : 0) + (hasTexCoords ? 1 : 0) + (hasColors ? 1 : 0) + (hasSkin ? 2 : 0);
	const uint32_t numGroups = (uint32_t)groups.size();

	uint32_t animAccessorCount = 0;
	uint32_t animSamplerCount = 0;
	for (const auto& ad : anims)
	{
		animAccessorCount += 1 + 2 * jointCount; // one time input + (T, R) output per joint
		animSamplerCount += 2 * jointCount;      // one sampler per output
	}

	const uint32_t accessorCount = numVertexAccessors + numGroups + (hasSkin ? 1 : 0) + animAccessorCount;
	const uint32_t nodeCount = (hasSkin ? jointCount : 0) + 1;
	const uint32_t meshNodeIndex = hasSkin ? jointCount : 0;

	// --- Assemble the binary blob -----------------------------------------
	std::vector< uint8_t > bin;
	const auto appendBytes = [&](const void* p, size_t n) -> uint32_t {
		const uint32_t offset = (uint32_t)bin.size();
		bin.resize(offset + n);
		if (n > 0)
			std::memcpy(bin.data() + offset, p, n);
		return offset;
	};

	const uint32_t posOffset = appendBytes(positions.data(), positions.size() * sizeof(float));
	uint32_t nrmOffset = 0, uvOffset = 0, colOffset = 0, jointsOffset = 0, weightsOffset = 0;
	if (hasNormals)
		nrmOffset = appendBytes(normals.data(), normals.size() * sizeof(float));
	if (hasTexCoords)
		uvOffset = appendBytes(texCoords.data(), texCoords.size() * sizeof(float));
	if (hasColors)
		colOffset = appendBytes(colors.data(), colors.size() * sizeof(float));
	if (hasSkin)
	{
		jointsOffset = appendBytes(jointsData.data(), jointsData.size() * sizeof(uint16_t));
		weightsOffset = appendBytes(weightsData.data(), weightsData.size() * sizeof(float));
	}

	std::vector< uint32_t > idxOffset(numGroups);
	for (uint32_t g = 0; g < numGroups; ++g)
		idxOffset[g] = appendBytes(groups[g].indices->data(), groups[g].indices->size() * sizeof(uint32_t));

	// Inverse bind matrices.
	uint32_t ibmOffset = 0;
	if (hasSkin)
	{
		std::vector< float > ibm(jointCount * 16);
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			const Matrix44 world = model->getJointGlobalTransform(i).toMatrix44();
			const Matrix44 inverseBind = axisTransform * world.inverse() * axisTransform;
			inverseBind.axisX().storeUnaligned(&ibm[i * 16 + 0]);
			inverseBind.axisY().storeUnaligned(&ibm[i * 16 + 4]);
			inverseBind.axisZ().storeUnaligned(&ibm[i * 16 + 8]);
			inverseBind.translation().storeUnaligned(&ibm[i * 16 + 12]);
		}
		ibmOffset = appendBytes(ibm.data(), ibm.size() * sizeof(float));
	}

	// Animation buffer sections: per animation a time input, then per joint a
	// translation output and a rotation output.
	struct AnimOffsets { uint32_t time; std::vector< uint32_t > translation; std::vector< uint32_t > rotation; };
	std::vector< AnimOffsets > animOffsets(anims.size());
	for (size_t a = 0; a < anims.size(); ++a)
	{
		animOffsets[a].time = appendBytes(anims[a].times.data(), anims[a].times.size() * sizeof(float));
		animOffsets[a].translation.resize(jointCount);
		animOffsets[a].rotation.resize(jointCount);
		for (uint32_t j = 0; j < jointCount; ++j)
			animOffsets[a].translation[j] = appendBytes(anims[a].translations[j].data(), anims[a].translations[j].size() * sizeof(float));
		for (uint32_t j = 0; j < jointCount; ++j)
			animOffsets[a].rotation[j] = appendBytes(anims[a].rotations[j].data(), anims[a].rotations[j].size() * sizeof(float));
	}

	// --- Allocate the cgltf object graph ----------------------------------
	// Every cross-reference is a pointer into one of these vectors and cgltf
	// derives JSON indices from pointer arithmetic, so none may be resized
	// after pointers into them are taken.
	std::vector< cgltf_buffer > buffers(1);
	std::vector< cgltf_buffer_view > views(accessorCount);
	std::vector< cgltf_accessor > accessors(accessorCount);
	std::vector< cgltf_attribute > attributes(numVertexAccessors);
	std::vector< cgltf_material > materials(materialCount);
	std::vector< cgltf_primitive > primitives(numGroups);
	std::vector< cgltf_mesh > meshes(1);
	std::vector< cgltf_image > images(uniqueImages.size());
	std::vector< cgltf_texture > textures(uniqueImages.size());
	std::vector< cgltf_sampler > samplers(uniqueImages.empty() ? 0 : 1);
	std::vector< cgltf_skin > skins(hasSkin ? 1 : 0);
	std::vector< cgltf_node > nodes(nodeCount);
	std::vector< cgltf_node* > childPtrs(childLinkCount);
	std::vector< cgltf_node* > skinJointPtrs(hasSkin ? jointCount : 0);
	std::vector< cgltf_node* > sceneNodes(1 + (hasSkin ? (uint32_t)rootJoints.size() : 0));
	std::vector< cgltf_scene > scenes(1);
	std::vector< cgltf_animation > animations(anims.size());
	std::vector< cgltf_animation_sampler > animSamplers(animSamplerCount);
	std::vector< cgltf_animation_channel > animChannels(animSamplerCount);

	buffers[0].size = bin.size();

	// Pool of dynamically-built strings referenced by the cgltf structures;
	// std::list guarantees stable addresses so the c_str() pointers stay valid.
	std::list< std::string > stringPool;
	const auto pool = [&](const std::string& s) -> char* {
		stringPool.push_back(s);
		return const_cast< char* >(stringPool.back().c_str());
	};

	const auto setupAccessor = [&](uint32_t index, uint32_t offset, uint32_t size, cgltf_component_type componentType, cgltf_type type, uint32_t count) {
		views[index].buffer = &buffers[0];
		views[index].offset = offset;
		views[index].size = size;

		accessors[index].component_type = componentType;
		accessors[index].type = type;
		accessors[index].count = count;
		accessors[index].buffer_view = &views[index];
	};

	// --- Vertex accessors --------------------------------------------------
	uint32_t ai = 0;

	const uint32_t posAccessor = ai;
	setupAccessor(ai, posOffset, (uint32_t)(positions.size() * sizeof(float)), cgltf_component_type_r_32f, cgltf_type_vec3, vertexCount);
	accessors[ai].has_min = 1;
	accessors[ai].has_max = 1;
	for (int32_t k = 0; k < 3; ++k)
	{
		accessors[ai].min[k] = posMin[k];
		accessors[ai].max[k] = posMax[k];
	}
	++ai;

	uint32_t nrmAccessor = 0, uvAccessor = 0, colAccessor = 0, jointsAccessor = 0, weightsAccessor = 0;
	if (hasNormals)
	{
		nrmAccessor = ai;
		setupAccessor(ai, nrmOffset, (uint32_t)(normals.size() * sizeof(float)), cgltf_component_type_r_32f, cgltf_type_vec3, vertexCount);
		++ai;
	}
	if (hasTexCoords)
	{
		uvAccessor = ai;
		setupAccessor(ai, uvOffset, (uint32_t)(texCoords.size() * sizeof(float)), cgltf_component_type_r_32f, cgltf_type_vec2, vertexCount);
		++ai;
	}
	if (hasColors)
	{
		colAccessor = ai;
		setupAccessor(ai, colOffset, (uint32_t)(colors.size() * sizeof(float)), cgltf_component_type_r_32f, cgltf_type_vec4, vertexCount);
		++ai;
	}
	if (hasSkin)
	{
		jointsAccessor = ai;
		setupAccessor(ai, jointsOffset, (uint32_t)(jointsData.size() * sizeof(uint16_t)), cgltf_component_type_r_16u, cgltf_type_vec4, vertexCount);
		++ai;

		weightsAccessor = ai;
		setupAccessor(ai, weightsOffset, (uint32_t)(weightsData.size() * sizeof(float)), cgltf_component_type_r_32f, cgltf_type_vec4, vertexCount);
		++ai;
	}

	// --- Index accessors ---------------------------------------------------
	std::vector< uint32_t > groupIndexAccessor(numGroups);
	for (uint32_t g = 0; g < numGroups; ++g)
	{
		groupIndexAccessor[g] = ai;
		setupAccessor(ai, idxOffset[g], (uint32_t)(groups[g].indices->size() * sizeof(uint32_t)), cgltf_component_type_r_32u, cgltf_type_scalar, (uint32_t)groups[g].indices->size());
		++ai;
	}

	// --- Inverse bind matrix accessor --------------------------------------
	uint32_t ibmAccessor = 0;
	if (hasSkin)
	{
		ibmAccessor = ai;
		setupAccessor(ai, ibmOffset, jointCount * 16 * sizeof(float), cgltf_component_type_r_32f, cgltf_type_mat4, jointCount);
		++ai;
	}

	// --- Animation accessors -----------------------------------------------
	std::vector< uint32_t > animTimeAccessor(anims.size());
	std::vector< std::vector< uint32_t > > animTransAccessor(anims.size());
	std::vector< std::vector< uint32_t > > animRotAccessor(anims.size());
	for (size_t a = 0; a < anims.size(); ++a)
	{
		const uint32_t keyFrameCount = (uint32_t)anims[a].times.size();

		animTimeAccessor[a] = ai;
		setupAccessor(ai, animOffsets[a].time, keyFrameCount * sizeof(float), cgltf_component_type_r_32f, cgltf_type_scalar, keyFrameCount);
		accessors[ai].has_min = 1;
		accessors[ai].has_max = 1;
		accessors[ai].min[0] = anims[a].times.front();
		accessors[ai].max[0] = anims[a].times.back();
		++ai;

		animTransAccessor[a].resize(jointCount);
		animRotAccessor[a].resize(jointCount);
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			animTransAccessor[a][j] = ai;
			setupAccessor(ai, animOffsets[a].translation[j], keyFrameCount * 3 * sizeof(float), cgltf_component_type_r_32f, cgltf_type_vec3, keyFrameCount);
			++ai;
		}
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			animRotAccessor[a][j] = ai;
			setupAccessor(ai, animOffsets[a].rotation[j], keyFrameCount * 4 * sizeof(float), cgltf_component_type_r_32f, cgltf_type_vec4, keyFrameCount);
			++ai;
		}
	}

	// --- Shared attribute set ----------------------------------------------
	uint32_t attr = 0;
	attributes[attr++] = { const_cast< char* >("POSITION"), cgltf_attribute_type_position, 0, &accessors[posAccessor] };
	if (hasNormals)
		attributes[attr++] = { const_cast< char* >("NORMAL"), cgltf_attribute_type_normal, 0, &accessors[nrmAccessor] };
	if (hasTexCoords)
		attributes[attr++] = { const_cast< char* >("TEXCOORD_0"), cgltf_attribute_type_texcoord, 0, &accessors[uvAccessor] };
	if (hasColors)
		attributes[attr++] = { const_cast< char* >("COLOR_0"), cgltf_attribute_type_color, 0, &accessors[colAccessor] };
	if (hasSkin)
	{
		attributes[attr++] = { const_cast< char* >("JOINTS_0"), cgltf_attribute_type_joints, 0, &accessors[jointsAccessor] };
		attributes[attr++] = { const_cast< char* >("WEIGHTS_0"), cgltf_attribute_type_weights, 0, &accessors[weightsAccessor] };
	}

	// --- Images, textures and the shared sampler ---------------------------
	if (!samplers.empty())
	{
		samplers[0].mag_filter = cgltf_filter_type_linear;
		samplers[0].min_filter = cgltf_filter_type_linear_mipmap_linear;
		samplers[0].wrap_s = cgltf_wrap_mode_repeat;
		samplers[0].wrap_t = cgltf_wrap_mode_repeat;
	}
	for (size_t i = 0; i < uniqueImages.size(); ++i)
	{
		if (!imageUris[i].empty())
			images[i].uri = pool(imageUris[i]);
		textures[i].image = &images[i];
		if (!samplers.empty())
			textures[i].sampler = &samplers[0];
	}

	// --- Materials ---------------------------------------------------------
	// Base color must always be written explicitly because a zero-initialised
	// factor would otherwise be emitted as opaque black.
	for (uint32_t m = 0; m < materialCount; ++m)
	{
		const Material& src = model->getMaterial(m);
		const MaterialTextures& mt = materialTextures[m];
		cgltf_material& dst = materials[m];

		dst.name = pool(wstombs(src.getName()));
		dst.has_pbr_metallic_roughness = 1;

		src.getColor().storeUnaligned(dst.pbr_metallic_roughness.base_color_factor);
		dst.pbr_metallic_roughness.metallic_factor = src.getMetalness();
		dst.pbr_metallic_roughness.roughness_factor = src.getRoughness();

		if (mt.baseColor >= 0)
		{
			dst.pbr_metallic_roughness.base_color_texture.texture = &textures[mt.baseColor];
			dst.pbr_metallic_roughness.base_color_texture.texcoord = 0;
		}
		if (mt.metallicRoughness >= 0)
		{
			dst.pbr_metallic_roughness.metallic_roughness_texture.texture = &textures[mt.metallicRoughness];
			dst.pbr_metallic_roughness.metallic_roughness_texture.texcoord = 0;
			// Drive metalness/roughness fully from the texture.
			dst.pbr_metallic_roughness.metallic_factor = 1.0f;
			dst.pbr_metallic_roughness.roughness_factor = 1.0f;
		}
		if (mt.normal >= 0)
		{
			dst.normal_texture.texture = &textures[mt.normal];
			dst.normal_texture.texcoord = 0;
			dst.normal_texture.scale = 1.0f;
		}

		float emissive = src.getEmissive();
		if (mt.emissive >= 0)
		{
			dst.emissive_texture.texture = &textures[mt.emissive];
			dst.emissive_texture.texcoord = 0;
			// Ensure the emissive texture is not multiplied away by a zero factor.
			if (emissive <= 0.0f)
				emissive = 1.0f;
		}
		dst.emissive_factor[0] = emissive;
		dst.emissive_factor[1] = emissive;
		dst.emissive_factor[2] = emissive;

		dst.double_sided = src.isDoubleSided() ? 1 : 0;

		switch (src.getBlendOperator())
		{
		case Material::BoAlpha:
			dst.alpha_mode = cgltf_alpha_mode_blend;
			break;
		case Material::BoAlphaTest:
			dst.alpha_mode = cgltf_alpha_mode_mask;
			dst.alpha_cutoff = 0.5f;
			break;
		default:
			dst.alpha_mode = cgltf_alpha_mode_opaque;
			break;
		}
	}

	// --- Primitives and mesh -----------------------------------------------
	for (uint32_t g = 0; g < numGroups; ++g)
	{
		cgltf_primitive& prim = primitives[g];
		prim.type = cgltf_primitive_type_triangles;
		prim.indices = &accessors[groupIndexAccessor[g]];
		prim.attributes = attributes.data();
		prim.attributes_count = numVertexAccessors;
		if (groups[g].material >= 0)
			prim.material = &materials[groups[g].material];
	}

	meshes[0].primitives = primitives.data();
	meshes[0].primitives_count = numGroups;

	// --- Joint nodes, skin and mesh node -----------------------------------
	if (hasSkin)
	{
		float tmp4[4];
		uint32_t childCursor = 0;
		for (uint32_t i = 0; i < jointCount; ++i)
		{
			const Joint& joint = model->getJoint(i);
			cgltf_node& node = nodes[i];
			node.name = pool(wstombs(joint.getName()));

			const Matrix44 local = axisTransform * joint.getTransform().toMatrix44() * axisTransform;

			node.has_translation = 1;
			local.translation().storeUnaligned(tmp4);
			node.translation[0] = tmp4[0];
			node.translation[1] = tmp4[1];
			node.translation[2] = tmp4[2];

			node.has_rotation = 1;
			const Quaternion q(local);
			q.e.storeUnaligned(tmp4);
			node.rotation[0] = tmp4[0];
			node.rotation[1] = tmp4[1];
			node.rotation[2] = tmp4[2];
			node.rotation[3] = tmp4[3];

			// Children.
			const std::vector< uint32_t >& children = jointChildren[i];
			if (!children.empty())
			{
				node.children = &childPtrs[childCursor];
				node.children_count = children.size();
				for (uint32_t c : children)
					childPtrs[childCursor++] = &nodes[c];
			}
		}

		for (uint32_t i = 0; i < jointCount; ++i)
			skinJointPtrs[i] = &nodes[i];

		skins[0].joints = skinJointPtrs.data();
		skins[0].joints_count = jointCount;
		skins[0].inverse_bind_matrices = &accessors[ibmAccessor];
		if (!rootJoints.empty())
			skins[0].skeleton = &nodes[rootJoints.front()];
	}

	cgltf_node& meshNode = nodes[meshNodeIndex];
	meshNode.mesh = &meshes[0];
	if (hasSkin)
		meshNode.skin = &skins[0];

	// --- Scene -------------------------------------------------------------
	uint32_t sceneCursor = 0;
	sceneNodes[sceneCursor++] = &meshNode;
	if (hasSkin)
	{
		for (uint32_t r : rootJoints)
			sceneNodes[sceneCursor++] = &nodes[r];
	}
	scenes[0].nodes = sceneNodes.data();
	scenes[0].nodes_count = sceneCursor;

	// --- Animations --------------------------------------------------------
	uint32_t samplerCursor = 0;
	for (size_t a = 0; a < anims.size(); ++a)
	{
		cgltf_animation& anim = animations[a];
		if (!anims[a].name.empty())
			anim.name = pool(wstombs(anims[a].name));

		anim.samplers = &animSamplers[samplerCursor];
		anim.channels = &animChannels[samplerCursor];
		anim.samplers_count = 2 * jointCount;
		anim.channels_count = 2 * jointCount;

		for (uint32_t j = 0; j < jointCount; ++j)
		{
			// Translation channel.
			animSamplers[samplerCursor].input = &accessors[animTimeAccessor[a]];
			animSamplers[samplerCursor].output = &accessors[animTransAccessor[a][j]];
			animSamplers[samplerCursor].interpolation = cgltf_interpolation_type_linear;
			animChannels[samplerCursor].sampler = &animSamplers[samplerCursor];
			animChannels[samplerCursor].target_node = &nodes[j];
			animChannels[samplerCursor].target_path = cgltf_animation_path_type_translation;
			++samplerCursor;

			// Rotation channel.
			animSamplers[samplerCursor].input = &accessors[animTimeAccessor[a]];
			animSamplers[samplerCursor].output = &accessors[animRotAccessor[a][j]];
			animSamplers[samplerCursor].interpolation = cgltf_interpolation_type_linear;
			animChannels[samplerCursor].sampler = &animSamplers[samplerCursor];
			animChannels[samplerCursor].target_node = &nodes[j];
			animChannels[samplerCursor].target_path = cgltf_animation_path_type_rotation;
			++samplerCursor;
		}
	}

	// --- Assemble the top-level document -----------------------------------
	// For a text glTF the buffer is embedded as a base64 data URI so the output
	// is a single self-contained file. For GLB the buffer is the binary chunk.
	cgltf_data data = {};
	data.file_type = binary ? cgltf_file_type_glb : cgltf_file_type_gltf;
	data.asset.version = const_cast< char* >("2.0");
	data.asset.generator = const_cast< char* >("Traktor");
	data.accessors = accessors.data();
	data.accessors_count = accessorCount;
	data.buffer_views = views.data();
	data.buffer_views_count = accessorCount;
	data.buffers = buffers.data();
	data.buffers_count = 1;
	data.meshes = meshes.data();
	data.meshes_count = 1;
	if (materialCount > 0)
	{
		data.materials = materials.data();
		data.materials_count = materialCount;
	}
	if (!images.empty())
	{
		data.images = images.data();
		data.images_count = images.size();
		data.textures = textures.data();
		data.textures_count = textures.size();
	}
	if (!samplers.empty())
	{
		data.samplers = samplers.data();
		data.samplers_count = samplers.size();
	}
	if (hasSkin)
	{
		data.skins = skins.data();
		data.skins_count = 1;
	}
	data.nodes = nodes.data();
	data.nodes_count = nodeCount;
	data.scenes = scenes.data();
	data.scenes_count = 1;
	data.scene = &scenes[0];
	if (!animations.empty())
	{
		data.animations = animations.data();
		data.animations_count = animations.size();
	}

	std::string embeddedUri;
	if (binary)
	{
		data.bin = bin.data();
		data.bin_size = bin.size();
	}
	else
	{
		Base64 base64;
		embeddedUri = "data:application/octet-stream;base64," + wstombs(base64.encode(bin.data(), (uint32_t)bin.size()));
		buffers[0].uri = pool(embeddedUri);
	}

	// Ensure the destination directory exists before cgltf opens the file.
	FileSystem::getInstance().makeAllDirectories(filePath.getPathOnly());

	cgltf_options options = {};
	options.type = binary ? cgltf_file_type_glb : cgltf_file_type_gltf;

	const std::string filePathStr = wstombs(filePath.getPathNameOS());
	const cgltf_result result = cgltf_write_file(&options, filePathStr.c_str(), &data);
	if (result != cgltf_result_success)
	{
		log::error << L"Failed to write glTF file: " << filePath.getPathNameOS() << L" (error " << (int)result << L")" << Endl;
		return false;
	}

	return true;
}

}
