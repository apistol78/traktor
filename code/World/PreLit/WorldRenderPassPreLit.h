#ifndef traktor_world_WorldRenderPassPreLit_H
#define traktor_world_WorldRenderPassPreLit_H

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

class ITexture;

	}

	namespace world
	{

class WorldRenderView;

/*! \brief World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassPreLit : public IWorldRenderPass
{
	T_RTTI_CLASS;
	
public:
	WorldRenderPassPreLit(
		render::handle_t technique,
		bool finalPass,
		const WorldRenderView& worldRenderView,
		float depthRange,
		render::ITexture* depthMap,
		render::ITexture* shadowMask,
		render::ITexture* lightMap
	);

	virtual render::handle_t getTechnique() const;

	virtual bool isFinal() const;

	virtual void setShaderTechnique(render::Shader* shader) const;

	virtual void setShaderCombination(render::Shader* shader) const;

	virtual void setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb3& bounds) const;

private:
	render::handle_t m_technique;
	bool m_finalPass;
	const WorldRenderView& m_worldRenderView;
	float m_depthRange;
	render::ITexture* m_depthMap;
	render::ITexture* m_shadowMask;
	render::ITexture* m_lightMap;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const;

	void setShadowMapProgramParameters(render::ProgramParameters* programParams) const;

	void setDepthMapProgramParameters(render::ProgramParameters* programParams) const;

	void setLightMapProgramParameters(render::ProgramParameters* programParams) const;
};
	
	}
}

#endif	// traktor_world_WorldRenderPassPreLit_H
