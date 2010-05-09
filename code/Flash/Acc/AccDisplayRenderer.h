#ifndef traktor_flash_AccDisplayRenderer_H
#define traktor_flash_AccDisplayRenderer_H

#include <map>
#include "Core/RefArray.h"
#include "Flash/IDisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
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

class IRenderSystem;
class IRenderView;
class RenderContext;
class RenderTargetSet;

	}

	namespace flash
	{

class AccTextureCache;
class AccShape;
class AccQuad;

/*! \brief Accelerated display renderer.
 * \ingroup Flash
 *
 * This display renderer uses the render system
 * in order to accelerate rendering of SWF shapes.
 */
class T_DLLCLASS AccDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	AccDisplayRenderer();

	virtual ~AccDisplayRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		float viewWidth,
		float viewHeight,
		uint32_t frameCount,
		bool clearBackground
	);

	void destroy();

	void build(uint32_t frame, bool correctAspectRatio);

	void render(render::IRenderView* renderView, uint32_t frame);

	void flush(uint32_t frame);

	void setViewSize(float width, float height);

	// \name IDisplayRenderer
	// \{

	virtual void begin(const FlashMovie& movie, const SwfColor& backgroundColor);

	virtual void beginMask(bool increment);

	virtual void endMask();

	virtual void renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform);

	virtual void renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform);

	virtual void renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform);

	virtual void end();

	// \}

private:
	struct CacheEntry
	{
		uint32_t unusedCount;
		Ref< AccShape > shape;
	};

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::RenderContext > m_renderContexts;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderTargetSet > m_renderTargetGlyphs;
	Ref< AccTextureCache > m_textureCache;
	Ref< AccQuad > m_quad;
	std::map< uint64_t, CacheEntry > m_shapeCache;
	std::map< uint64_t, int32_t > m_glyphCache;
	int32_t m_nextIndex;
	Vector4 m_frameSize;
	Vector4 m_viewSize;
	Vector4 m_viewOffset;
	float m_aspectRatio;
	bool m_clearBackground;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
};

	}
}

#endif	// traktor_flash_AccDisplayRenderer_H
