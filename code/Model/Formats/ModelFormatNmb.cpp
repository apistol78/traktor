/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatNmb.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

#if defined(_MSC_VER)
#	define COMPARE_IGNORE_CASE(a, b) \
	_stricmp(a, b)
#else
#	define COMPARE_IGNORE_CASE(a, b) \
	strcasecmp(a, b)
#endif

#pragma pack(1)

struct NmbChunkHeader
{
	uint16_t type;
	uint16_t unk[3];
};

struct NmbGeometryHeader
{
	uint32_t type;
	uint32_t unk;
	uint32_t nchunks;
};

struct NmbA2VHeader
{
	uint16_t type;
	uint16_t unk[3];
};

struct NmbTrisHeader
{
	uint32_t unkn;
	uint32_t nchunks;
};

struct NmbTrisChunk
{
	uint32_t unkn[2];
	uint32_t type;
	uint32_t unkn2;
	uint32_t nindices;
};

#pragma pack()

Vector2 unpackv2_1f(uint8_t*& ptr)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 1;
	return Vector2(fp[0], 0.0f);
}

Vector2 unpackv2_2f(uint8_t*& ptr)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 2;
	return Vector2(fp[0], fp[1]);
}

Vector2 unpackv2_3f(uint8_t*& ptr)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 3;
	return Vector2(fp[0], fp[1]);
}

Vector2 unpackv2_4f(uint8_t*& ptr)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 4;
	return Vector2(fp[0], fp[1]);
}

Vector4 unpackv4_1f(uint8_t*& ptr, float w)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 1;
	return Vector4(fp[0], 0.0f, 0.0f, w);
}

Vector4 unpackv4_2f(uint8_t*& ptr, float w)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 2;
	return Vector4(fp[0], fp[1], 0.0f, w);
}

Vector4 unpackv4_3f(uint8_t*& ptr, float w)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 3;
	return Vector4(fp[0], fp[1], fp[2], w);
}

Vector4 unpackv4_4f(uint8_t*& ptr, float w)
{
	float* fp = reinterpret_cast< float* >(ptr); ptr += sizeof(float) * 4;
	return Vector4(fp[0], fp[1], fp[2], fp[3]);
}

struct ModelBase
{
	uint32_t vertexBase;

	ModelBase()
	:	vertexBase(c_InvalidIndex)
	{
	}
};

uint8_t* parseA2V(Model& model, uint8_t* ptr, ModelBase& base)
{
	NmbA2VHeader* header = reinterpret_cast< NmbA2VHeader* >(ptr);
	ptr += sizeof(NmbA2VHeader);

	int32_t nameLength = *reinterpret_cast< int32_t* >(ptr);
	ptr += sizeof(int32_t);

	char* name = reinterpret_cast< char* >(ptr); name[nameLength] = 0;
	ptr += nameLength + 12;

	int32_t count = *reinterpret_cast< int32_t* >(ptr);
	ptr += sizeof(int32_t);

	log::info << L"Attribute \"" << mbstows(name) << L"\"; " << count << L" value(s)" << Endl;

	std::vector< Vertex > vertices = model.getVertices();
	if (base.vertexBase == c_InvalidIndex)
	{
		base.vertexBase = uint32_t(vertices.size());
		vertices.resize(vertices.size() + count);
	}

	Vector2 (*unpackv2)(uint8_t*& ptr) = 0;
	Vector4 (*unpackv4)(uint8_t*& ptr, float w) = 0;
	switch (header->type & 7)
	{
	case 0:
		unpackv2 = &unpackv2_1f;
		unpackv4 = &unpackv4_1f;
		break;
	case 1:
		unpackv2 = &unpackv2_2f;
		unpackv4 = &unpackv4_2f;
		break;
	case 2:
		unpackv2 = &unpackv2_3f;
		unpackv4 = &unpackv4_3f;
		break;
	case 3:
		unpackv2 = &unpackv2_4f;
		unpackv4 = &unpackv4_4f;
		break;
	case 4:
		unpackv2 = &unpackv2_2f;
		unpackv4 = &unpackv4_2f;
		break;
	case 5:
		unpackv2 = &unpackv2_3f;
		unpackv4 = &unpackv4_3f;
		break;
	default:
		log::error << L"Unknown data packing type" << Endl;
		return 0;
	}

	if (COMPARE_IGNORE_CASE(name, "a2v.objCoord") == 0 || COMPARE_IGNORE_CASE(name, "a2v.worldCoord") == 0)
	{
		for (int32_t i = 0; i < count; ++i)
			vertices[base.vertexBase + i].setPosition(model.addPosition(unpackv4(ptr, 1.0f) / Vector4(100.0f, 100.0f, 100.0f, 1.0f)));
	}
	else if (COMPARE_IGNORE_CASE(name, "a2v.objNormal") == 0 || COMPARE_IGNORE_CASE(name, "a2v.worldNormal") == 0)
	{
		for (int32_t i = 0; i < count; ++i)
			vertices[base.vertexBase + i].setNormal(model.addNormal(unpackv4(ptr, 0.0f)));
	}
	else if (COMPARE_IGNORE_CASE(name, "a2v.objTangent") == 0 || COMPARE_IGNORE_CASE(name, "a2v.worldTangent") == 0)
	{
		for (int32_t i = 0; i < count; ++i)
			vertices[base.vertexBase + i].setTangent(model.addNormal(unpackv4(ptr, 0.0f)));
	}
	else if (COMPARE_IGNORE_CASE(name, "a2v.objBinormal") == 0 || COMPARE_IGNORE_CASE(name, "a2v.worldBinormal") == 0)
	{
		for (int32_t i = 0; i < count; ++i)
			vertices[base.vertexBase + i].setBinormal(model.addNormal(unpackv4(ptr, 0.0f)));
	}
	else if (COMPARE_IGNORE_CASE(name, "a2v.c_texCoord") == 0 || COMPARE_IGNORE_CASE(name, "a2v.tex") == 0)
	{
		for (int32_t i = 0; i < count; ++i)
			vertices[base.vertexBase + i].setTexCoord(0, model.addTexCoord(unpackv2(ptr) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f)));
	}
	else
	{
		log::error << L"Unsupported vertex attribute \"" << mbstows(name) << L"\"" << Endl;
		const int32_t c_packingSize[] = { 1 * 4, 2 * 4, 3 * 4, 4 * 4, 2 * 4, 3 * 4, 0, 0 };
		T_ASSERT (c_packingSize[header->type & 7] != 0);
		ptr += count * c_packingSize[header->type & 7];
	}

	model.setVertices(vertices);

	ptr += 4;
	return ptr;
}

