/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Float.h"
#include "Core/Math/Random.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/ProgramParameters.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::weather
{
	namespace
	{

const render::Handle c_handleFrustumEdges(L"Precipitation_FrustumEdges");
const render::Handle c_handleParallaxDistance(L"Precipitation_ParallaxDistance");
const render::Handle c_handleDepthDistance(L"Precipitation_DepthDistance");
const render::Handle c_handleOpacity(L"Precipitation_Opacity");
const render::Handle c_handleLayerAngle(L"Precipitation_LayerAngle");

class PrecipitationMeshCallback : public mesh::IMeshParameterCallback
{
public:
	Vector4 m_frustumEdges[4];
	float m_parallaxDistance;
	float m_depthDistance;
	float m_opacity;
	float* m_layerAngle;

	virtual void setParameters(render::ProgramParameters* programParameters) const override final
	{
		programParameters->setVectorArrayParameter(c_handleFrustumEdges, m_frustumEdges, sizeof_array(m_frustumEdges));
		programParameters->setFloatParameter(c_handleParallaxDistance, m_parallaxDistance);
		programParameters->setFloatParameter(c_handleDepthDistance, m_depthDistance);
		programParameters->setFloatParameter(c_handleOpacity, m_opacity);
		programParameters->setVectorParameter(c_handleLayerAngle, Vector4::loadUnaligned(m_layerAngle));
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.PrecipitationComponent", PrecipitationComponent, world::IEntityComponent)

PrecipitationComponent::PrecipitationComponent(
	const resource::Proxy< mesh::StaticMesh >& mesh,
	float tiltRate,
	float parallaxDistance,
	float depthDistance,
	float opacity
)
:	m_mesh(mesh)
,	m_tiltRate(tiltRate)
,	m_parallaxDistance(parallaxDistance)
,	m_depthDistance(depthDistance)
,	m_opacity(opacity)
{
	m_layerAngle[0] =
	m_layerAngle[1] =
	m_layerAngle[2] =
	m_layerAngle[3] = 0.0f;

	for (int32_t i = 0; i < sizeof_array(m_lastEyePosition); ++i)
		m_lastEyePosition[i] = Vector4::origo();

	for (int32_t i = 0; i < sizeof_array(m_rotation); ++i)
		m_rotation[i] = Quaternion::identity();
}

void PrecipitationComponent::destroy()
{
	m_mesh.clear();
}

void PrecipitationComponent::setOwner(world::Entity* owner)
{
}

void PrecipitationComponent::setTransform(const Transform& transform)
{
}

Aabb3 PrecipitationComponent::getBoundingBox() const
{
	return Aabb3();
}

void PrecipitationComponent::update(const world::UpdateParams& update)
{
	const struct { float k1, k2, k3; } c_layerFactors[] =
	{
		{  0.7f,  0.4f, -0.02f },
		{ -0.5f, -0.2f,  0.02f },
		{  0.8f,  0.3f,  0.01f },
		{ -0.3f,  0.1f,  0.02f }
	};

	const float x = (float)update.totalTime;
	for (int i = 0; i < 4; ++i)
	{
		const float s1 = std::sin(x);
		const float s2 = std::sin(2.0f * x);
		const float s3 = std::sin(3.0f * x);
		m_layerAngle[i] = (s1 * c_layerFactors[i].k1 + s2 * c_layerFactors[i].k2 + s3 * c_layerFactors[i].k3) * 0.25f;
	}
}

void PrecipitationComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const mesh::StaticMesh::techniqueParts_t* techniqueParts = m_mesh->findTechniqueParts(worldRenderPass.getTechnique());
	if (!techniqueParts)
		return;

	Vector4& lastEyePosition = m_lastEyePosition[worldRenderView.getIndex()];
	Quaternion& rotation = m_rotation[worldRenderView.getIndex()];

	const Matrix44 view = worldRenderView.getView();
	const Matrix44 viewInv = view.inverse();
	const Vector4 eyePosition = viewInv.translation().xyz1();
	const Vector4 movement = lastEyePosition - eyePosition;

	Vector4 pivot = Vector4::zero();
	Scalar angle(0.0f);

	if (movement.length2() > Scalar(FUZZY_EPSILON))
	{
		pivot = cross(movement, Vector4(0.0f, 1.0f, 0.0f)).normalized();
		angle = clamp(movement.length() * 0.8_simd, Scalar(-HALF_PI / 2.0f), Scalar(HALF_PI / 2.0f));
	}

	lastEyePosition = eyePosition;

	rotation = lerp(
		rotation,
		Quaternion::fromAxisAngle(pivot, angle),
		m_tiltRate / 60.0f
	);

	const Frustum& viewFrustum = worldRenderView.getViewFrustum();

	PrecipitationMeshCallback mc;
	mc.m_frustumEdges[0] = viewFrustum.corners[4] - viewFrustum.corners[0];
	mc.m_frustumEdges[1] = viewFrustum.corners[5] - viewFrustum.corners[1];
	mc.m_frustumEdges[2] = viewFrustum.corners[6] - viewFrustum.corners[2];
	mc.m_frustumEdges[3] = viewFrustum.corners[7] - viewFrustum.corners[3];
	mc.m_parallaxDistance = m_parallaxDistance;
	mc.m_depthDistance = m_depthDistance;
	mc.m_opacity = m_opacity;
	mc.m_layerAngle = m_layerAngle;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		*techniqueParts,
		Transform(rotation),
		Transform(rotation),
		0.0f,
		&mc
	);
}

}
