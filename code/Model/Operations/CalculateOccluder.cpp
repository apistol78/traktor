/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/VoxelGrid.h"
#include "Model/Operations/CalculateOccluder.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const float c_untouchedZ = std::numeric_limits< float >::max();

struct CoverageVisitor
{
	float* depthBuffer;
	float* clipBuffer;
	uint8_t* tagBuffer;
	float z[3];

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		if (x < 0 || y < 0 || x >= 64 || y >= 64)
			return;

		float fz = z[0] * alpha + z[1] * beta + z[2] * gamma;
		float cz = clipBuffer[x + y * 64];
		if (fz > cz + FUZZY_EPSILON)
		{
			float dz = depthBuffer[x + y * 64];
			if (fz < dz)
			{
				depthBuffer[x + y * 64] = fz;
				tagBuffer[x + y * 64] = 1;
			}
		}
	}
};

//struct LimitPredicate
//{
//	bool operator () (const std::pair< float, int32_t >& a, const std::pair< float, int32_t >& b) const
//	{
//		if (a.first < b.first)
//			return true;
//		else if (a.first > b.first)
//			return false;
//		else if (a.second < b.second)
//			return false;
//		else if (a.second > b.second)
//			return true;
//		else
//			return true;
//	}
//};

void addBoundingBox(Model& outModel, const Aabb3& boundingBox)
{
	Vector4 extents[8];
	boundingBox.getExtents(extents);

	const int* faces = Aabb3::getFaces();
	for (uint32_t i = 0; i < 6; ++i)
	{
		const int* face = &faces[i * 4];

		Polygon p;
		for (uint32_t j = 0; j < 4; ++j)
		{
			Vertex v;
			v.setPosition(outModel.addUniquePosition(extents[face[j]]));
			p.addVertex(outModel.addUniqueVertex(v));
		}
		p.flipWinding();

		outModel.addPolygon(p);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CalculateOccluder", CalculateOccluder, IModelOperation)

bool CalculateOccluder::apply(Model& model) const
{
	AlignedVector< Winding3 > windings;
	
	Aabb3 boundingBox = model.getBoundingBox();
	if (boundingBox.empty())
		return false;

	// Extract non-transparent windings.
	uint32_t polygonCount = model.getPolygonCount();
	windings.reserve(polygonCount);
	for (uint32_t i = 0; i < polygonCount; ++i)
	{
		const Polygon& polygon = model.getPolygon(i);

		uint32_t material = polygon.getMaterial();
		if (model.getMaterial(material).getBlendOperator() != Material::BoDecal)
			continue;

		Winding3 winding;

		uint32_t vertexCount = polygon.getVertexCount();
		for (uint32_t j = 0; j < vertexCount; ++j)
		{
			const Vertex& vertex = model.getVertex(polygon.getVertex(j));

			uint32_t position = vertex.getPosition();
			if (position == c_InvalidIndex)
				continue;

			winding.push(model.getPosition(position));
		}

		if (winding.size() < 3)
			continue;

		windings.push_back(winding);
	}


	Model strippedModel;
	Model voxelModel;


	Vector4 diag = boundingBox.getExtent();
	Vector4 diagL = diag * Scalar(4.0f);

	int32_t nvx = int32_t(diagL.x() + 0.5f);
	int32_t nvy = int32_t(diagL.y() + 0.5f);
	int32_t nvz = int32_t(diagL.z() + 0.5f);

	nvx = clamp(nvx, 8, 24);
	nvy = clamp(nvy, 8, 24);
	nvz = clamp(nvz, 8, 24);

	log::info << L"Using voxel grid size " << nvx << L" * " << nvy << L" * " << nvz << Endl;

	VoxelGrid gridShell(nvx, nvy, nvz);
	VoxelGrid gridVoid(nvx, nvy, nvz);

	Vector4 voxelSize(
		2.0f / float(nvx - 1),
		2.0f / float(nvy - 1),
		2.0f / float(nvz - 1)
	);

	Vector4 tunnelSize(
		diag.x() * voxelSize.x(),
		diag.y() * voxelSize.y(),
		diag.z() * Scalar(2.0f)
	);

	AlignedVector< Winding3 > tunnelWindings;
	AlignedVector< Winding3 > voxelWindings;


	for (int32_t iy = 0; iy < nvy; ++iy)
	{
		float fy = float(iy / float(nvy - 1)) * 2.0f - 1.0f;
		for (int32_t ix = 0; ix < nvx; ++ix)
		{
			float fx = float(ix / float(nvx - 1)) * 2.0f - 1.0f;

			Vector4 tunnelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, 0.0f);
			Aabb3 tunnelBounds(
				tunnelCenter - tunnelSize * Scalar(0.5f),
				tunnelCenter + tunnelSize * Scalar(0.5f)
			);

			Plane tunnelPlanes[6];
			for (uint32_t i = 0; i < 6; ++i)
			{
				const Vector4* normals = Aabb3::getNormals();
				tunnelPlanes[i] = Plane(
					normals[i],
					tunnelCenter + normals[i] * tunnelSize * Scalar(0.5f)
				);
			}

			tunnelWindings.resize(0);
			for (uint32_t i = 0; i < windings.size(); ++i)
			{
				Winding3 w = windings[i];
				T_ASSERT (!w.empty());

				for (uint32_t j = 0; j < sizeof_array(tunnelPlanes); ++j)
				{
					Winding3 wf, wb;
					w.split(tunnelPlanes[j], wf, wb);
					
					w = wb;

					if (w.empty())
						break;
				}

				if (w.size() >= 3)
					tunnelWindings.push_back(w);
			}

			for (int32_t iz = 0; iz < nvz; ++iz)
			{
				float fz = float(iz / float(nvz - 1)) * 2.0f - 1.0f;

				Vector4 voxelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, fz);
				Aabb3 voxelBounds(
					voxelCenter - diag * voxelSize * Scalar(0.5f),
					voxelCenter + diag * voxelSize * Scalar(0.5f)
				);

				Plane voxelPlanes[2];
				voxelPlanes[0] = Plane(
					Vector4(0.0f, 0.0f, 1.0f),
					voxelCenter + Vector4(0.0f, 0.0f, 1.0f) * diag * voxelSize * Scalar(0.5f)
				);
				voxelPlanes[1] = Plane(
					Vector4(0.0f, 0.0f, -1.0f),
					voxelCenter + Vector4(0.0f, 0.0f, -1.0f) * diag * voxelSize * Scalar(0.5f)
				);

				voxelWindings.resize(0);
				for (uint32_t i = 0; i < tunnelWindings.size(); ++i)
				{
					Winding3 w = tunnelWindings[i];
					T_ASSERT (!w.empty());

					for (uint32_t j = 0; j < sizeof_array(voxelPlanes); ++j)
					{
						Winding3 wf, wb;
						w.split(voxelPlanes[j], wf, wb);

						w = wb;

						if (w.empty())
							break;
					}

					if (w.size() >= 3)
						voxelWindings.push_back(w);
				}

	
				if (!voxelWindings.empty())
					gridShell.set(ix, iy, iz, 1);
				else
					gridShell.set(ix, iy, iz, 0);
			}
		}
	}



	for (int32_t ix = 0; ix < nvx; ++ix)
	{
		for (int32_t iy = 0; iy < nvy; ++iy)
		{
			int32_t iz0 = 0;
			for (; iz0 < nvz; ++iz0)
			{
				if (gridShell.get(ix, iy, iz0) != 0)
					break;
			}
			for (; iz0 < nvz; ++iz0)
			{
				if (gridShell.get(ix, iy, iz0) == 0)
					break;
			}
			if (iz0 >= nvz - 1)
				break;

			int32_t iz1 = iz0 + 1;
			for (; iz1 < nvz; ++iz1)
			{
				if (gridShell.get(ix, iy, iz1) != 0)
					break;
			}
			if (iz1 >= nvz)
				break;

			for (int32_t iz = iz0; iz < iz1; ++iz)
				gridShell.set(ix, iy, iz, 2);

			for (; iz1 < nvz; ++iz1)
			{
				if (gridShell.get(ix, iy, iz1) == 0)
					break;
			}

			iz0 = iz1;
		}
	}






	//for (int32_t ix = 0; ix < nvx; ++ix)
	//{
	//	for (int32_t iy = 0; iy < nvy; ++iy)
	//	{
	//		for (int32_t iz = 0; iz < nvz; ++iz)
	//		{
	//			uint32_t v = gridShell.get(ix, iy, iz);
	//			gridShell.set(ix, iy, iz, (v == 0) ? 1 : 0);
	//		}
	//	}
	//}


	for (int32_t ix = 0; ix < nvx; ++ix)
	{
		float fx = float(ix / float(nvx - 1)) * 2.0f - 1.0f;
		for (int32_t iy = 0; iy < nvy; ++iy)
		{
			float fy = float(iy / float(nvy - 1)) * 2.0f - 1.0f;
			for (int32_t iz = 0; iz < nvz; ++iz)
			{
				float fz = float(iz / float(nvz - 1)) * 2.0f - 1.0f;

				uint32_t v = gridShell.get(ix, iy, iz);
				//if (v == 0)
				//{
				//	Vector4 voxelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, fz);
				//	Aabb3 voxelBounds(
				//		voxelCenter - diag * voxelSize * Scalar(0.25f),
				//		voxelCenter + diag * voxelSize * Scalar(0.25f)
				//	);

				//	addBoundingBox(voxelModel, voxelBounds);
				//}
				if (v == 2)
				{
					Vector4 voxelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, fz);
					Aabb3 voxelBounds(
						voxelCenter - diag * voxelSize * Scalar(0.5f),
						voxelCenter + diag * voxelSize * Scalar(0.5f)
					);

					addBoundingBox(voxelModel, voxelBounds);
				}
			}
		}
	}




			//float depth0[64 * 64];
			//float depth1[64 * 64];
			//float clip[64 * 64];
			//uint8_t tag0[64 * 64];
			//uint8_t tag1[64 * 64];
			//Vector2 Txy[64];
			//float Tz[64];

			//CoverageVisitor visitor0, visitor1;

			//visitor0.depthBuffer = depth0;
			//visitor0.clipBuffer = clip;
			//visitor0.tagBuffer = tag0;

			//visitor1.depthBuffer = depth1;
			//visitor1.clipBuffer = clip;
			//visitor1.tagBuffer = tag1;

			//int32_t count = 0;
			//float startZ = 0.0f;

			//for (uint32_t i = 0; i < 64 * 64; ++i)
			//	clip[i] = -std::numeric_limits< float >::max();

			//for (;;)
			//{
			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//	{
			//		tag0[i] = 0;
			//		tag1[i] = 0;
			//	}

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//		depth0[i] = std::numeric_limits< float >::max();

			//	for (AlignedVector< Winding3 >::const_iterator i = tunnelWindings.begin(); i != tunnelWindings.end(); ++i)
			//	{
			//		for (uint32_t j = 0; j < i->points.size(); ++j)
			//		{
			//			Vector4 np = Scalar(0.5f) * (i->points[j] - tunnelCenter) / tunnelBounds.getExtent() + Scalar(0.5f);
			//			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			//			Tz[j] = i->points[j].z();
			//		}

			//		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			//		{
			//			visitor0.z[0] = Tz[0];
			//			visitor0.z[1] = Tz[k + 2];
			//			visitor0.z[2] = Tz[k + 1];
			//			triangle(Txy[0], Txy[k + 2], Txy[k + 1], visitor0);
			//		}
			//	}

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//		depth1[i] = depth0[i];

			//	for (AlignedVector< Winding3 >::const_iterator i = tunnelWindings.begin(); i != tunnelWindings.end(); ++i)
			//	{
			//		for (uint32_t j = 0; j < i->points.size(); ++j)
			//		{
			//			Vector4 np = Scalar(0.5f) * (i->points[j] - tunnelCenter) / tunnelBounds.getExtent() + Scalar(0.5f);
			//			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			//			Tz[j] = i->points[j].z();
			//		}

			//		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			//		{
			//			visitor1.z[0] = Tz[0];
			//			visitor1.z[1] = Tz[k + 1];
			//			visitor1.z[2] = Tz[k + 2];
			//			triangle(Txy[0], Txy[k + 1], Txy[k + 2], visitor1);
			//		}
			//	}

			//	float range0[2] = { std::numeric_limits< float >::max(), -std::numeric_limits< float >::max() };
			//	int32_t touched0 = 0;

			//	float range1[2] = { std::numeric_limits< float >::max(), -std::numeric_limits< float >::max() };
			//	int32_t touched1 = 0;

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//	{
			//		if (tag0[i])
			//		{
			//			range0[0] = min(range0[0], depth0[i]);
			//			range0[1] = max(range0[1], depth0[i]);
			//			clip[i] = max(clip[i], depth0[i]);
			//			++touched0;
			//		}

			//		if (tag1[i])
			//		{
			//			range1[0] = min(range1[0], depth1[i]);
			//			range1[1] = max(range1[1], depth1[i]);
			//			clip[i] = max(clip[i], depth1[i]);
			//			++touched1;
			//		}
			//	}

			//	const int32_t c_limit = (64 * 64 * 99) / 100;

			//	bool t0 = touched0 >= c_limit;
			//	bool t1 = touched1 >= c_limit;

			//	if (t0 && !t1)	// Enter solid
			//	{
			//		if (++count == 1)
			//		{
			//			startZ = range0[1];
			//		}
			//	}
			//	else if (!t0 && t1)	// Leave solid
			//	{
			//		if (--count == 0)
			//		{
			//			Aabb3 solidBounds(
			//				Vector4(
			//					tunnelBounds.mn.x(),
			//					tunnelBounds.mn.y(),
			//					startZ
			//				),
			//				Vector4(
			//					tunnelBounds.mx.x(),
			//					tunnelBounds.mx.y(),
			//					range1[0]
			//				)
			//			);

			//			addBoundingBox(voxelModel, solidBounds);
			//		}
			//	}
			//	else if (!t0 && !t1)
			//		break;
			//}





			//	for (std::set< int32_t >::const_reverse_iterator i = ids0.rbegin(); i != ids0.rend(); ++i)
			//		tunnelWindings.erase(tunnelWindings.begin() + *i);



			//	// Clear coverage buffer.
			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//		coverage[i] = std::make_pair(
			//			std::numeric_limits< float >::max(),
			//			-1
			//		);

			//	// Rasterize windings to coverage buffer.
			//	for (AlignedVector< Winding3 >::const_iterator i = tunnelWindings.begin(); i != tunnelWindings.end(); ++i)
			//	{
			//		for (uint32_t j = 0; j < i->points.size(); ++j)
			//		{
			//			Vector4 np = Scalar(0.5f) * (i->points[j] - tunnelCenter) / tunnelBounds.getExtent() + Scalar(0.5f);
			//			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			//			Tz[j] = i->points[j].z();
			//		}

			//		visitor.id = std::distance(tunnelWindings.begin(), i);

			//		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			//		{
			//			visitor.z[0] = Tz[0];
			//			visitor.z[1] = Tz[k + 1];
			//			visitor.z[2] = Tz[k + 2];
			//			triangle(Txy[0], Txy[k + 1], Txy[k + 2], visitor);
			//		}
			//	}

			//	float range1[2] = { c_untouchedZ, -c_untouchedZ };
			//	int32_t touched1 = 0;
			//	std::set< int32_t > ids1;

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//	{
			//		if (coverage[i].second >= 0)
			//		{
			//			range1[0] = min(range1[0], coverage[i].first);
			//			range1[1] = max(range1[1], coverage[i].first);
			//			++touched1;
			//			ids1.insert(coverage[i].second);
			//		}
			//	}


			//	for (std::set< int32_t >::const_reverse_iterator i = ids0.rbegin(); i != ids0.rend(); ++i)
			//		tunnelWindings.erase(tunnelWindings.begin() + *i);




			//}



			//std::pair< float, int32_t > coverage[64 * 64];
			//Vector2 Txy[64];
			//float Tz[64];

			//std::vector< Range< float > > ranges[2];

			//CoverageVisitor visitor;
			//visitor.coverage = coverage;
			//visitor.clip = -std::numeric_limits< float >::max();

			//int32_t dummy = 0;

			//// Positive ranges.
			//for (;;)
			//{
			//	// Clear coverage buffer.
			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//		coverage[i] = std::numeric_limits< float >::max();

			//	// Rasterize windings to coverage buffer.
			//	for (AlignedVector< Winding3 >::const_iterator i = voxelWindings.begin(); i != voxelWindings.end(); ++i)
			//	{
			//		for (uint32_t j = 0; j < i->points.size(); ++j)
			//		{
			//			Vector4 np = Scalar(0.5f) * (i->points[j] - voxelCenter) / voxelBounds.getExtent() + Scalar(0.5f);
			//			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			//			Tz[j] = i->points[j].z();
			//		}

			//		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			//		{
			//			visitor.z[0] = Tz[0];
			//			visitor.z[1] = Tz[k + 2];
			//			visitor.z[2] = Tz[k + 1];
			//			triangle(Txy[0], Txy[k + 2], Txy[k + 1], visitor);
			//		}
			//	}

			//	// Check for solid threshold.
			//	int32_t touched0 = 0;
			//	float range0[2] = { c_untouchedZ, -c_untouchedZ };

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//	{
			//		if (coverage[i] < c_untouchedZ - FUZZY_EPSILON)
			//		{
			//			range0[0] = min(range0[0], coverage[i]);
			//			range0[1] = max(range0[1], coverage[i]);
			//			++touched0;
			//		}
			//	}


			//	// Clear coverage buffer.
			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//		coverage[i] = std::numeric_limits< float >::max();

			//	// Rasterize windings to coverage buffer.
			//	for (AlignedVector< Winding3 >::const_iterator i = voxelWindings.begin(); i != voxelWindings.end(); ++i)
			//	{
			//		for (uint32_t j = 0; j < i->points.size(); ++j)
			//		{
			//			Vector4 np = Scalar(0.5f) * (i->points[j] - voxelCenter) / voxelBounds.getExtent() + Scalar(0.5f);
			//			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			//			Tz[j] = i->points[j].z();
			//		}

			//		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			//		{
			//			visitor.z[0] = Tz[0];
			//			visitor.z[1] = Tz[k + 1];
			//			visitor.z[2] = Tz[k + 2];
			//			triangle(Txy[0], Txy[k + 1], Txy[k + 2], visitor);
			//		}
			//	}

			//	// Check for solid threshold.
			//	int32_t touched1 = 0;
			//	float range1[2] = { c_untouchedZ, -c_untouchedZ };

			//	for (uint32_t i = 0; i < 64 * 64; ++i)
			//	{
			//		if (coverage[i] < c_untouchedZ - FUZZY_EPSILON)
			//		{
			//			range1[0] = min(range1[0], coverage[i]);
			//			range1[1] = max(range1[1], coverage[i]);
			//			++touched1;
			//		}
			//	}

			//	const int32_t c_limit = (64 * 64 * 99) / 100;

			//	if (touched0 >= c_limit)
			//	{
			//		if (touched1 < c_limit)
			//			break;

			//		if (range0[1] < range1[0])
			//			ranges[0].push_back(Range< float >(range0[1], range1[0]));
			//		else
			//		{
			//			if (!ranges[0].empty())
			//				ranges[0].back().max = range1[0];
			//		}
			//	}
			//	else
			//	{
			//		if (touched1 < c_limit)
			//			break;

			//		if (!ranges[0].empty())
			//			ranges[0].back().max = range1[0];
			//		//else
			//		//	break;
			//	}

			//	// ranges[0].push_back(Range< float >(range0[1], range1[0]));

			//	visitor.clip = range1[0] + FUZZY_EPSILON;

			//	//{
			//	//	drawing::Image coverageP(drawing::PixelFormat::getR8G8B8(), 64, 64);
			//	//	for (int32_t y = 0; y < 64; ++y)
			//	//	{
			//	//		for (int32_t x = 0; x < 64; ++x)
			//	//		{
			//	//			float z = coverage[x + y * 64];
			//	//			float nz = 0.5f * (z - voxelCenter.z()) / voxelBounds.getExtent().z() + 0.5f;

			//	//			coverageP.setPixel(
			//	//				x,
			//	//				y,
			//	//				Color4f(
			//	//					nz,
			//	//					(nz < 0.0f) ? 1.0f : 0.0f,
			//	//					(nz > 1.0f) ? 1.0f : 0.0f,
			//	//					1.0f
			//	//				)
			//	//			);
			//	//			
			//	//		}
			//	//	}
			//	//	coverageP.save(L"data/temp/coverage_" + toString(ix) + L"_" + toString(iy) + L"_" + toString(dummy) + L".png");
			//	//	++dummy;
			//	//}
			//}

			////// Negative ranges.
			////visitor.clip = -std::numeric_limits< float >::max();
			////for (;;)
			////{
			////	// Clear coverage buffer.
			////	for (uint32_t i = 0; i < 64 * 64; ++i)
			////		coverage[i] = std::numeric_limits< float >::max();

			////	// Rasterize windings to coverage buffer.
			////	for (AlignedVector< Winding3 >::const_iterator i = voxelWindings.begin(); i != voxelWindings.end(); ++i)
			////	{
			////		for (uint32_t j = 0; j < i->points.size(); ++j)
			////		{
			////			Vector4 np = Scalar(0.5f) * (i->points[j] - voxelCenter) / voxelBounds.getExtent() + Scalar(0.5f);
			////			Txy[j] = Vector2(64.0f * np.x(), 64.0f * np.y());
			////			Tz[j] = i->points[j].z();
			////		}

			////		for (uint32_t k = 0; k < i->points.size() - 2; ++k)
			////		{
			////			visitor.z[0] = Tz[0];
			////			visitor.z[1] = Tz[k + 2];
			////			visitor.z[2] = Tz[k + 1];
			////			triangle(Txy[0], Txy[k + 2], Txy[k + 1], visitor);
			////		}
			////	}

			////	// Check for solid threshold.
			////	int32_t touched = 0;
			////	float range[2] = { c_untouchedZ, -c_untouchedZ };

			////	for (uint32_t i = 0; i < 64 * 64; ++i)
			////	{
			////		if (coverage[i] < c_untouchedZ - FUZZY_EPSILON)
			////		{
			////			range[0] = min(range[0], coverage[i]);
			////			range[1] = max(range[1], coverage[i]);
			////			++touched;
			////		}
			////	}

			////	const int32_t c_limit = (64 * 64 * 99) / 100;
			////	if (touched < c_limit)
			////		break;

			////	ranges[1].push_back(Range< float >(range[0], range[1]));
			////	visitor.clip = range[1] + FUZZY_EPSILON;
			////}


			//for (std::vector< Range< float > >::const_iterator i = ranges[0].begin(); i != ranges[0].end(); ++i)
			//{
			//	Aabb3 solidBounds(
			//		Vector4(
			//			voxelBounds.mn.x(),
			//			voxelBounds.mn.y(),
			//			i->min
			//		),
			//		Vector4(
			//			voxelBounds.mx.x(),
			//			voxelBounds.mx.y(),
			//			i->max 
			//		)
			//	);

			//	addBoundingBox(voxelModel, solidBounds);
			//}

			////for (std::vector< Range< float > >::const_iterator i = ranges[1].begin(); i != ranges[1].end(); ++i)
			////{
			////	Aabb3 solidBounds(
			////		Vector4(
			////			voxelBounds.mn.x(),
			////			voxelBounds.mn.y(),
			////			i->min
			////		),
			////		Vector4(
			////			voxelBounds.mx.x(),
			////			voxelBounds.mx.y(),
			////			i->max 
			////		)
			////	);

			////	addBoundingBox(voxelModel, solidBounds);
			////}


			////std::vector< std::pair< float, int32_t > > limits;

			////for (std::vector< Range< float > >::const_iterator i = ranges[0].begin(); i != ranges[0].end(); ++i)
			////	limits.push_back(std::make_pair(i->max, 1));
			////for (std::vector< Range< float > >::const_iterator i = ranges[1].begin(); i != ranges[1].end(); ++i)
			////	limits.push_back(std::make_pair(i->min, -1));

			////std::sort(limits.begin(), limits.end(), LimitPredicate());


			////if (limits.empty())
			////	continue;


			////log::info << L"Limits" << Endl;
			////for (std::vector< std::pair< float, int32_t > >::const_iterator i = limits.begin(); i != limits.end(); ++i)
			////{
			////	log::info << i->first << L" " << i->second << Endl;
			////}


			////int32_t count = 0;
			////Range< float > solid;

			////for (std::vector< std::pair< float, int32_t > >::const_iterator i = limits.begin(); i != limits.end(); ++i)
			////{
			////	if (i->second == 1 && count == 0)
			////	{
			////		// Into solid.
			////		solid.min = i->first;
			////	}
			////	else if (i->second == -1 && count == 1)
			////	{
			////		// Into void.
			////		solid.max = i->first;

			////		Aabb3 solidBounds(
			////			Vector4(
			////				voxelBounds.mn.x(),
			////				voxelBounds.mn.y(),
			////				solid.min
			////			),
			////			Vector4(
			////				voxelBounds.mx.x(),
			////				voxelBounds.mx.y(),
			////				solid.max 
			////			)
			////		);

			////		//addBoundingBox(voxelModel, solidBounds);
			////	}
			////	count += i->second;
			////}

			////log::info << L"Leave count " << count << Endl;



			////for (std::vector< float >::const_iterator i = positions.begin(); i != positions.end(); ++i)
			////{
			////	Vector4 center = voxelBounds.getCenter();
			////	Aabb3 solidBounds(
			////		Vector4(
			////			center.x() - Scalar(0.2f),
			////			center.y() - Scalar(0.2f),
			////			*i - Scalar(0.2f)
			////		),
			////		Vector4(
			////			center.x() + Scalar(0.2f),
			////			center.y() + Scalar(0.2f),
			////			*i + Scalar(0.2f)
			////		)
			////	);
			////	addBoundingBox(voxelModel, solidBounds);
			////}

			////if (positions.size() < 2)
			////	continue;

			////for (size_t i = 0; i < positions.size() - 1; i += 2)
			////{
			////	float nz = positions[i];
			////	float fz = positions[i + 1];

			////	if (fz > nz)
			////	{
			////		Aabb3 solidBounds(
			////			Vector4(
			////				voxelBounds.mn.x(),
			////				voxelBounds.mn.y(),
			////				nz
			////			),
			////			Vector4(
			////				voxelBounds.mx.x(),
			////				voxelBounds.mx.y(),
			////				fz
			////			)
			////		);

			////		addBoundingBox(voxelModel, solidBounds);
			////	}
			////}
	//	}
	//}


	model::ModelFormat::writeAny(L"data/Temp/Voxel.obj", &voxelModel);

