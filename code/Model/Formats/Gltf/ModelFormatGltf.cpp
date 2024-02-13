/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Json/JsonDocument.h"
#include "Json/JsonObject.h"
#include "Model/Formats/Gltf/ModelFormatGltf.h"
#include "Model/Model.h"

// https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/figures/gltfOverview-2.0.0b.png

namespace traktor::model
{
	namespace
	{

std::wstring decodeString(const std::wstring& text)
{
	std::vector< wchar_t > chrs;
	for (size_t i = 0; i < text.length(); )
	{
		if (text[i] == L'%' && i < text.length() - 2)
		{
			char hex[] = { char(text[i + 1]), char(text[i + 2]), 0 };
			chrs.push_back(wchar_t(std::strtol(hex, 0, 16)));
			i += 3;
		}
		else
		{
			chrs.push_back(wchar_t(text[i]));
			i++;
		}
	}
	return std::wstring(chrs.begin(), chrs.end());
}

Matrix44 parseTransform(const json::JsonObject* node)
{
	Matrix44 transform = Matrix44::identity();
	if (node->getMember(L"translation") != nullptr)
	{
		auto translation = node->getMemberValue(L"translation").getObject< json::JsonArray >();
		if (translation != nullptr && translation->size() >= 3)
		{
			transform = transform * translate(
				translation->get(0).getFloat(),
				translation->get(1).getFloat(),
				translation->get(2).getFloat()
			);
		}
	}
	if (node->getMember(L"rotation") != nullptr)
	{
		auto rotation = node->getMemberValue(L"rotation").getObject< json::JsonArray >();
		if (rotation != nullptr && rotation->size() >= 4)
		{
			transform = transform * Quaternion(
				rotation->get(0).getFloat(),
				rotation->get(1).getFloat(),
				rotation->get(2).getFloat(),
				rotation->get(3).getFloat()
			).toMatrix44();
		}
	}
	if (node->getMember(L"scale") != nullptr)
	{
		auto scale = node->getMemberValue(L"scale").getObject< json::JsonArray >();
		if (scale != nullptr && scale->size() >= 3)
		{
			transform = transform * traktor::scale(
				scale->get(0).getFloat(),
				scale->get(1).getFloat(),
				scale->get(2).getFloat()
			);
		}
	}
	if (node->getMember(L"matrix") != nullptr)
	{
		auto mx = node->getMemberValue(L"scale").getObject< json::JsonArray >();
		if (mx != nullptr && mx->size() >= 4 * 4)
		{
			transform = transform * Matrix44(
				mx->get( 0).getFloat(), mx->get (1).getFloat(), mx->get( 2).getFloat(), mx->get( 3).getFloat(),
				mx->get( 4).getFloat(), mx->get( 5).getFloat(), mx->get( 6).getFloat(), mx->get( 7).getFloat(),
				mx->get( 8).getFloat(), mx->get( 9).getFloat(), mx->get(10).getFloat(), mx->get(11).getFloat(),
				mx->get(12).getFloat(), mx->get(13).getFloat(), mx->get(14).getFloat(), mx->get(15).getFloat()
			);
		}
	}
	return transform;
}

bool decodeAsIndices(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< uint32_t >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	const int32_t bufferViewIndex = accessor->getMemberInt32(L"bufferView", 0);
	const int32_t accessorByteOffset = accessor->getMemberInt32(L"byteOffset", 0);
	const int32_t componentType = accessor->getMemberInt32(L"componentType", 0);
	const std::wstring type = accessor->getMemberString(L"type", L"");
	const int32_t count = accessor->getMemberInt32(L"count", 0);

	if (componentType != 5123 && componentType != 5125)
		return false;

	if (type != L"SCALAR")
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
	const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);
	const int32_t byteStride = bufferView->getMemberInt32(L"byteStride", (componentType == 5123) ? 2 : 4);

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset + accessorByteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		if (componentType == 5123)
		{
			uint16_t v;
			if (bufferStream->read(&v, sizeof(v)) != sizeof(v))
				return false;
			outData[i] = (uint32_t)v;
		}
		else // 5125
		{
			uint32_t v;
			if (bufferStream->read(&v, sizeof(v)) != sizeof(v))
				return false;
			outData[i] = (uint32_t)v;
		}
	}

	return true;
}

