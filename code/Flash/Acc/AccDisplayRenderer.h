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
		bool clearBackground
	);

	void destroy();

	/*! \brief Begin rendering Flash movie.
	 *
	 * \param renderView Output render view.
	 * \param correctAspectRatio Render movie with correct aspect ratio.
	 */
	void beginRender(render::IRenderView* renderView, bool correctAspectRatio);

	void endRender();

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
	RefArray< render::RenderTargetSet > m_renderTargetGlyphs;
	Ref< AccTextureCache > m_textureCache;
	Ref< AccQuad > m_quad;
	std::map< uint64_t, CacheEntry > m_shapeCache;
	std::map< uint64_t, render::RenderTargetSet* > m_glyphCache;
	Vector4 m_frameSize;
	float m_aspectRatio;
	float m_scaleX;
	bool m_clearBackground;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
};

	}
}

#endif	// traktor_flash_AccDisplayRenderer_H
