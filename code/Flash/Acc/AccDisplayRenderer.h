#ifndef traktor_flash_AccDisplayRenderer_H
#define traktor_flash_AccDisplayRenderer_H

#include <map>
#include "Core/RefArray.h"
#include "Flash/IDisplayRenderer.h"
#include "Render/Types.h"

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

class AccGlyph;
class AccQuad;
class AccShape;
class AccShapeResources;
class AccShapeVertexPool;
class AccTextureCache;

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
		float aspectRatio,
		uint32_t frameCount,
		bool clearBackground,
		float stereoscopicOffset
	);

	void destroy();

	void build(uint32_t frame);

	void build(render::RenderContext* renderContext);

	void render(render::IRenderView* renderView, uint32_t frame, render::EyeType eye);

	void setViewSize(float width, float height);

	void setAspectRatio(float aspectRatio);

	// \name IDisplayRenderer
	// \{
	
	virtual void preload(const FlashMovie& movie);

	virtual void begin(const FlashMovie& movie, const SwfColor& backgroundColor);

	virtual void beginMask(bool increment);

	virtual void endMask();

	virtual void renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform);

	virtual void renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform);

	virtual void renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform);

	virtual void renderCanvas(const FlashMovie& movie, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform);

	virtual void end();

	// \}

private:
	struct CacheEntry
	{
		uint32_t unusedCount;
		uint32_t tag;
		Ref< AccShape > shape;
	};

	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
	RefArray< render::RenderContext > m_renderContexts;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderContext > m_globalContext;
	Ref< render::RenderTargetSet > m_renderTargetGlyphs;
	Ref< AccShapeResources > m_shapeResources;
	RefArray< AccShapeVertexPool > m_vertexPools;
	AccShapeVertexPool* m_vertexPool;
	Ref< AccTextureCache > m_textureCache;
	Ref< AccGlyph > m_glyph;
	Ref< AccQuad > m_quad;
	std::map< uint64_t, CacheEntry > m_shapeCache;
	std::map< uint64_t, int32_t > m_glyphCache;
	int32_t m_nextIndex;
	Vector4 m_frameSize;
	Vector4 m_viewSize;
	Vector4 m_viewOffset;
	float m_aspectRatio;
	bool m_clearBackground;
	float m_stereoscopicOffset;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
	render::handle_t m_handleScreenOffset;
};

	}
}

#endif	// traktor_flash_AccDisplayRenderer_H