bool decodeAsScalars(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< float >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	const int32_t bufferViewIndex = accessor->getMemberInt32(L"bufferView", 0);
	const int32_t accessorByteOffset = accessor->getMemberInt32(L"byteOffset", 0);
	const int32_t componentType = accessor->getMemberInt32(L"componentType", 0);
	const std::wstring type = accessor->getMemberString(L"type", L"");
	const int32_t count = accessor->getMemberInt32(L"count", 0);

	if (componentType != 5126)	// must be float
		return false;

	if (type != L"SCALAR")
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
	const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset + accessorByteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		float v;
		if (bufferStream->read(&v, sizeof(v)) != sizeof(v))
			return false;
		outData[i] = v;
	}

	return true;
}

bool decodeAsVectors(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< Vector4 >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	const int32_t bufferViewIndex = accessor->getMemberInt32(L"bufferView", 0);
	const int32_t accessorByteOffset = accessor->getMemberInt32(L"byteOffset", 0);
	const int32_t componentType = accessor->getMemberInt32(L"componentType", 0);
	const std::wstring type = accessor->getMemberString(L"type", L"");
	const int32_t count = accessor->getMemberInt32(L"count", 0);

	if (componentType != 5126)	// must be float
		return false;

	int32_t width = 0;
	if (type == L"SCALAR")
		width = 1;
	else if (type == L"VEC2")
		width = 2;
	else if (type == L"VEC3")
		width = 3;
	else if (type == L"VEC4")
		width = 4;
	else
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
	const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset + accessorByteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		float v[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		if (bufferStream->read(v, width * sizeof(float)) != width * sizeof(float))
			return false;
		outData[i] = Vector4::loadUnaligned(v);
	}

	return true;
}

std::wstring getImageName(const json::JsonObject* image)
{
	std::wstring name;
	
	if (!(name = image->getMemberString(L"name")).empty())
		return name;
	if (!(name = image->getMemberString(L"uri")).empty())
		return name;

	return L"";
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatGltf", 0, ModelFormatGltf, ModelFormat)

void ModelFormatGltf::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"glTF Object";
	outExtensions.push_back(L"gltf");
}

bool ModelFormatGltf::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"gltf") == 0;
}

