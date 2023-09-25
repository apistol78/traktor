/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Range.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "World/WorldRenderView.h"
#include "World/Entity/LightComponent.h"
#include "World/Shared/Passes/LightClusterPass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightClusterPass", LightClusterPass, Object)

LightClusterPass::LightClusterPass(
    const WorldRenderSettings& settings
)
:   m_settings(settings)
{
}

bool LightClusterPass::create(render::IRenderSystem* renderSystem)
{
	// Tile light index array buffer.
	m_lightIndexSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster,
		sizeof(LightIndexShaderData),
		true
	);
	if (!m_lightIndexSBuffer)
		return false;

	// Tile cluster buffer.
	m_tileSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ,
		sizeof(TileShaderData),
		true
	);
	if (!m_tileSBuffer)
		return false;

	return true;
}

void LightClusterPass::destroy()
{
	safeDestroy(m_tileSBuffer);
	safeDestroy(m_lightIndexSBuffer);
}

void LightClusterPass::setup(
	const WorldRenderView& worldRenderView,
    const GatherView& gatheredView
) const
{
	T_PROFILER_SCOPE(L"LightClusterPass::setup");

	const auto& viewFrustum = worldRenderView.getViewFrustum();

	TileShaderData* tileShaderData = (TileShaderData*)m_tileSBuffer->lock();
	LightIndexShaderData* lightIndexShaderData = (LightIndexShaderData*)m_lightIndexSBuffer->lock();

	StaticVector< Vector4, c_maxLightCount > lightPositions;
	StaticVector< int32_t, c_maxLightCount > sliceLights;

	// Calculate positions of lights in view space.
	for (const auto& light : gatheredView.lights)
	{
		if (light)
		{
			const Vector4 lightPosition = light->getTransform().translation().xyz1();
			lightPositions.push_back(worldRenderView.getView() * lightPosition);
		}
		else
			lightPositions.push_back(Vector4::zero());
	}

	// Update tile data.
	const Scalar dx(1.0f / ClusterDimXY);
	const Scalar dy(1.0f / ClusterDimXY);
	const Scalar dz(1.0f / ClusterDimZ);

	const Vector4& tl = viewFrustum.corners[0];
	const Vector4& tr = viewFrustum.corners[1];
	const Vector4& bl = viewFrustum.corners[3];

	const Vector4 vx = tr - tl;
	const Vector4 vy = bl - tl;

	const Scalar vnz = viewFrustum.getNearZ();
	const Scalar vfz = viewFrustum.getFarZ();

	// Calculate XY tile frustums.
	Frustum tileFrustums[ClusterDimXY * ClusterDimXY];
	for (int32_t y = 0; y < ClusterDimXY; ++y)
	{
		const Scalar fy = Scalar((float)y) * dy;
		for (int32_t x = 0; x < ClusterDimXY; ++x)
		{
			const Scalar fx = Scalar((float)x) * dx;

			const Vector4 a = tl + vx * fx + vy * fy;
			const Vector4 b = tl + vx * (fx + dx) + vy * fy;
			const Vector4 c = tl + vx * (fx + dx) + vy * (fy + dy);
			const Vector4 d = tl + vx * fx + vy * (fy + dy);

			auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
			tileFrustum.planes[Frustum::Left] = Plane(Vector4::zero(), d, a);
			tileFrustum.planes[Frustum::Right] = Plane(Vector4::zero(), b, c);
			tileFrustum.planes[Frustum::Bottom] = Plane(Vector4::zero(), c, d);
			tileFrustum.planes[Frustum::Top] = Plane(Vector4::zero(), a, b);
		}
	}

	// Group lights per cluster.
	uint32_t lightOffset = 0;
	for (int32_t z = 0; z < ClusterDimZ; ++z)
	{
		const float fz = (float)z;
		const Scalar snz = vnz * power(vfz / vnz, Scalar(fz) / Scalar(ClusterDimZ));
		const Scalar sfz = vnz * power(vfz / vnz, Scalar(fz + 1.0f) / Scalar(ClusterDimZ));

		// Gather all lights intersecting slice.
		sliceLights.clear();
		for (uint32_t i = 0; i < gatheredView.lights.size(); ++i)
		{
			const auto light = gatheredView.lights[i];
			if (light == nullptr)
				continue;

			if (light->getLightType() == LightType::Directional)
			{
				sliceLights.push_back(i);
			}
			else if (light->getLightType() == LightType::Point)
			{
				const Scalar lr = light->getRange();
				const Scalar lz = lightPositions[i].z();
				if (lz + lr >= snz && lz - lr <= sfz)
					sliceLights.push_back(i);
			}
			else if (light->getLightType() == LightType::Spot)
			{
				const Scalar lr = light->getRange();
				const Scalar lz = lightPositions[i].z();
				if (lz + lr >= snz && lz - lr <= sfz)
				{
					Frustum spotFrustum;
					spotFrustum.buildPerspective(light->getRadius(), 1.0f, 0.0f, lr);

					Matrix44 lmt = light->getTransform().toMatrix44() * rotateX(deg2rad(90.0f));

					Vector4 fr[4] =
					{
						lmt * spotFrustum.corners[4],
						lmt * spotFrustum.corners[5],
						lmt * spotFrustum.corners[6],
						lmt * spotFrustum.corners[7]
					};

					Vector4 p[5];
					p[0] = lightPositions[i];
					p[1] = worldRenderView.getView() * fr[0].xyz1();
					p[2] = worldRenderView.getView() * fr[1].xyz1();
					p[3] = worldRenderView.getView() * fr[2].xyz1();
					p[4] = worldRenderView.getView() * fr[3].xyz1();

					Range< Scalar > bb;
					bb.min = lz;
					bb.max = lz;
					for (int i = 0; i < 5; ++i)
					{
						bb.min = min(bb.min, p[i].z());
						bb.max = max(bb.max, p[i].z());
					}
					if (Range< Scalar >::intersection(bb, Range< Scalar >(snz, sfz)).delta() >= 0.0_simd)
						sliceLights.push_back(i);
				}
			}
		}

		if (sliceLights.empty())
			continue;

		for (int32_t y = 0; y < ClusterDimXY; ++y)
		{
			for (int32_t x = 0; x < ClusterDimXY; ++x)
			{
				auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
				tileFrustum.planes[Frustum::Near] = Plane(Vector4(0.0f, 0.0f, 1.0f), snz);
				tileFrustum.planes[Frustum::Far] = Plane(Vector4(0.0f, 0.0f, -1.0f), -sfz);

				int32_t count = 0;
				for (uint32_t i = 0; i < sliceLights.size(); ++i)
				{
					const int32_t lightIndex = sliceLights[i];
					const auto light = gatheredView.lights[lightIndex];
					if (light->getLightType() == LightType::Directional)
					{
						lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
						++count;
					}
					else if (light->getLightType() == LightType::Point)
					{
						if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::Result::Outside)
						{
							lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
							++count;
						}
					}
					else if (light->getLightType() == LightType::Spot)
					{
						// \fixme Implement frustum to frustum culling.
						if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::Result::Outside)
						{
							lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
							++count;
						}
					}
					if (count >= MaxLightsPerCluster)
						break;
				}

				const uint32_t tileOffset = x + y * ClusterDimXY + z * ClusterDimXY * ClusterDimXY;
				tileShaderData[tileOffset].lightOffsetAndCount[0] = (int32_t)lightOffset;
				tileShaderData[tileOffset].lightOffsetAndCount[1] = count;

				lightOffset += count;
			}
		}
	}

	m_lightIndexSBuffer->unlock();
	m_tileSBuffer->unlock();
}

}
