#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/Forward/WorldRenderPassForward.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassForward", WorldRenderPassForward, IWorldRenderPass)

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	uint32_t passFlags,
	const Matrix44& view,
	render::StructBuffer* tileSBuffer,
	render::StructBuffer* lightSBuffer,
	bool irradianceEnable,
	bool fogEnabled,
	float fogDistanceY,
	float fogDistanceZ,
	float fogDensityY,
	float fogDensityZ,
	const Vector4& fogColor,
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* occlusionMap,
	render::ISimpleTexture* shadowCascade,
	render::ISimpleTexture* shadowAtlas
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_passFlags(passFlags)
,	m_view(view)
,	m_viewInverse(view.inverse())
,	m_tileSBuffer(tileSBuffer)
,	m_lightSBuffer(lightSBuffer)
,	m_fogEnabled(fogEnabled)
,	m_irradianceEnable(irradianceEnable)
,	m_fogDistanceY(fogDistanceY)
,	m_fogDistanceZ(fogDistanceZ)
,	m_fogDensityY(fogDensityY)
,	m_fogDensityZ(fogDensityZ)
,	m_fogColor(fogColor)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_occlusionMap(occlusionMap)
,	m_shadowCascade(shadowCascade)
,	m_shadowAtlas(shadowAtlas)
{
}

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	uint32_t passFlags,
	const Matrix44& view,
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* occlusionMap
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_passFlags(passFlags)
,	m_view(view)
,	m_viewInverse(view.inverse())
,	m_tileSBuffer(nullptr)
,	m_lightSBuffer(nullptr)
,	m_fogEnabled(false)
,	m_fogDistanceY(0.0f)
,	m_fogDistanceZ(0.0f)
,	m_fogDensityY(0.0f)
,	m_fogDensityZ(0.0f)
,	m_fogColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_occlusionMap(occlusionMap)
,	m_shadowCascade(nullptr)
{
}

render::handle_t WorldRenderPassForward::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassForward::getPassFlags() const
{
	return m_passFlags;
}

render::Shader::Permutation WorldRenderPassForward::getPermutation(const render::Shader* shader) const
{
	render::Shader::Permutation perm(m_technique);
	shader->setCombination(s_handleFogEnable, m_fogEnabled, perm);
	shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable, perm);
	shader->setCombination(s_handleShadowEnable, m_shadowCascade != nullptr && m_shadowAtlas != nullptr, perm);
	return perm;
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity());

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_techniqueForwardColor)
	{
		setColorMapProgramParameters(programParams);
		setLightProgramParameters(programParams);
		setFogProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
		setOcclusionMapProgramParameters(programParams);
	}
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_techniqueForwardColor)
	{
		setColorMapProgramParameters(programParams);
		setLightProgramParameters(programParams);
		setFogProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
		setOcclusionMapProgramParameters(programParams);
	}
}

void WorldRenderPassForward::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const
{
	Matrix44 w = world.toMatrix44();
	programParams->attachParameters(m_sharedParams);
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);
}

void WorldRenderPassForward::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	programParams->setStructBufferParameter(s_handleTileSBuffer, m_tileSBuffer);
	programParams->setStructBufferParameter(s_handleLightSBuffer, m_lightSBuffer);
}

void WorldRenderPassForward::setFogProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_fogEnabled)
	{
		programParams->setVectorParameter(s_handleFogDistanceAndDensity, Vector4(m_fogDistanceY, m_fogDistanceZ, m_fogDensityY, m_fogDensityZ));
		programParams->setVectorParameter(s_handleFogColor, m_fogColor);
	}
}

void WorldRenderPassForward::setColorMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_colorMap)
		programParams->setTextureParameter(s_handleColorMap, m_colorMap);
}

void WorldRenderPassForward::setShadowMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_shadowCascade)
		programParams->setTextureParameter(s_handleShadowMapCascade, m_shadowCascade);
	if (m_shadowAtlas)
		programParams->setTextureParameter(s_handleShadowMapAtlas, m_shadowAtlas);
}

void WorldRenderPassForward::setDepthMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_depthMap)
		programParams->setTextureParameter(s_handleDepthMap, m_depthMap);
}

void WorldRenderPassForward::setOcclusionMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_occlusionMap)
		programParams->setTextureParameter(s_handleOcclusionMap, m_occlusionMap);
}

	}
}
