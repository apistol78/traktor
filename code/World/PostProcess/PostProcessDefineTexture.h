#ifndef traktor_world_PostProcessDefineTexture_H
#define traktor_world_PostProcessDefineTexture_H

#include "Render/Types.h"
#include "Resource/Id.h"
#include "World/PostProcess/PostProcessDefine.h"

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

/*! \brief Define a texture parameter.
 * \ingroup World
 */
class T_DLLCLASS PostProcessDefineTexture : public PostProcessDefine
{
	T_RTTI_CLASS;

public:
	virtual bool define(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight);

	virtual void serialize(ISerializer& s);

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

private:
	std::wstring m_parameter;
	resource::Id< render::ITexture > m_texture;
};

	}
}

#endif	// traktor_world_PostProcessDefineTexture_H
