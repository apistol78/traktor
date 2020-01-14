#pragma once

#include "Render/Types.h"
#include "Resource/Id.h"
#include "Render/Image/ImageProcessDefine.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! Define a texture parameter.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessDefineTexture : public ImageProcessDefine
{
	T_RTTI_CLASS;

public:
	virtual bool define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< ITexture >& getTexture() const { return m_texture; }

private:
	std::wstring m_parameter;
	resource::Id< ITexture > m_texture;
};

	}
}

