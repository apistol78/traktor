#include <algorithm>
#include "Mesh/Editor/ModelOptimizations.h"
#include "Model/Utilities.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

inline int _iround(float f)
{
	return int(f);
}

inline int _min(int a, int b, int c)
{
	return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

inline int _max(int a, int b, int c)
{
	return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

template < typename VisitorType >
void triangle(
	const Vector2* v,
	VisitorType* visitor,
	void (VisitorType::*visitMethod)(int x1, int x2, int y)
)
{
	// 28.4 fixed-point coordinates
	const int Y1 = _iround(16.0f * v[0].y);
	const int Y2 = _iround(16.0f * v[1].y);
	const int Y3 = _iround(16.0f * v[2].y);

	const int X1 = _iround(16.0f * v[0].x);
	const int X2 = _iround(16.0f * v[1].x);
	const int X3 = _iround(16.0f * v[2].x);

	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;

	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;

	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;

	// Bounding rectangle
	int minx = (_min(X1, X2, X3) + 0xF) >> 4;
	int maxx = (_max(X1, X2, X3) + 0xF) >> 4;
	int miny = (_min(Y1, Y2, Y3) + 0xF) >> 4;
	int maxy = (_max(Y1, Y2, Y3) + 0xF) >> 4;

	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

	int CY1 = C1 + DX12 * (miny << 4) - DY12 * (minx << 4);
	int CY2 = C2 + DX23 * (miny << 4) - DY23 * (minx << 4);
	int CY3 = C3 + DX31 * (miny << 4) - DY31 * (minx << 4);

	for(int y = miny; y < maxy; y++)
	{
		int CX1 = CY1;
		int CX2 = CY2;
		int CX3 = CY3;

		int x1 = minx;
		while (x1 < maxx)
		{
			if(CX1 > 0 && CX2 > 0 && CX3 > 0)
				break;
			CX1 -= FDY12;
			CX2 -= FDY23;
			CX3 -= FDY31;
			x1++;
		}
		if (x1 < maxx)
		{
			int x2 = x1 + 1;
			while (x2 < maxx)
			{
				if(!(CX1 > 0 && CX2 > 0 && CX3 > 0))
					break;
				CX1 -= FDY12;
				CX2 -= FDY23;
				CX3 -= FDY31;
				x2++;
			}
			(visitor->*visitMethod)(x1, x2, y);
		}

		CY1 += FDX12;
		CY2 += FDX23;
		CY3 += FDX31;
	}
}

struct ProjectionRaster
{
	uint8_t* face;

	ProjectionRaster(uint8_t* face_)
	:	face(face_)
	{
	}

	void raster(int x1, int x2, int y)
	{
		if (x1 > 63 || x2 < 0 || y < 0 || y > 63)
			return;
		if (x1 < 0)
			x1 = 0;
		if (x2 > 63)
			x2 = 63;
		for (int x = x1; x <= x2; ++x)
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

		}

void sortMaterialsByProjectedArea(model::Model& model)
{
	uint8_t face[3][64 * 64];

	Aabb boundingBox = model::calculateModelBoundingBox(model);

	std::vector< std::pair< uint32_t, uint32_t > > covers;

	std::vector< model::Material > materials = model.getMaterials();
	std::vector< model::Polygon > polygons = model.getPolygons();

	for (uint32_t i = 0; i < materials.size(); ++i)
	{
		memset(face, 0, sizeof(face));

		for (std::vector< model::Polygon >::const_iterator j = polygons.begin(); j != polygons.end(); ++j)
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
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
				std::swap(v[0], v[2]);
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
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
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
				std::swap(v[0], v[2]);
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
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
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
				std::swap(v[0], v[2]);
				triangle< ProjectionRaster >(v, &raster, &ProjectionRaster::raster);
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
	std::sort(covers.begin(), covers.end(), SortCovers());
	for (uint32_t i = 0; i < covers.size(); ++i)
		materials[i] = model.getMaterial(covers[i].first);

	// Update polygons with new material IDs.
	for (std::vector< model::Polygon >::iterator i = polygons.begin(); i != polygons.end(); ++i)
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
}

	}
}