Ref< Model > ModelFormatGltf::read(const Path& filePath, const std::wstring& filter) const
{
	const Path dirPath(filePath.getPathOnly());
	RefArray< IStream > bufferStreams;

	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return nullptr;

	json::JsonDocument doc;
	if (!doc.loadFromStream(stream))
		return nullptr;

	if (doc.empty())
		return nullptr;
	if (!doc.front().isObject< json::JsonObject >())
		return nullptr;

	auto docobj = doc.front().getObjectUnsafe< json::JsonObject >();
	T_ASSERT(docobj != nullptr);

	// Create buffer streams.
	auto buffers = docobj->getMemberValue(L"buffers").getObject< json::JsonArray >();
	if (buffers)
	{
		bufferStreams.resize(buffers->size());
		for (uint32_t i = 0; i < buffers->size(); ++i)
		{
			auto buffer = buffers->get(i).getObject< json::JsonObject >();
			if (!buffer)
				return nullptr;

			const std::wstring uri = buffer->getMemberValue(L"uri").getWideString();
			if (startsWith(uri, L"data:application/octet-stream;base64,"))
			{
				AlignedVector< uint8_t > data = Base64().decode(uri.substr(37));
				Ref< DynamicMemoryStream > ms = new DynamicMemoryStream(true, false);
				ms->getBuffer().swap(data);
				bufferStreams[i] = ms;
			}
			else
			{
				if ((bufferStreams[i] = FileSystem::getInstance().open(dirPath + Path(decodeString(uri)), File::FmRead)) == nullptr)
					return nullptr;
			}
		}
	}

	auto bufferViews = docobj->getMemberValue(L"bufferViews").getObject< json::JsonArray >();
	auto accessors = docobj->getMemberValue(L"accessors").getObject< json::JsonArray >();
	auto textures = docobj->getMemberValue(L"textures").getObject< json::JsonArray >();
	auto images = docobj->getMemberValue(L"images").getObject< json::JsonArray >();

	Ref< Model > md = new Model();
	md->addUniqueTexCoordChannel(L"UV0");

	// Parse materials.
	auto materials = docobj->getMemberValue(L"materials").getObject< json::JsonArray >();
	if (materials)
	{
		for (uint32_t i = 0; i < materials->size(); ++i)
		{
			auto material = materials->get(i).getObject< json::JsonObject >();
			if (!material)
				return nullptr;

			Material mt;

			// Name
			const auto name = material->getMemberValue(L"name").getWideString();
			mt.setName(name);

			// Normal map
			const auto normalTexture = material->getMemberValue(L"normalTexture").getObject< json::JsonObject >();
			if (normalTexture)
			{
				const int32_t index = normalTexture->getMemberInt32(L"index", -1);
				if (!textures || index < 0 || index >= (int32_t)textures->size())
					return nullptr;

				const auto texture = textures->get(index).getObject< json::JsonObject >();
				const int32_t source = texture->getMemberInt32(L"source", -1);
				if (!images || source < 0 || source >= (int32_t)images->size())
					return nullptr;

				const auto image = images->get(source).getObject< json::JsonObject >();
				if (!image)
					return nullptr;

				const std::wstring name = getImageName(image);
				if (name.empty())
					return nullptr;

				Material::Map normalMap(name, L"UV0", true);

				// Embedded texture.
				const int32_t bufferViewIndex = image->getMemberInt32(L"bufferView", -1);
				if (bufferViewIndex >= 0)
				{
					auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
					if (!bufferView)
						return nullptr;

					const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
					const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);

					IStream* bufferStream = bufferStreams[buffer];
					bufferStream->seek(IStream::SeekSet, byteOffset);

					normalMap.image = drawing::Image::load(bufferStream, L"png");
				}

				const std::wstring uri = image->getMemberString(L"uri");
				if (!uri.empty() && !normalMap.image)
				{
					normalMap.image = drawing::Image::load(dirPath + Path(uri));
				}

				mt.setNormalMap(normalMap);
			}

			// Extract maps from PBR material setup.
			const auto pbrMetallicRoughness = material->getMemberValue(L"pbrMetallicRoughness").getObject< json::JsonObject >();
			if (pbrMetallicRoughness)
			{
				const float metallicFactor = pbrMetallicRoughness->getMemberFloat(L"metallicFactor", 1.0f);
				const float roughnessFactor = pbrMetallicRoughness->getMemberFloat(L"roughnessFactor", 0.5f);

				const auto baseColorTexture = pbrMetallicRoughness->getMemberValue(L"baseColorTexture").getObject< json::JsonObject >();
				if (baseColorTexture)
				{
					const int32_t index = baseColorTexture->getMemberInt32(L"index", -1);
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					const auto texture = textures->get(index).getObject< json::JsonObject >();
					const int32_t source = texture->getMemberInt32(L"source", -1);
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					const auto image = images->get(source).getObject< json::JsonObject >();
					if (!image)
						return nullptr;

					const std::wstring name = getImageName(image);
					if (name.empty())
						return nullptr;

					Material::Map diffuseMap(name, L"UV0", true);

					// Embedded texture.
					const int32_t bufferViewIndex = image->getMemberInt32(L"bufferView", -1);
					if (bufferViewIndex >= 0)
					{
						auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
						if (!bufferView)
							return nullptr;

						const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
						const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);

						IStream* bufferStream = bufferStreams[buffer];
						bufferStream->seek(IStream::SeekSet, byteOffset);

						diffuseMap.image = drawing::Image::load(bufferStream, L"png");
					}

					const std::wstring uri = image->getMemberString(L"uri");
					if (!uri.empty() && !diffuseMap.image)
					{
						diffuseMap.image = drawing::Image::load(dirPath + Path(uri));
					}

					mt.setDiffuseMap(diffuseMap);
				}

				const auto metallicRoughnessTexture = pbrMetallicRoughness->getMemberValue(L"metallicRoughnessTexture").getObject< json::JsonObject >();
				if (metallicRoughnessTexture)
				{
					const int32_t index = metallicRoughnessTexture->getMemberInt32(L"index", -1);
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					const auto texture = textures->get(index).getObject< json::JsonObject >();
					const int32_t source = texture->getMemberInt32(L"source", -1);
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					const auto image = images->get(source).getObject< json::JsonObject >();
					if (!image)
						return nullptr;

					const std::wstring name = getImageName(image);
					if (name.empty())
						return nullptr;

					Material::Map metallicMap(name + L"_M", L"UV0", true);
					Material::Map roughnessMap(name + L"_R", L"UV0", true);

					// Embedded texture.
					Ref< drawing::Image > metallicRoughness;

					const int32_t bufferViewIndex = image->getMemberInt32(L"bufferView", -1);
					if (bufferViewIndex >= 0)
					{
						auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
						if (!bufferView)
							return nullptr;

						const int32_t buffer = bufferView->getMemberInt32(L"buffer", 0);
						const int32_t byteOffset = bufferView->getMemberInt32(L"byteOffset", 0);

						IStream* bufferStream = bufferStreams[buffer];
						bufferStream->seek(IStream::SeekSet, byteOffset);

						metallicRoughness = drawing::Image::load(bufferStream, L"png");
					}

					const std::wstring uri = image->getMemberString(L"uri");
					if (!uri.empty() && !roughnessMap.image)
					{
						metallicRoughness = drawing::Image::load(dirPath + Path(uri));
					}

					if (metallicRoughness)
					{
						const drawing::SwizzleFilter b(L"BBBB");
						metallicMap.image = metallicRoughness->clone();
						metallicMap.image->apply(&b);
						metallicMap.image->convert(drawing::PixelFormat::getR8());

						const drawing::SwizzleFilter g(L"GGGG");
						roughnessMap.image = metallicRoughness->clone();
						roughnessMap.image->apply(&g);
						roughnessMap.image->convert(drawing::PixelFormat::getR8());

						metallicRoughness = nullptr;
					}

					mt.setMetalnessMap(metallicMap);
					mt.setRoughnessMap(roughnessMap);
				}

				mt.setMetalness(metallicFactor);
				mt.setRoughness(roughnessFactor);

				const auto baseColorFactor = pbrMetallicRoughness->getMemberValue(L"baseColorFactor").getObject< json::JsonArray >();
				if (baseColorFactor != nullptr && baseColorFactor->size() >= 4)
				{
					mt.setColor(Color4f(
						baseColorFactor->get(0).getFloat(),
						baseColorFactor->get(1).getFloat(),
						baseColorFactor->get(2).getFloat(),
						baseColorFactor->get(3).getFloat()
					));
				}
			}

			md->addMaterial(mt);
		}
	}

	auto nodes = docobj->getMemberValue(L"nodes").getObject< json::JsonArray >();
	if (!nodes)
		return nullptr;


	//// Calculate global transforms of each node.
	//auto scenes = docobj->getMemberValue(L"scenes").getObject< json::JsonArray >();
	//if (!scenes || scenes->size() != 1)
	//	return nullptr;

	//const auto scene = scenes->get(0).getObject< json::JsonObject >();
	//scene->getMemberInt32(L"nodes");


	const Matrix44 Tpost(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Parse skeleton.
	auto skins = docobj->getMemberValue(L"skins").getObject< json::JsonArray >();
	if (skins && skins->size() > 0)
	{
		const auto skin = skins->get(0).getObject< json::JsonObject >();
		const auto joints = skin->getMemberValue(L"joints").getObject< json::JsonArray >();

		AlignedVector< Joint > modelJoints;
		SmallMap< int32_t, int32_t > jointMap;

		for (uint32_t i = 0; i < joints->size(); ++i)
		{
			const int32_t jointIndex = joints->get(i).getInt32();

			const auto jointNode = nodes->get(jointIndex).getObject< json::JsonObject >();
			if (!jointNode)
				return nullptr;

			const std::wstring name = jointNode->getMemberValue(L"name").getWideString();
			const Matrix44 Tnode = Tpost * parseTransform(jointNode);

			Joint& jnt = modelJoints.push_back();
			jnt.setName(name);
			jnt.setTransform(Transform(Tnode));

			jointMap[jointIndex] = (int32_t)(modelJoints.size());
		}

		// Update relationship between joints.
		for (uint32_t i = 0; i < joints->size(); ++i)
		{
			const int32_t jointIndex = joints->get(i).getInt32();
			T_ASSERT(jointMap[jointIndex] == (int32_t)i);

			const auto jointNode = nodes->get(jointIndex).getObject< json::JsonObject >();
			if (!jointNode)
				return nullptr;

			const auto children = jointNode->getMemberValue(L"children").getObject< json::JsonArray >();
			if (!children)
				continue;

			for (uint32_t j = 0; j < children->size(); ++j)
			{
				const int32_t childIndex = children->get(j).getInt32();
				const int32_t childModelJoint = jointMap[childIndex];
				modelJoints[childModelJoint].setParent(i);
			}
		}

		md->setJoints(modelJoints);
	}

	// Parse geometry.
	auto meshes = docobj->getMemberValue(L"meshes").getObject< json::JsonArray >();
	if (meshes)
	{
		for (uint32_t i = 0; i < nodes->size(); ++i)
		{
			const auto node = nodes->get(i).getObject< json::JsonObject >();
			if (!node)
				return nullptr;

			if (node->getMember(L"mesh") == nullptr)
				continue;

			const int32_t meshIndex = node->getMemberInt32(L"mesh", -1);
			if (meshIndex < 0 || meshIndex >= (int32_t)meshes->size())
				continue;

			const auto mesh = meshes->get(meshIndex).getObject< json::JsonObject >();
			if (!mesh)
				return nullptr;

			const Matrix44 Tnode = Tpost * parseTransform(node);

			const auto primitives = mesh->getMemberValue(L"primitives").getObject< json::JsonArray >();
			if (!primitives)
				return nullptr;

			const uint32_t vertexBase = md->getVertexCount();

			for (uint32_t j = 0; j < primitives->size(); ++j)
			{
				const auto prim = primitives->get(j).getObject< json::JsonObject >();
				if (!prim)
					return nullptr;

				const int32_t material = prim->getMemberInt32(L"material", 0);
				const int32_t indices = prim->getMemberInt32(L"indices", 0);
				const int32_t mode = prim->getMemberInt32(L"mode", 4);

				// Must be triangles; which is the default if no mode is given.
				if (mode != 4)
					return nullptr;

				const auto attributes = prim->getMemberValue(L"attributes").getObject< json::JsonObject >();
				if (!attributes)
					return nullptr;

				const int32_t position = attributes->getMemberInt32(L"POSITION", 0);
				const int32_t normal = attributes->getMemberInt32(L"NORMAL", 0);
				const int32_t texCoord0 = attributes->getMemberInt32(L"TEXCOORD_0", 0);
				const int32_t joints0 = attributes->getMemberInt32(L"JOINTS_0", 0);
				const int32_t weights0 = attributes->getMemberInt32(L"WEIGHTS_0", 0);

				AlignedVector< uint32_t > dataIndices;
				AlignedVector< Vector4 > dataPositions;
				AlignedVector< Vector4 > dataNormals;
				AlignedVector< Vector4 > dataTexCoord0s;
				AlignedVector< Vector4 > dataJoints;
				AlignedVector< Vector4 > dataWeights;

				decodeAsIndices(
					indices,
					accessors,
					bufferViews,
					bufferStreams,
					dataIndices
				);
				decodeAsVectors(
					position,
					accessors,
					bufferViews,
					bufferStreams,
					dataPositions
				);
				decodeAsVectors(
					normal,
					accessors,
					bufferViews,
					bufferStreams,
					dataNormals
				);
				decodeAsVectors(
					texCoord0,
					accessors,
					bufferViews,
					bufferStreams,
					dataTexCoord0s
				);
				decodeAsVectors(
					joints0,
					accessors,
					bufferViews,
					bufferStreams,
					dataJoints
				);
				decodeAsVectors(
					weights0,
					accessors,
					bufferViews,
					bufferStreams,
					dataWeights
				);

				for (uint32_t k = 0; k < dataPositions.size(); ++k)
				{
					Vertex vx;
					vx.setPosition(md->addPosition(
						Tnode * dataPositions[k].xyz1()
					));

					if (dataNormals.size() == dataPositions.size())
						vx.setNormal(md->addUniqueNormal(
							Tnode * dataNormals[k].xyz0()
						));

					if (dataTexCoord0s.size() == dataPositions.size())
						vx.setTexCoord(0, md->addUniqueTexCoord(
							Vector2(
								dataTexCoord0s[k].x(),
								dataTexCoord0s[k].y()
							)
						));

					if (dataJoints.size() == dataPositions.size())
					{
						int32_t T_MATH_ALIGN16 jointIndices[4];
						dataJoints[k].storeIntegersAligned(jointIndices);

						float T_MATH_ALIGN16 jointWeights[4];
						dataWeights[k].storeAligned(jointWeights);

						for (int32_t ii = 0; ii < 4; ++ii)
						{
							if (jointWeights[ii] > FUZZY_EPSILON)
								vx.setJointInfluence(jointIndices[ii], jointWeights[ii]);
						}
					}

					md->addVertex(vx);
				}

				for (uint32_t k = 0; k < dataIndices.size(); k += 3)
				{
					Polygon pol;
					pol.setMaterial(material);
					pol.addVertex(vertexBase + dataIndices[k + 0]);
					pol.addVertex(vertexBase + dataIndices[k + 1]);
					pol.addVertex(vertexBase + dataIndices[k + 2]);
					md->addPolygon(pol);
				}
			}
		}
	}

	return md;
}

bool ModelFormatGltf::write(const Path& filePath, const Model* model) const
{
	return false;
}

}
