#pragma once

#include "World/IEntityRenderer.h"

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

/*! Group entity renderer.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit GroupEntityRenderer(uint32_t filter);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const WorldGatherContext& context,
		const Object* renderable,
		AlignedVector< Light >& outLights
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

	virtual void setup(const WorldSetupContext& context) override final;

private:
	uint32_t m_filter;
};

	}
}

