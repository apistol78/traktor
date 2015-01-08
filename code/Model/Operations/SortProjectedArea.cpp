#include <cstring>
#include "Core/Math/Triangle.h"
#include "Model/Model.h"
#include "Model/Operations/SortProjectedArea.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct ProjectionRaster
{
	uint8_t* face;

	ProjectionRaster(uint8_t* face_)
	:	face(face_)
	{
	}

	void operator () (int x, int y, float alpha, float beta, float gamma)
	{
		if (x > 63 || x < 0 || y < 0 || y > 63)
			return;
		face[x + y * 64] = 1;
	}
};

struct SortCovers
{
	bool operator () (const std::pair< uint32_t, uint32_t >& c1, const std::pair< uint32_t, uint32_t >& c2)
	{
		return c1.second > c2.second;
	}
};

struct SortCoversInsideOut
{
	bool operator () (const std::pair< uint32_t, uint32_t >& c1, const std::pair< uint32_t, uint32_t >& c2)
	{
		return c1.second < c2.second;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.SortProjectedArea", SortProjectedArea, IModelOperation)

SortProjectedArea::SortProjectedArea(bool insideOut)
:	m_insideOut(insideOut)
{
}

bool SortProjectedArea::apply(Model& model) const
{
	uint8_t face[3][64 * 64];

	Aabb3 boundingBox = model.getBoundingBox();

	std::vector< std::pair< uint32_t, uint32_t > > covers;

	std::vector< Material > materials = model.getMaterials();
	std::vector< Polygon > polygons = model.getPolygons();

	for (uint32_t i = 0; i < materials.size(); ++i)
	{
		std::memset(face, 0, sizeof(face));

		for (std::vector< Polygon >::const_iterator j = polygons.begin(); j != polygons.end(); ++j)
		{
			if (j->getMaterial() != i)
				continue;

			T_ASSERT_M (j->getVertices().size() == 3, L"Model must be triangulated");

			Vector4 pv[] =
			{
				model.getPosition(model.getVertex(j->getVertex(0)).getPosition()),
				model.getPosition(model.getVertex(j->getVertex(1)).getPosition()),
				model.getPosition(model.getVertex(j->getVertex(2)).getPosition())
			};

			// x/y plane
			{
				Vector2 o(boundingBox.getExtent().x(), boundingBox.getExtent().y());
				Vector2 v[] =
				{
					64.0f * (Vector2(pv[0].x(), pv[0].y()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[1].x(), pv[1].y()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[2].x(), pv[2].y()) + o) / (2.0f * o)
				};
				ProjectionRaster raster(face[0]);
				triangle(v[0], v[1], v[2], raster);
				triangle(v[2], v[1], v[0], raster);
			}

			// x/z plane
			{
				Vector2 o(boundingBox.getExtent().x(), boundingBox.getExtent().z());
				Vector2 v[] =
				{
					64.0f * (Vector2(pv[0].x(), pv[0].z()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[1].x(), pv[1].z()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[2].x(), pv[2].z()) + o) / (2.0f * o)
				};
				ProjectionRaster raster(face[1]);
				triangle(v[0], v[1], v[2], raster);
				triangle(v[2], v[1], v[0], raster);
			}

			// y/z plane
			{
				Vector2 o(boundingBox.getExtent().y(), boundingBox.getExtent().z());
				Vector2 v[] =
				{
					64.0f * (Vector2(pv[0].y(), pv[0].z()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[1].y(), pv[1].z()) + o) / (2.0f * o),
					64.0f * (Vector2(pv[2].y(), pv[2].z()) + o) / (2.0f * o)
				};
				ProjectionRaster raster(face[2]);
				triangle(v[0], v[1], v[2], raster);
				triangle(v[2], v[1], v[0], raster);
			}
		}

		// Accumulate total cover of current material.
		uint32_t cover = 0;
		for (uint32_t j = 0; j < 3; ++j)
		{
			for (uint32_t k = 0; k < 64 * 64; ++k)
				cover += face[j][k];
		}
		covers.push_back(std::make_pair(i, cover));
	}

	// Sort materials by cover.
	if (!m_insideOut)
		std::sort(covers.begin(), covers.end(), SortCovers());
	else
		std::sort(covers.begin(), covers.end(), SortCoversInsideOut());

	for (uint32_t i = 0; i < covers.size(); ++i)
		materials[i] = model.getMaterial(covers[i].first);

	// Update polygons with new material IDs.
	for (std::vector< Polygon >::iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		for (uint32_t j = 0; j < covers.size(); ++j)
		{
			if (covers[j].first == i->getMaterial())
			{
				i->setMaterial(j);
				break;
			}
		}
	}

	// Update model with new data.
	model.setMaterials(materials);
	model.setPolygons(polygons);

	return true;
}

	}
}
