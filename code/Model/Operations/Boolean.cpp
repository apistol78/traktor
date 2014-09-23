#include <vector>
#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/MergeModel.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct GatherFront
{
	AlignedVector< Winding3 > windings;
	int32_t mask;

	void operator () (uint32_t index, const Winding3& w, int32_t cl, bool splitted)
	{
		if (w.size() < 3)
			return;

		mask |= (1 << cl);
		if (cl == Winding3::CfFront)
			windings.push_back(w);
	}
};

void buildWindings(const Model& model, const Transform& transform, AlignedVector< Winding3 >& outWindings)
{
	const std::vector< Polygon >& polygons = model.getPolygons();
	outWindings.resize(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		const Polygon& polygon = polygons[i];
		
		outWindings[i] = Winding3();
		for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			outWindings[i].push(transform * model.getVertexPosition(polygon.getVertex(j)).xyz1());
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Boolean", Boolean, IModelOperation)

Boolean::Boolean(
	const Model& modelA, const Transform& modelTransformA,
	const Model& modelB, const Transform& modelTransformB
)
:	m_modelA(modelA)
,	m_modelB(modelB)
,	m_modelTransformA(modelTransformA)
,	m_modelTransformB(modelTransformB)
{
}

bool Boolean::apply(Model& model) const
{
	if (m_modelA.getPolygonCount() == 0)
	{
		MergeModel(m_modelB, m_modelTransformB, 0.01f).apply(model);
		return true;
	}

	if (m_modelB.getPolygonCount() == 0)
	{
		MergeModel(m_modelA, m_modelTransformA, 0.01f).apply(model);
		return true;
	}

	AlignedVector< Winding3 > windingsA;
	AlignedVector< Winding3 > windingsB;

	buildWindings(m_modelA, m_modelTransformA, windingsA);
	buildWindings(m_modelB, m_modelTransformB, windingsB);

	BspTree treeA;
	BspTree treeB;

	if (!treeA.build(windingsA) || !treeB.build(windingsB))
		return false;

	model.clear(Model::CfAll);

	// Clip all A to tree B.
	{
		GatherFront visitor;
		for (uint32_t i = 0; i < windingsA.size(); ++i)
		{
			visitor.mask = 0;
			visitor.windings.resize(0);

			treeB.clip(windingsA[i], visitor);

			if (visitor.mask == ((1 << Winding3::CfFront) | (1 << Winding3::CfBack)))
			{
				for (uint32_t j = 0; j < visitor.windings.size(); ++j)
				{
					const Winding3& w = visitor.windings[j];

					Polygon polygon;
					for (uint32_t k = 0; k < w.size(); ++k)
					{
						Vertex vertex;
						vertex.setPosition(model.addUniquePosition(w[k]));
						polygon.addVertex(model.addUniqueVertex(vertex));
					}
					model.addPolygon(polygon);
				}
			}
			else if (visitor.mask == (1 << Winding3::CfFront))
			{
				const Winding3& w = windingsA[i];

				Polygon polygon;
				for (uint32_t k = 0; k < w.size(); ++k)
				{
					Vertex vertex;
					vertex.setPosition(model.addUniquePosition(w[k]));
					polygon.addVertex(model.addUniqueVertex(vertex));
				}
				model.addPolygon(polygon);
			}
		}
	}

	// Clip all B to tree A.
	{
		GatherFront visitor;
		for (uint32_t i = 0; i < windingsB.size(); ++i)
		{
			visitor.mask = 0;
			visitor.windings.resize(0);

			treeA.clip(windingsB[i], visitor);

			if (visitor.mask == ((1 << Winding3::CfFront) | (1 << Winding3::CfBack)))
			{
				for (uint32_t j = 0; j < visitor.windings.size(); ++j)
				{
					const Winding3& w = visitor.windings[j];

					Polygon polygon;
					for (uint32_t k = 0; k < w.size(); ++k)
					{
						Vertex vertex;
						vertex.setPosition(model.addUniquePosition(w[k]));
						polygon.addVertex(model.addUniqueVertex(vertex));
					}
					model.addPolygon(polygon);
				}
			}
			else if (visitor.mask == (1 << Winding3::CfFront))
			{
				const Winding3& w = windingsB[i];

				Polygon polygon;
				for (uint32_t k = 0; k < w.size(); ++k)
				{
					Vertex vertex;
					vertex.setPosition(model.addUniquePosition(w[k]));
					polygon.addVertex(model.addUniqueVertex(vertex));
				}
				model.addPolygon(polygon);
			}
		}
	}

	return true;
}

	}
}