/*

	Model strippedModel;
	Model shellModel;
	Model occlusionModel;
	Model clippedModel;

	// Trace voxels which are completely inside solid space.
	const Vector4* normals = Aabb3::getNormals();

	Vector4 diag = boundingBox.getExtent();
	Vector4 diagL = diag * Scalar(0.5f);

	int32_t nvx = int32_t(diagL.x() + 0.5f);
	int32_t nvy = int32_t(diagL.y() + 0.5f);
	int32_t nvz = int32_t(diagL.z() + 0.5f);

	nvx = clamp(nvx, 8, 24);
	nvy = clamp(nvy, 8, 24);
	nvz = clamp(nvz, 8, 24);

	log::info << L"Using voxel grid size " << nvx << L" * " << nvy << L" * " << nvz << Endl;

	VoxelGrid gridShell(nvx, nvy, nvz);
	VoxelGrid gridSolid(nvx, nvy, nvz);

	Vector4 voxelSize(
		2.0f / float(nvx - 1),
		2.0f / float(nvy - 1),
		2.0f / float(nvz - 1)
	);

	const Vector4 c_directions[] =
	{
		Vector4( 1.0f,  0.0f,  0.0f),	// 1
		Vector4(-1.0f,  0.0f,  0.0f),	// 2
		Vector4( 0.0f,  1.0f,  0.0f),	// 4
		Vector4( 0.0f, -1.0f,  0.0f),	// 8
		Vector4( 0.0f,  0.0f,  1.0f),	// 16
		Vector4( 0.0f,  0.0f, -1.0f)	// 32
	};

	AlignedVector< Winding3 > voxelWindings;

	drawing::Image coverage[] =
	{
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64),
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64),
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64),
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64),
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64),
		drawing::Image(drawing::PixelFormat::getR8(), 64, 64)
	};

	for (int32_t iz = 0; iz < nvz; ++iz)
	{
		log::info << ((iz * 100) / (nvz - 1)) << L"%..." << Endl;

		float fz = float(iz / float(nvz - 1)) * 2.0f - 1.0f;
		for (int32_t iy = 0; iy < nvy; ++iy)
		{
			float fy = float(iy / float(nvy - 1)) * 2.0f - 1.0f;
			for (int32_t ix = 0; ix < nvx; ++ix)
			{
				float fx = float(ix / float(nvx - 1)) * 2.0f - 1.0f;

				Vector4 voxelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, fz);
				Aabb3 voxelBounds(
					voxelCenter - diag * voxelSize * Scalar(0.5f),
					voxelCenter + diag * voxelSize * Scalar(0.5f)
				);
				Vector4 voxelExtents[8];
				voxelBounds.getExtents(voxelExtents);

				Plane voxelPlanes[6];
				for (uint32_t i = 0; i < 6; ++i)
				{
					voxelPlanes[i] = Plane(
						normals[i],
						voxelCenter + normals[i] * diag * voxelSize * Scalar(0.5f - FUZZY_EPSILON)
					);
				}

				voxelWindings.resize(0);

				for (uint32_t i = 0; i < windings.size(); ++i)
				{
					Winding3 w = windings[i];
					T_ASSERT (!w.points.empty());

					for (uint32_t j = 0; j < sizeof_array(voxelPlanes); ++j)
					{
						Winding3 wf, wb;
						w.split(voxelPlanes[j], wf, wb);
						
						w = wb;

						if (w.points.empty())
							break;
					}

					if (w.points.size() >= 3)
						voxelWindings.push_back(w);
				}

				if (!voxelWindings.empty())
				{
					// Clear coverage buffers.
					for (uint32_t i = 0; i < 6; ++i)
						coverage[i].clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

					for (AlignedVector< Winding3 >::const_iterator i = voxelWindings.begin(); i != voxelWindings.end(); ++i)
					{
						Vector2 T[64];
						for (int32_t j = 0; j < 6; ++j)
						{
							int32_t axis = j / 2;
							int32_t sign = j & 1;

							for (uint32_t k = 0; k < i->points.size(); ++k)
							{
								Vector4 np = Scalar(0.5f) * (i->points[k] - voxelCenter) / voxelBounds.getExtent() + Scalar(0.5f);
								switch (axis)
								{
								case 0:
									T[k] = Vector2(64.0f * np.y(), 64.0f * np.z());
									break;

								case 1:
									T[k] = Vector2(64.0f * np.z(), 64.0f * np.x());
									break;

								case 2:
									T[k] = Vector2(64.0f * np.y(), 64.0f - 64.0f * np.x());
									break;
								}
							}

							CoverageVisitor visitor(coverage[j]);

							if (!sign)
							{
								for (uint32_t k = 0; k < i->points.size() - 2; ++k)
									triangle(T[0], T[k + 1], T[k + 2], visitor);
							}
							else
							{
								for (uint32_t k = 0; k < i->points.size() - 2; ++k)
									triangle(T[0], T[k + 2], T[k + 1], visitor);
							}
						}
					}

					for (uint32_t i = 0; i < 6; ++i)
					{
						//coverage[i].save(L"data/temp/Coverage_" + toString(ix) + L"_" + toString(iy) + L"_" + toString(iz) + L"_" + toString(i) + L".png");

						uint8_t* data = (uint8_t*)coverage[i].getData();
						int32_t total = 0;

						for (int32_t j = 0; j < 64 * 64; ++j)
						{
							if (data[j] != 0)
							{
								++total;
								if ((total * 100) / (64 * 64) >= 50)
									break;
							}
						}

						if ((total * 100) / (64 * 64) >= 50)
						{
							uint32_t mask = 1 << i;
							gridShell.set(ix, iy, iz, mask | gridShell.get(ix, iy, iz));
						}
					}
				}
			}
		}
	}

	log::info << L"Scanning X/Y..." << Endl;

	for (int32_t iy = 0; iy < nvy; ++iy)
	{
		for (int32_t ix = 0; ix < nvx; ++ix)
		{
			int32_t iz0 = 0;
			for (; iz0 < nvz; ++iz0)
			{
				// Scan until voxel facing toward.
				for (; iz0 < nvz; ++iz0)
				{
					if (gridShell.get(ix, iy, iz0) & 32)
						break;
				}
				if (iz0 >= nvz)
					continue;

				// Scan further until first non-empty voxel.
				int32_t iz1 = iz0 + 1;
				for (; iz1 < nvz; ++iz1)
				{
					if (gridShell.get(ix, iy, iz1))
						break;
				}
				if (iz1 >= nvz)
					continue;

				if (!(gridShell.get(ix, iy, iz1) & 16))
					continue;

				// Found solid span; fill entire span in output grid.
				for (int32_t iz = iz0 + 1; iz <= iz1 - 1; ++iz)
					gridSolid.set(ix, iy, iz, 1);

				iz0 = iz1;
			}
		}
	}

	log::info << L"Scanning X/Z..." << Endl;

	for (int32_t iz = 0; iz < nvz; ++iz)
	{
		for (int32_t ix = 0; ix < nvx; ++ix)
		{
			int32_t iy0 = 0;
			for (; iy0 < nvy; ++iy0)
			{
				// Scan until voxel facing toward.
				for (; iy0 < nvy; ++iy0)
				{
					if (gridShell.get(ix, iy0, iz) & 8)
						break;
				}
				if (iy0 >= nvy)
					continue;

				// Scan further until first non-empty voxel.
				int32_t iy1 = iy0 + 1;
				for (; iy1 < nvy; ++iy1)
				{
					if (gridShell.get(ix, iy1, iz))
						break;
				}
				if (iy1 >= nvy)
					continue;

				if (!(gridShell.get(ix, iy1, iz) & 4))
					continue;

				// Found solid span; fill entire span in output grid.
				for (int32_t iy = iy0 + 1; iy <= iy1 - 1; ++iy)
					gridSolid.set(ix, iy, iz, 1);

				iy0 = iy1;
			}
		}
	}

	log::info << L"Scanning Y/Z..." << Endl;

	for (int32_t iz = 0; iz < nvz; ++iz)
	{
		for (int32_t iy = 0; iy < nvy; ++iy)
		{
			int32_t ix0 = 0;
			for (; ix0 < nvx; ++ix0)
			{
				// Scan until voxel facing toward.
				for (; ix0 < nvx; ++ix0)
				{
					if (gridShell.get(ix0, iy, iz) & 2)
						break;
				}
				if (ix0 >= nvx)
					continue;

				// Scan further until first non-empty voxel.
				int32_t ix1 = ix0 + 1;
				for (; ix1 < nvx; ++ix1)
				{
					if (gridShell.get(ix1, iy, iz))
						break;
				}
				if (ix1 >= nvx)
					continue;

				if (!(gridShell.get(ix1, iy, iz) & 1))
					continue;

				// Found solid span; fill entire span in output grid.
				for (int32_t ix = ix0 + 1; ix <= ix1 - 1; ++ix)
					gridSolid.set(ix, iy, iz, 1);

				ix0 = ix1;
			}
		}
	}

	log::info << L"Generating model..." << Endl;

	for (int32_t iz = 0; iz < nvz; ++iz)
	{
		float fz = float(iz / float(nvz - 1)) * 2.0f - 1.0f;
		for (int32_t iy = 0; iy < nvy; ++iy)
		{
			float fy = float(iy / float(nvy - 1)) * 2.0f - 1.0f;
			for (int32_t ix = 0; ix < nvx; ++ix)
			{
				float fx = float(ix / float(nvx - 1)) * 2.0f - 1.0f;

				Vector4 voxelCenter = boundingBox.getCenter() + diag * Vector4(fx, fy, fz);

				Aabb3 voxelBounds(
					voxelCenter - diag * voxelSize * Scalar(0.5f),
					voxelCenter + diag * voxelSize * Scalar(0.5f)
				);

				Vector4 voxelExtents[8];
				voxelBounds.getExtents(voxelExtents);

				if (gridSolid.get(ix, iy, iz))
				{
					const int32_t c_offset[6][3] =
					{
						{  0,  0, -1 },
						{  1,  0,  0 },
						{  0,  0,  1 },
						{ -1,  0,  0 },
						{  0,  1,  0 },
						{  0, -1,  0 }
					};


					const int* faces = Aabb3::getFaces();
					for (uint32_t i = 0; i < 6; ++i)
					{
						if (gridSolid.get(ix + c_offset[i][0], iy + c_offset[i][1], iz + c_offset[i][2]))
							continue;

						const int* face = &faces[i * 4];

						Polygon p;
						for (uint32_t j = 0; j < 4; ++j)
						{
							Vertex v;
							v.setPosition(occlusionModel.addUniquePosition(voxelExtents[face[j]]));
							p.addVertex(occlusionModel.addUniqueVertex(v));
						}
						p.flipWinding();

						occlusionModel.addPolygon(p);
					}
				}

				if (gridShell.get(ix, iy, iz))
				{
					for (uint32_t ii = 0; ii < sizeof_array(c_directions); ++ii)
					{
						if (gridShell.get(ix, iy, iz) & (1 << ii))
						{
							shellModel.addPolygon(Polygon(
								c_InvalidIndex,
								shellModel.addUniqueVertex(Vertex(shellModel.addUniquePosition(voxelCenter))),
								shellModel.addUniqueVertex(Vertex(shellModel.addUniquePosition(voxelCenter + c_directions[ii] * voxelSize.length() * Scalar(0.15f))))
							));
						}
					}

					const int* faces = Aabb3::getFaces();
					for (uint32_t i = 0; i < 6; ++i)
					{
						const int* face = &faces[i * 4];

						Polygon p;
						for (uint32_t j = 0; j < 4; ++j)
						{
							Vertex v;
							v.setPosition(shellModel.addUniquePosition(voxelExtents[face[j]]));
							p.addVertex(shellModel.addUniqueVertex(v));
						}
						p.flipWinding();
						shellModel.addPolygon(p);
					}
				}
			}
		}
	}
*/

	for (AlignedVector< Winding3 >::const_iterator i = windings.begin(); i != windings.end(); ++i)
	{
		Polygon p;
		for (Winding3::points_t::const_iterator j = i->getPoints().begin(); j != i->getPoints().end(); ++j)
		{
			Vertex v;
			v.setPosition(strippedModel.addUniquePosition(*j));
			p.addVertex(strippedModel.addUniqueVertex(v));
		}
		strippedModel.addPolygon(p);
	}

	model::ModelFormat::writeAny(L"data/Temp/Stripped.obj", &strippedModel);

/*
	model::ModelFormat::writeAny(L"data/Temp/Occlusion.obj", &occlusionModel);
	model::ModelFormat::writeAny(L"data/Temp/Shell.obj", &shellModel);
	//model::ModelFormat::writeAny(L"data/Temp/Clipped.obj", &clippedModel);

	model = occlusionModel;
	triangulateModel(model);
*/

	return true;
}

	}
}
