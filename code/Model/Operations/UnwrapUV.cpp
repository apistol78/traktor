#include <cmath>
#include <thekla/thekla_atlas.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Model/Operations/UnwrapUV.h"

using namespace Thekla;

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.UnwrapUV", UnwrapUV, IModelOperation)

UnwrapUV::UnwrapUV(int32_t channel, uint32_t textureSize)
:	m_channel(channel)
,	m_textureSize(textureSize)
{
}

bool UnwrapUV::apply(Model& model) const
{
	Atlas_Options options;
	atlas_set_default_options(&options);
	options.packer_options.witness.packing_quality = 1;
	options.packer_options.witness.texel_area = 128.0f / (float)m_textureSize;
	//options.packer_options.witness.conservative = true;

	// Build input vertices to atlas generator.
	AlignedVector< Atlas_Input_Vertex > inputVertices;
	for (int32_t i = 0; i < (int32_t)model.getVertexCount(); ++i)
	{
		auto& aiv = inputVertices.push_back();

		const Vertex& vertex = model.getVertex(i);
		const Vector4& p = model.getPosition(vertex.getPosition());
		aiv.position[0] = p.x();
		aiv.position[1] = p.y();
		aiv.position[2] = p.z();

		if (vertex.getNormal() != c_InvalidIndex)
		{
			const Vector4& n = model.getNormal(vertex.getNormal());
			aiv.normal[0] = n.x();
			aiv.normal[1] = n.y();
			aiv.normal[2] = n.z();
		}
		else
		{
			aiv.normal[0] = 0.0f;
			aiv.normal[1] = 0.0f;
			aiv.normal[2] = 0.0f;
		}

		if (vertex.getTexCoord(m_channel) != c_InvalidIndex)
		{
			const Vector2& tc = model.getTexCoord(vertex.getTexCoord(m_channel));
			aiv.uv[0] = tc.x * m_textureSize;
			aiv.uv[1] = tc.y * m_textureSize;
		}
		else
		{
			aiv.uv[0] = 0.0f;
			aiv.uv[1] = 0.0f;
		}

		aiv.first_colocal = i;
		for (int32_t j = 0; j < i; ++j)
		{
			if (model.getVertex(j).getPosition() == vertex.getPosition())
			{
				aiv.first_colocal = j;
				break;
			}
		}
	}

	// Build input faces to atlas generator.
	AlignedVector< Atlas_Input_Face > inputFaces;
	SmallMap< uint32_t, uint32_t > vertexToPolygon;

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const Polygon& polygon = model.getPolygon(i);

		auto& aif = inputFaces.push_back();
		aif.vertex_index[0] = polygon.getVertex(0);
		aif.vertex_index[1] = polygon.getVertex(1);
		aif.vertex_index[2] = polygon.getVertex(2);
		aif.material_index = 0;

		vertexToPolygon[aif.vertex_index[0]] = i;
		vertexToPolygon[aif.vertex_index[1]] = i;
		vertexToPolygon[aif.vertex_index[2]] = i;
	}

	// Generate UV atlas.
	Atlas_Input_Mesh input;
    input.vertex_count = (int)inputVertices.size();
    input.vertex_array = inputVertices.ptr();
    input.face_count = (int)inputFaces.size();
    input.face_array = inputFaces.ptr();

	Atlas_Error error = Atlas_Error_Success;
	Atlas_Output_Mesh* output = atlas_generate(&input, &options, &error);
	if (!output || error != Atlas_Error_Success)
		return false;

	// Calculate bounding box of generated texcoords.
	Aabb2 uvbb;
	for (int32_t i = 0; i < output->vertex_count; ++i)
	{
		const auto& aov = output->vertex_array[i];
		uvbb.contain(Vector2(aov.uv[0], aov.uv[1]));
	}
	uvbb.mn -= Vector2(1.0f, 1.0f);
	uvbb.mx += Vector2(1.0f, 1.0f);

	// Insert normalized texcoords into model.
	{
		AlignedVector< Polygon > originalPolygons = model.getPolygons();

		model.clear(Model::CfPolygons);

		for (int32_t i = 0; i < output->index_count; i += 3)
		{
			Polygon polygon = originalPolygons[vertexToPolygon[
				output->vertex_array[output->index_array[i]].xref
			]];

			for (int32_t j = 0; j < 3; ++j)
			{
				int32_t index = output->index_array[i + j];
				const auto& aov = output->vertex_array[index];

				Vertex vx = model.getVertex(aov.xref);

				Vector2 uv(aov.uv[0], aov.uv[1]);
				uv -= uvbb.mn;
				uv /= uvbb.mx - uvbb.mn;

				uv.x = (std::floor(uv.x * m_textureSize)) / m_textureSize;
				uv.y = (std::floor(uv.y * m_textureSize)) / m_textureSize;

				vx.setTexCoord(m_channel, model.addTexCoord(uv));

				polygon.setVertex(j, model.addUniqueVertex(vx));
			}

			model.addPolygon(polygon);
		}
	}

	atlas_free(output);
	return true;
}

void UnwrapUV::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"channel", m_channel);
	s >> Member< uint32_t >(L"textureSize", m_textureSize);
}

	}
}