uint8_t* parseTriangles(Model& model, uint8_t* ptr, int32_t materialId, const ModelBase& base)
{
	enum
	{
		NmbTriangles = 2,
		NmbTriangleStrip = 4
	};

	NmbTrisHeader* header = reinterpret_cast< NmbTrisHeader* >(ptr);
	ptr += sizeof(NmbTrisHeader);

	int32_t nchunks = header->nchunks;
	while (nchunks--)
	{
		NmbTrisChunk* chunk = reinterpret_cast< NmbTrisChunk* >(ptr);
		ptr += sizeof(NmbTrisChunk);

		if (chunk->type & NmbTriangleStrip)
		{
			uint16_t* indices = reinterpret_cast< uint16_t* >(ptr);

			uint32_t count;
			uint32_t gid;

			for (gid = 0; gid < chunk->nindices; ++gid)
			{
				for (count = 0; indices[count] != 0xffff && indices[count] != 0xfffc; count++)
					gid++;

				uint16_t last[3];
				last[0] = *indices++;
				last[1] = *indices++;

				for (uint32_t i = 2; i < count; ++i)
				{
					last[2] = *indices++;

					Polygon polygon(
						materialId,
						base.vertexBase + last[2],
						base.vertexBase + last[1],
						base.vertexBase + last[0]
					);

					last[0] = last[1];
					last[1] = last[2];

					if ((i & 1) != 0)
						polygon.flipWinding();

					model.addPolygon(polygon);
				}

				indices++;
			}
		}
		else if (chunk->type & NmbTriangles)
		{
			uint16_t* indices = reinterpret_cast< uint16_t* >(ptr);
			for (uint32_t i = 0; i < chunk->nindices; i += 3)
			{
				Polygon polygon(
					materialId,
					base.vertexBase + indices[i + 2],
					base.vertexBase + indices[i + 1],
					base.vertexBase + indices[i + 0]
				);
				model.addPolygon(polygon);
			}
		}
		else
			log::error << L"Unknown primitive type" << Endl;

		ptr += chunk->nindices * sizeof(uint16_t);
		ptr += sizeof(uint32_t);
	}

	return ptr;
}

uint8_t* parseChunk(Model& model, uint8_t* ptr)
{
	NmbChunkHeader* header = reinterpret_cast< NmbChunkHeader* >(ptr);
	ptr += sizeof(NmbChunkHeader);

	int32_t nameLength = *reinterpret_cast< int32_t* >(ptr);
	ptr += sizeof(int32_t);

	char* name = reinterpret_cast< char* >(ptr); name[nameLength] = 0;
	ptr += nameLength;

	log::info << L"Chunk \"" << mbstows(name) << L"\"" << Endl;

	Material material(mbstows(name));
	int32_t materialId = model.addMaterial(material);

	ptr += 1;
	ptr += 8;

	if (header->type == 0x31)
	{
		ModelBase base;

		// Vertex attributes and data.
		int32_t a2v_count = *reinterpret_cast< int32_t* >(ptr);
		ptr += sizeof(int32_t);

		while (a2v_count--)
		{
			ptr = parseA2V(model, ptr, base);
			if (!ptr)
				return 0;
		}

		// Triangles.
		ptr = parseTriangles(model, ptr, materialId, base);
	}
	else
	{
		log::error << L"Unknown chunk type " << int32_t(header->type) << Endl;
		return 0;
	}

	ptr += 4;

	return ptr;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatNmb", 0, ModelFormatNmb, ModelFormat)

void ModelFormatNmb::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Nvidia Model Binary";
	outExtensions.push_back(L"nmb");
}

bool ModelFormatNmb::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"nmb") == 0;
}

Ref< Model > ModelFormatNmb::read(IStream* stream, uint32_t importFlags) const
{
	// Read entire file into memory.
	int32_t nbytes = stream->available();

	AutoPtr< uint8_t > buffer(new uint8_t [nbytes]);
	if (stream->read(buffer.ptr(), nbytes) != nbytes)
		return 0;

	stream->close();

	Ref< Model > model = new Model();

	uint8_t* ptr = buffer.ptr();

	NmbGeometryHeader* header = reinterpret_cast< NmbGeometryHeader* >(ptr);
	ptr += sizeof(NmbGeometryHeader);

	for (uint32_t i = 0; i < header->nchunks; ++i)
	{
		ptr = parseChunk(*model, ptr);
		if (!ptr)
			return 0;

		T_ASSERT (ptr <= buffer.ptr() + nbytes);
	}

	return model;
}

bool ModelFormatNmb::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
