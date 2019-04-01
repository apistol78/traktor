#include "Core/Math/Ray3.h"
#include "Render/Shader.h"
#include "Render/Types.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/SH/SHCoeffs.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/WorldRenderPassDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

enum { MaxForwardLightCount = 2 };

bool s_handlesInitialized = false;
render::handle_t s_techniqueDeferredColor;
render::handle_t s_techniqueVelocityWrite;
render::handle_t s_techniqueIrradianceWrite;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleLastWorld;
render::handle_t s_handleLastWorldView;
render::handle_t s_handleFogEnable;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightColor;

render::handle_t s_handleProbeR0_3;
render::handle_t s_handleProbeR4_7;
render::handle_t s_handleProbeG0_3;
render::handle_t s_handleProbeG4_7;
render::handle_t s_handleProbeB0_3;
render::handle_t s_handleProbeB4_7;
render::handle_t s_handleProbeRGB_8;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");
	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");
	s_techniqueIrradianceWrite = render::getParameterHandle(L"World_IrradianceWrite");

	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");
	s_handleLastWorld = render::getParameterHandle(L"World_LastWorld");
	s_handleLastWorldView = render::getParameterHandle(L"World_LastWorldView");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleLightPositionAndType = render::getParameterHandle(L"World_LightPositionAndType");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"World_LightDirectionAndRange");
	s_handleLightColor = render::getParameterHandle(L"World_LightColor");

	s_handleProbeR0_3 = render::getParameterHandle(L"World_ProbeR0_3");
	s_handleProbeR4_7 = render::getParameterHandle(L"World_ProbeR4_7");
	s_handleProbeG0_3 = render::getParameterHandle(L"World_ProbeG0_3");
	s_handleProbeG4_7 = render::getParameterHandle(L"World_ProbeG4_7");
	s_handleProbeB0_3 = render::getParameterHandle(L"World_ProbeB0_3");
	s_handleProbeB4_7 = render::getParameterHandle(L"World_ProbeB4_7");
	s_handleProbeRGB_8 = render::getParameterHandle(L"World_ProbeRGB_8");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool fogEnabled,
	bool depthEnable
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(fogEnabled)
,	m_depthEnable(depthEnable)
{
	initializeHandles();
}

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(false)
,	m_depthEnable(false)
{
	initializeHandles();
}

render::handle_t WorldRenderPassDeferred::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassDeferred::getPassFlags() const
{
	return m_passFlags;
}

void WorldRenderPassDeferred::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassDeferred::setShaderCombination(render::Shader* shader) const
{
	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthEnable);
	}
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
	if (m_technique == s_techniqueIrradianceWrite)
		setProbeProgramParameters(programParams, world, bounds);
}

void WorldRenderPassDeferred::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * w);

	if (m_technique == s_techniqueVelocityWrite)
	{
		Matrix44 w0 = lastWorld.toMatrix44();
		programParams->setMatrixParameter(s_handleLastWorld, w0);
		programParams->setMatrixParameter(s_handleLastWorldView, m_worldRenderView.getLastView() * w0);
	}
}

void WorldRenderPassDeferred::setProbeProgramParameters(render::ProgramParameters* programParams, const Transform& world, const Aabb3& bounds) const
{
	// \tbd Blend closest 3 probes.

	// \hack For now use brute force...
	AlignedVector< Light > lights;

	for (int i = 0; i < m_worldRenderView.getLightCount(); ++i)
	{
		const Light& light = m_worldRenderView.getLight(i);
		if (light.type == LtProbe && light.probe.shCoeffs != nullptr)
		{
			const auto& c = light.probe.shCoeffs->get();
			if (c.size() >= 9)
				lights.push_back(light);
		}
	}
	if (lights.empty())
		return;

	Vector4 position = world.translation();

	std::sort(lights.begin(), lights.end(), [&](const Light& a, const Light& b) {
		Scalar la = (a.position - position).xyz0().length2();
		Scalar lb = (b.position - position).xyz0().length2();
		return la < lb;
	});

	AlignedVector< Vector4 > c(9);

	if (lights.size() >= 2)
	{
		const auto& c0 = lights[0].probe.shCoeffs->get();
		const auto& c1 = lights[1].probe.shCoeffs->get();

		Vector4 direction = lights[1].position - lights[0].position;
		Scalar length = direction.normalize();

		Vector4 v = (position - lights[0].position).xyz0();
		Scalar ln2 = v.normalize();

		Scalar k = clamp(dot3(direction, v), Scalar(0.0f), Scalar(1.0f));
		for (int i = 0; i < 9; ++i)
		{
			c[i] = c0[i] * (Scalar(1.0f) - k) + c1[i] * k;
		}
	}
	else
		c = lights[0].probe.shCoeffs->get();

	programParams->setVectorParameter(s_handleProbeR0_3, Vector4(c[0].x(), c[1].x(), c[2].x(), c[3].x()));
	programParams->setVectorParameter(s_handleProbeR4_7, Vector4(c[4].x(), c[5].x(), c[6].x(), c[7].x()));
	programParams->setVectorParameter(s_handleProbeG0_3, Vector4(c[0].y(), c[1].y(), c[2].y(), c[3].y()));
	programParams->setVectorParameter(s_handleProbeG4_7, Vector4(c[4].y(), c[5].y(), c[6].y(), c[7].y()));
	programParams->setVectorParameter(s_handleProbeB0_3, Vector4(c[0].z(), c[1].z(), c[2].z(), c[3].z()));
	programParams->setVectorParameter(s_handleProbeB4_7, Vector4(c[4].z(), c[5].z(), c[6].z(), c[7].z()));
	programParams->setVectorParameter(s_handleProbeRGB_8, Vector4(c[8].x(), c[8].y(), c[8].z(), 0.0f));
}

	}
}
