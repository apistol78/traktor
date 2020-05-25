#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"
#include "Json/JsonDocument.h"
#include "Json/JsonObject.h"
#include "Model/Formats/Gltf/ModelFormatGltf.h"
#include "Model/Model.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

bool decodeAsIndices(
	int32_t index,
	json::JsonArray* accessors,
	json::JsonArray* bufferViews,
	const RefArray< IStream >& bufferStreams,
	AlignedVector< int32_t >& outData
)
{
	auto accessor = accessors->get(index).getObject< json::JsonObject >();

	int32_t bufferViewIndex = accessor->getMemberValue(L"bufferView").getInt32();
	int32_t componentType = accessor->getMemberValue(L"componentType").getInt32();
	std::wstring type = accessor->getMemberValue(L"type").getWideString();
	int32_t count = accessor->getMemberValue(L"count").getInt32();

	if (componentType != 5123)	// must be uint16
		return false;

	if (type != L"SCALAR")
		return false;

	auto bufferView = bufferViews->get(bufferViewIndex).getObject< json::JsonObject >();
	if (!bufferView)
		return false;

	int32_t buffer = bufferView->getMemberValue(L"buffer").getInt32();
	int32_t byteOffset = bufferView->getMemberValue(L"byteOffset").getInt32();

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset);

	outData.resize(count);
	for (int32_t i = 0; i < count; ++i)
	{
		uint16_t v;
		if (bufferStream->read(&v, sizeof(v)) != sizeof(v))
			return false;
		outData[i] = int32_t(v);
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

	int32_t bufferViewIndex = accessor->getMemberValue(L"bufferView").getInt32();
	int32_t componentType = accessor->getMemberValue(L"componentType").getInt32();
	std::wstring type = accessor->getMemberValue(L"type").getWideString();
	int32_t count = accessor->getMemberValue(L"count").getInt32();

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

	int32_t buffer = bufferView->getMemberValue(L"buffer").getInt32();
	int32_t byteOffset = bufferView->getMemberValue(L"byteOffset").getInt32();

	IStream* bufferStream = bufferStreams[buffer];
	bufferStream->seek(IStream::SeekSet, byteOffset);

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

Ref< Model > ModelFormatGltf::read(const Path& filePath, const std::wstring& filter, const std::function< Ref< IStream >(const Path&) >& openStream) const
{
	RefArray< IStream > bufferStreams;

	Ref< IStream > stream = openStream(filePath);
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
		Path dirPath(filePath.getPathOnly());

		bufferStreams.resize(buffers->size());
		for (uint32_t i = 0; i < buffers->size(); ++i)
		{
			auto buffer = buffers->get(i).getObject< json::JsonObject >();
			if (!buffer)
				return nullptr;

			std::wstring uri = buffer->getMemberValue(L"uri").getWideString();
			if ((bufferStreams[i] = openStream(dirPath + Path(uri))) == nullptr)
				return nullptr;
		}
	}

	auto bufferViews = docobj->getMemberValue(L"bufferViews").getObject< json::JsonArray >();
	auto accessors = docobj->getMemberValue(L"accessors").getObject< json::JsonArray >();
	auto textures = docobj->getMemberValue(L"textures").getObject< json::JsonArray >();
	auto images = docobj->getMemberValue(L"images").getObject< json::JsonArray >();

	Ref< Model > md = new Model();

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
			auto name = material->getMemberValue(L"name").getWideString();
			mt.setName(name);

			// Normal map
			auto normalTexture = material->getMemberValue(L"normalTexture").getObject< json::JsonObject >();
			if (normalTexture)
			{
				int32_t index = normalTexture->getMemberValue(L"index").getInt32();
				if (!textures || index < 0 || index >= (int32_t)textures->size())
					return nullptr;

				auto texture = textures->get(index).getObject< json::JsonObject >();
				int32_t source = texture->getMemberValue(L"source").getInt32();
				if (!images || source < 0 || source >= (int32_t)images->size())
					return nullptr;

				auto image = images->get(source).getObject< json::JsonObject >();
				auto uri = image->getMemberValue(L"uri").getWideString();
				mt.setNormalMap(Material::Map(uri, 0, true));
			}

			auto pbrMetallicRoughness = material->getMemberValue(L"pbrMetallicRoughness").getObject< json::JsonObject >();
			if (pbrMetallicRoughness)
			{
				auto baseColorTexture = pbrMetallicRoughness->getMemberValue(L"baseColorTexture").getObject< json::JsonObject >();
				if (baseColorTexture)
				{
					int32_t index = baseColorTexture->getMemberValue(L"index").getInt32();
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					auto texture = textures->get(index).getObject< json::JsonObject >();
					int32_t source = texture->getMemberValue(L"source").getInt32();
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					auto image = images->get(source).getObject< json::JsonObject >();
					auto uri = image->getMemberValue(L"uri").getWideString();
					mt.setDiffuseMap(Material::Map(uri, 0, true));
				}

				auto metallicRoughnessTexture = pbrMetallicRoughness->getMemberValue(L"metallicRoughnessTexture").getObject< json::JsonObject >();
				if (metallicRoughnessTexture)
				{
					int32_t index = metallicRoughnessTexture->getMemberValue(L"index").getInt32();
					if (!textures || index < 0 || index >= (int32_t)textures->size())
						return nullptr;

					auto texture = textures->get(index).getObject< json::JsonObject >();
					int32_t source = texture->getMemberValue(L"source").getInt32();
					if (!images || source < 0 || source >= (int32_t)images->size())
						return nullptr;

					auto image = images->get(source).getObject< json::JsonObject >();
					auto uri = image->getMemberValue(L"uri").getWideString();
					mt.setMetalnessMap(Material::Map(uri, 0, true));
					mt.setRoughnessMap(Material::Map(uri, 0, true));
				}
			}

			md->addMaterial(mt);
		}
	}

	// Parse geometry.
	auto meshes = docobj->getMemberValue(L"meshes").getObject< json::JsonArray >();
	if (meshes)
	{
		for (uint32_t i = 0; i < meshes->size(); ++i)
		{
			auto mesh = meshes->get(i).getObject< json::JsonObject >();
			if (!mesh)
				return nullptr;

			auto primitives = mesh->getMemberValue(L"primitives").getObject< json::JsonArray >();
			if (!primitives)
				return nullptr;

			for (uint32_t j = 0; j < primitives->size(); ++j)
			{
				auto prim = primitives->get(j).getObject< json::JsonObject >();
				if (!prim)
					return nullptr;

				int32_t material = prim->getMemberValue(L"material").getInt32();
				int32_t indices = prim->getMemberValue(L"indices").getInt32();

				auto attributes = prim->getMemberValue(L"attributes").getObject< json::JsonObject >();
				if (!attributes)
					return nullptr;

				int32_t position = attributes->getMemberValue(L"POSITION").getInt32();
				int32_t normal = attributes->getMemberValue(L"NORMAL").getInt32();
				int32_t texCoord0 = attributes->getMemberValue(L"TEXCOORD_0").getInt32();

				AlignedVector< int32_t > dataIndices;
				AlignedVector< Vector4 > dataPositions;
				AlignedVector< Vector4 > dataNormals;
				AlignedVector< Vector4 > dataTexCoord0s;

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

				for (uint32_t k = 0; k < dataPositions.size(); ++k)
				{
					Vertex vx;
					vx.setPosition(md->addPosition(dataPositions[k] * Vector4(-1.0f, 1.0f, 1.0f, 0.0f) + Vector4(0.0f, 0.0f, 0.0f, 1.0f)));

					if (dataNormals.size() == dataPositions.size())
						vx.setNormal(md->addUniqueNormal(dataNormals[k] * Vector4(-1.0f, 1.0f, 1.0f, 0.0f)));

					if (dataTexCoord0s.size() == dataPositions.size())
						vx.setTexCoord(0, md->addUniqueTexCoord(
							Vector2(
								dataTexCoord0s[k].x(),
								dataTexCoord0s[k].y()
							)
						));

					md->addVertex(vx);
				}

				for (uint32_t k = 0; k < dataIndices.size(); k += 3)
				{
					Polygon pol;
					pol.setMaterial(material);
					pol.addVertex(dataIndices[k + 0]);
					pol.addVertex(dataIndices[k + 1]);
					pol.addVertex(dataIndices[k + 2]);
					md->addPolygon(pol);
				}
			}
		}
	}

	return md;
}

bool ModelFormatGltf::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
