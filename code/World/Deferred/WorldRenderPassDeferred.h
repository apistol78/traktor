#ifndef traktor_world_WorldRenderPassDeferred_H
#define traktor_world_WorldRenderPassDeferred_H

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
class T_DLLCLASS WorldRenderPassDeferred : public IWorldRenderPass
{
	T_RTTI_CLASS;
	
public:
	WorldRenderPassDeferred(
		render::handle_t technique,
		const WorldRenderView& worldRenderView,
		bool firstPassFromEye,
		bool fogEnabled,
		bool depthEnable
	);

	WorldRenderPassDeferred(
		render::handle_t technique,
		const WorldRenderView& worldRenderView,
		bool firstPassFromEye
	);

	virtual render::handle_t getTechnique() const T_OVERRIDE T_FINAL;

	virtual bool isFirstPassFromEye() const T_OVERRIDE T_FINAL;

	virtual void setShaderTechnique(render::Shader* shader) const T_OVERRIDE T_FINAL;

	virtual void setShaderCombination(render::Shader* shader) const T_OVERRIDE T_FINAL;

	virtual void setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const T_OVERRIDE T_FINAL;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const T_OVERRIDE T_FINAL;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb3& bounds) const T_OVERRIDE T_FINAL;

private:
	render::handle_t m_technique;
	const WorldRenderView& m_worldRenderView;
	bool m_firstPassFromEye;
	bool m_fogEnabled;
	bool m_depthEnable;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;
};
	
	}
}

#endif	// traktor_world_WorldRenderPassDeferred_H
