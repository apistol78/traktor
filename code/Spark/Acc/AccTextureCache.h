#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class AccBitmapRect;
class Bitmap;

/*! Texture cache for accelerated rendering.
 * \ingroup Spark
 */
class AccTextureCache : public Object
{
public:
	AccTextureCache(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		bool reuseTextures
	);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getBitmapTexture(const Bitmap& bitmap);

	void freeTexture(render::ISimpleTexture* texture);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::ISimpleTexture > m_freeTextures;
	bool m_reuseTextures;
};

	}
}

