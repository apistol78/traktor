#include "Core/Math/Float.h"
#include "Core/Math/Random.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/ProgramParameters.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

class PrecipitationMeshCallback : public RefCountImpl< mesh::IMeshParameterCallback >
{
public:
	Vector4 m_frustumEdges[4];
	float m_parallaxDistance;
	float m_depthDistance;
	float m_opacity;
	float* m_layerAngle;

	virtual void setCombination(render::Shader* shader) const T_FINAL {}

	virtual void setParameters(render::ProgramParameters* programParameters) const T_FINAL
	{
		programParameters->setVectorArrayParameter(L"Precipitation_FrustumEdges", m_frustumEdges, sizeof_array(m_frustumEdges));
		programParameters->setFloatParameter(L"Precipitation_ParallaxDistance", m_parallaxDistance);
		programParameters->setFloatParameter(L"Precipitation_DepthDistance", m_depthDistance);
		programParameters->setFloatParameter(L"Precipitation_Opacity", m_opacity);
		programParameters->setFloatArrayParameter(L"Precipitation_LayerAngle", m_layerAngle, 4);
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
,	m_lastEyePosition(Vector4::origo())
,	m_rotation(Quaternion::identity())
{
	m_layerAngle[0] =
	m_layerAngle[1] =
	m_layerAngle[2] =
	m_layerAngle[3] = 0.0f;
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
	const struct
	{
		float k1, k2, k3;
	}
	c_layerFactors[] =
	{
		{  0.7f,  0.4f, -0.02f },
		{ -0.5f, -0.2f,  0.02f },
		{  0.8f,  0.3f,  0.01f },
		{ -0.3f,  0.1f,  0.02f }
	};

	float x = update.totalTime;

	for (int i = 0; i < 4; ++i)
	{
		float s1 = std::sin(x);
		float s2 = std::sin(2.0f * x);
		float s3 = std::sin(3.0f * x);

		m_layerAngle[i] = (s1 * c_layerFactors[i].k1 + s2 * c_layerFactors[i].k2 + s3 * c_layerFactors[i].k3) * 0.25f;
	}
}

void PrecipitationComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	const Matrix44& view = worldRenderView.getView();
	Matrix44 viewInv = view.inverse();
	Vector4 eyePosition = viewInv.translation().xyz1();
	Vector4 movement = m_lastEyePosition - eyePosition;

	Vector4 pivot = Vector4::zero();
	Scalar angle(0.0f);

	if (movement.length2() > Scalar(FUZZY_EPSILON))
	{
		pivot = cross(movement, Vector4(0.0f, 1.0f, 0.0f)).normalized();
		angle = clamp(movement.length() * Scalar(0.8f), Scalar(-HALF_PI / 2.0f), Scalar(HALF_PI / 2.0f));
	}

	m_lastEyePosition = eyePosition;

	m_rotation = lerp(
		m_rotation,
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

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		Transform(m_rotation),
		0.0f,
		&mc
	);
}

	}
}
