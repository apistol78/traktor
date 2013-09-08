#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct TangentBase
{
	Vector4 normal;
	Vector4 tangent;
	Vector4 binormal;

	TangentBase()
	:	normal(0.0f, 0.0f, 0.0f, 0.0f)
	,	tangent(0.0f, 0.0f, 0.0f, 0.0f)
	,	binormal(0.0f, 0.0f, 0.0f, 0.0f)
	{
	}
};

bool findBaseIndex(const Model& model, const Polygon& polygon, uint32_t& outBaseIndex)
{
	outBaseIndex = c_InvalidIndex;

	const std::vector< uint32_t >& vertices = polygon.getVertices();
	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		const Vertex* v[] =
		{
			&model.getVertex(vertices[i]),
			&model.getVertex(vertices[(i + 1) % vertices.size()]),
			&model.getVertex(vertices[(i + 2) % vertices.size()])
		};

		Vector4 p[] =
		{
			model.getPosition(v[0]->getPosition()),
			model.getPosition(v[1]->getPosition()),
			model.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		if (ep[0].length() > FUZZY_EPSILON && ep[1].length() > FUZZY_EPSILON && cross(ep[0], ep[1]).length() > FUZZY_EPSILON)
		{
			outBaseIndex = i;
			return true;
		}
	}

	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CalculateTangents", CalculateTangents, IModelOperation)

bool CalculateTangents::apply(Model& model) const
{
	const std::vector< Polygon >& polygons = model.getPolygons();
	AlignedVector< TangentBase > polygonTangentBases;
	AlignedVector< TangentBase > vertexTangentBases;
	uint32_t degenerated = 0;
	uint32_t invalid = 0;

	// Calculate tangent base for each polygon.
	polygonTangentBases.resize(polygons.size());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		uint32_t baseIndex;

		if (polygon.getVertexCount() < 3)
			continue;

		if (!findBaseIndex(model, polygon, baseIndex))
		{
			++degenerated;
			continue;
		}

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		const Vertex* v[] =
		{
			&model.getVertex(vertices[baseIndex]),
			&model.getVertex(vertices[(baseIndex + 1) % vertices.size()]),
			&model.getVertex(vertices[(baseIndex + 2) % vertices.size()])
		};

		Vector4 p[] =
		{
			model.getPosition(v[0]->getPosition()),
			model.getPosition(v[1]->getPosition()),
			model.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		T_ASSERT (ep[0].length() > FUZZY_EPSILON);
		T_ASSERT (ep[1].length() > FUZZY_EPSILON);

		ep[0] = ep[0].normalized();
		ep[1] = ep[1].normalized();

		TangentBase& tb = polygonTangentBases[i];
		tb.normal = cross(ep[0], ep[1]).normalized();

		if (abs(dot3(Vector4(0.0f, 1.0f, 0.0f, 0.0f), tb.normal)) < Scalar(1.0f - FUZZY_EPSILON))
			tb.tangent = cross(Vector4(0.0f, 1.0f, 0.0f, 0.0f), tb.normal).normalized();
		else
			tb.tangent = cross(Vector4(1.0f, 0.0f, 0.0f, 0.0f), tb.normal).normalized();

		tb.binormal = cross(tb.tangent, tb.normal).normalized();

		T_ASSERT (tb.normal.length() > FUZZY_EPSILON);
		tb.normal = tb.normal.normalized();

		if (
			v[0]->getTexCoord(0) != c_InvalidIndex &&
			v[1]->getTexCoord(0) != c_InvalidIndex &&
			v[2]->getTexCoord(0) != c_InvalidIndex
		)
		{
			Vector2 tc[] =
			{
				model.getTexCoord(v[0]->getTexCoord(0)),
				model.getTexCoord(v[1]->getTexCoord(0)),
				model.getTexCoord(v[2]->getTexCoord(0))
			};

			Vector2 etc[] = { tc[2] - tc[0], tc[1] - tc[0] };

			float denom = etc[0].x * etc[1].y - etc[1].x * etc[0].y;
			Scalar r = Scalar(denom != 0.0f ? 1.0f / denom : 0.0f);

			tb.tangent = ((Scalar(etc[0].y) * ep[1] - Scalar(etc[1].y) * ep[0]) * r).xyz0();
			tb.binormal = ((Scalar(etc[1].x) * ep[0] - Scalar(etc[0].x) * ep[1]) * r).xyz0();

			bool tangentValid = tb.tangent.length() > FUZZY_EPSILON;
			bool binormalValid = tb.binormal.length() > FUZZY_EPSILON;

			if (tangentValid || binormalValid)
			{
				if (!tangentValid)
					tb.tangent = cross(tb.binormal, tb.normal);
				if (!binormalValid)
					tb.binormal = cross(tb.tangent, tb.normal);

				tb.tangent = tb.tangent.normalized();
				tb.binormal = tb.binormal.normalized();

				Vector4 normal = cross(tb.tangent, tb.binormal);
				if (normal.length() >= FUZZY_EPSILON)
				{
					if (dot3(normal.normalized(), tb.normal) < 0.0f)
						tb.tangent = -tb.tangent;
				}
				else
					++invalid;
			}
			else
				++invalid;
		}
	}

	if (degenerated)
		log::warning << L"Degenerate " << degenerated << L" polygon(s) found in model" << Endl;

	if (invalid)
		log::warning << L"Invalid tangent space vectors; " << invalid << L" invalid UV base(s) of " << uint32_t(polygons.size()) << Endl;

	// Normalize polygon tangent bases.
	for (AlignedVector< TangentBase >::iterator i = polygonTangentBases.begin(); i != polygonTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Build new vertex normals.
	vertexTangentBases.resize(model.getVertexCount());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertices.begin(); j != vertices.end(); ++j)
		{
			if (polygonTangentBases[i].normal.length() > FUZZY_EPSILON)
				vertexTangentBases[*j].normal += polygonTangentBases[i].normal;
			if (polygonTangentBases[i].tangent.length() > FUZZY_EPSILON)
				vertexTangentBases[*j].tangent += polygonTangentBases[i].tangent;
			if (polygonTangentBases[i].binormal.length() > FUZZY_EPSILON)
				vertexTangentBases[*j].binormal += polygonTangentBases[i].binormal;
		}
	}

	// Normalize vertex tangent bases.
	for (AlignedVector< TangentBase >::iterator i = vertexTangentBases.begin(); i != vertexTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Update vertices.
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		const TangentBase& tb = vertexTangentBases[i];

		Vertex vertex = model.getVertex(i);
		if (vertex.getNormal() == c_InvalidIndex)
			vertex.setNormal(model.addUniqueNormal(tb.normal));
		if (vertex.getTangent() == c_InvalidIndex)
			vertex.setTangent(model.addUniqueNormal(tb.tangent));
		if (vertex.getBinormal() == c_InvalidIndex)
			vertex.setBinormal(model.addUniqueNormal(tb.binormal));

		model.setVertex(i, vertex);
	}

	return true;
}

	}
}
