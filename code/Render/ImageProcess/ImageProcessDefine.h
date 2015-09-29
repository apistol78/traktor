#ifndef traktor_render_ImageProcessDefine_H
#define traktor_render_ImageProcessDefine_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class ImageProcess;
class IRenderSystem;

/*! \brief Post process definition.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessDefine : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight) = 0;
};

	}
}

#endif	// traktor_render_ImageProcessDefine_H
