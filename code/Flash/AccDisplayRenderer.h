#ifndef traktor_flash_AccDisplayRenderer_H
#define traktor_flash_AccDisplayRenderer_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Flash/DisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;

	}

	namespace flash
	{

class AccTextureCache;
class AccShape;

/*! \brief Accelerated display renderer.
 * \ingroup Flash
 *
 * This display renderer uses the render system
 * in order to accelerate rendering of SWF shapes.
 */
class T_DLLCLASS AccDisplayRenderer : public DisplayRenderer
{
	T_RTTI_CLASS(AccDisplayRenderer)

public:
	AccDisplayRenderer();

	virtual ~AccDisplayRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		bool clearBackground
	);

	void destroy();

	virtual void begin(const FlashMovie& movie, const SwfColor& backgroundColor);

	virtual void beginMask(bool increment);

	virtual void endMask();

	virtual void renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform);

	virtual void renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform);

	virtual void renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform);

	virtual void end();

private:
	struct CacheEntry
	{
		uint32_t unusedCount;
		Ref< AccShape > shape;
	};

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< AccTextureCache > m_textureCache;
	std::map< uint32_t, CacheEntry > m_shapeCache;
	Vector4 m_frameSize;
	bool m_clearBackground;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
};

	}
}

#endif	// traktor_flash_AccDisplayRenderer_H
