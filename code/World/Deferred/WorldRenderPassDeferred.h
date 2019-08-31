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
		uint32_t passFlags,
		bool fogEnabled,
		bool depthEnable
	);

	WorldRenderPassDeferred(
		render::handle_t technique,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual void setShaderTechnique(render::Shader* shader) const override final;

	virtual void setShaderCombination(render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const override final;

private:
	render::handle_t m_technique;
	const WorldRenderView& m_worldRenderView;
	uint32_t m_passFlags;
	bool m_fogEnabled;
	bool m_depthEnable;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const;
};

	}
}

