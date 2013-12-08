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
		const WorldRenderView& worldRenderView,
		bool fogEnabled,
		bool depthEnable
	);

	WorldRenderPassPreLit(
		render::handle_t technique,
		const WorldRenderView& worldRenderView
	);

	virtual render::handle_t getTechnique() const;

	virtual void setShaderTechnique(render::Shader* shader) const;

	virtual void setShaderCombination(render::Shader* shader) const;

	virtual void setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const;

	virtual void setProgramParameters(render::ProgramParameters* programParams, uint32_t priority) const;

	virtual void setProgramParameters(render::ProgramParameters* programParams, uint32_t priority, const Matrix44& world, const Aabb3& bounds) const;

private:
	render::handle_t m_technique;
	const WorldRenderView& m_worldRenderView;
	bool m_fogEnabled;
	bool m_depthEnable;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;
};
	
	}
}

#endif	// traktor_world_WorldRenderPassPreLit_H
