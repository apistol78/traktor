#include <cmath>
#include <thekla/thekla_atlas.h>
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Model/Model.h"
#include "Model/Operations/UnwrapUV.h"

using namespace Thekla;

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.UnwrapUV", UnwrapUV, IModelOperation)

UnwrapUV::UnwrapUV(int32_t channel)
:	m_channel(channel)
{
}

bool UnwrapUV::apply(Model& model) const
{
	Atlas_Options options;
	atlas_set_default_options(&options);
	options.packer_options.witness.texel_area = 0.5f;
	options.packer_options.witness.conservative = true;

	AlignedVector< Atlas_Input_Vertex > inputVertices;

	const auto& vertices = model.getVertices();
	for (int32_t i = 0; i < int32_t(vertices.size()); ++i)
	{
		const auto& vertex = vertices[i];

		auto& aiv = inputVertices.push_back();

		const auto& p = model.getPosition(vertex.getPosition());
		aiv.position[0] = p.x();
		aiv.position[1] = p.y();
		aiv.position[2] = p.z();

		// \tbd Using normals cause weird artifacts, should probably be face normals?
		// const auto& n = model.getNormal(vertex.getNormal());
		// aiv.normal[0] = n.x();
		// aiv.normal[1] = n.y();
		// aiv.normal[2] = n.z();

		aiv.normal[0] = 0.0f;
		aiv.normal[1] = 0.0f;
		aiv.normal[2] = 0.0f;

		aiv.uv[0] = 0.0f;
		aiv.uv[1] = 0.0f;

		if (vertex.getTexCoordCount() > 0)
		{
			const auto& tc = model.getTexCoord(vertex.getTexCoord(0));
			aiv.uv[0] = tc.x;
			aiv.uv[1] = tc.y;
		}

		aiv.first_colocal = i;
		for (int32_t j = 0; j < i; ++j)
		{
			if (vertices[j].getPosition() == vertex.getPosition())
			{
				aiv.first_colocal = j;
				break;
			}
		}
	}

	AlignedVector< Atlas_Input_Face > inputFaces;
	for (const auto& pol : model.getPolygons())
	{
		auto& aif = inputFaces.push_back();

		aif.vertex_index[0] = pol.getVertex(0);
		aif.vertex_index[1] = pol.getVertex(1);
		aif.vertex_index[2] = pol.getVertex(2);
		aif.material_index = 0;
	}

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

	// Insert normalized texcoords into model.
	{
		AlignedVector< Vertex > vertices;
		for (int32_t i = 0; i < output->vertex_count; ++i)
		{
			const auto& aov = output->vertex_array[i];

			Vector2 uv(aov.uv[0], aov.uv[1]);
			uv -= uvbb.mn;
			uv /= uvbb.mx - uvbb.mn;

			Vertex vx = model.getVertex(aov.xref);
			vx.setTexCoord(m_channel, model.addTexCoord(uv));
			vertices.push_back(vx);
		}

		AlignedVector< Polygon > polygons = model.getPolygons();
		for (int32_t i = 0; i < int32_t(polygons.size()); ++i)
		{
			for (int32_t j = 0; j < 3; ++j)
			{
				T_FATAL_ASSERT(output->index_array[i * 3 + j] < vertices.size());
				polygons[i].setVertex(j, output->index_array[i * 3 + j]);
			}
		}

		model.setVertices(vertices);
		model.setPolygons(polygons);
	}

	atlas_free(output);
	return true;
}

	}
}
