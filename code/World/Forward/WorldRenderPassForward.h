#pragma once

#include "World/IWorldRenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;
class StructBuffer;

	}

	namespace world
	{

/*! World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassForward : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	WorldRenderPassForward(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		uint32_t passFlags,
		const Matrix44& view,
		render::StructBuffer* lightSBuffer,
		uint32_t lightCount,
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
	);

	WorldRenderPassForward(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		uint32_t passFlags,
		const Matrix44& view,
		render::ISimpleTexture* colorMap,
		render::ISimpleTexture* depthMap,
		render::ISimpleTexture* occlusionMap
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual render::Shader::Permutation getPermutation(const render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const override final;

private:
	render::handle_t m_technique;
	render::ProgramParameters* m_sharedParams;
	uint32_t m_passFlags;
	Matrix44 m_view;
	Matrix44 m_viewInverse;
	render::StructBuffer* m_lightSBuffer;
	uint32_t m_lightCount;
	bool m_fogEnabled;
	float m_fogDistanceY;
	float m_fogDistanceZ;
	float m_fogDensityY;
	float m_fogDensityZ;
	Vector4 m_fogColor;
	render::ISimpleTexture* m_colorMap;
	render::ISimpleTexture* m_depthMap;
	render::ISimpleTexture* m_occlusionMap;
	render::ISimpleTexture* m_shadowCascade;
	render::ISimpleTexture* m_shadowAtlas;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;

	void setFogProgramParameters(render::ProgramParameters* programParams) const;

	void setColorMapProgramParameters(render::ProgramParameters* programParams) const;

	void setShadowMapProgramParameters(render::ProgramParameters* programParams) const;

	void setDepthMapProgramParameters(render::ProgramParameters* programParams) const;

	void setOcclusionMapProgramParameters(render::ProgramParameters* programParams) const;
};

	}
}
