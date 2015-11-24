#ifndef traktor_flash_AccShapeRenderer_H
#define traktor_flash_AccShapeRenderer_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"

namespace rbp
{

class GuillotineBinPack;

}

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class RenderContext;
class RenderTargetSet;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class AccShape;
class AccQuad;

/*! \brief
 */
class AccShapeRenderer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager);

	void destroy();

	void beginFrame();

	void endFrame();

	void beginCacheAsBitmap(
		render::RenderContext* renderContext,
		const FlashSpriteInstance& spriteInstance,
		const Vector4& frameSize,
		const Vector4& viewSize,
		const Vector4& viewOffset,
		const Matrix33& transform
	);

	void endCacheAsBitmap(
		render::RenderContext* renderContext,
		const Vector4& frameSize,
		const Vector4& viewOffset,
		const Matrix33& transform
	);

	void render(
		render::RenderContext* renderContext,
		AccShape* shape,
		int32_t tag,
		const SwfCxTransform& cxform,
		const Vector4& frameSize,
		const Vector4& viewSize,
		const Vector4& viewOffset,
		const Matrix33& transform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference,
		uint8_t blendMode
	);

private:
	struct Cache
	{
		int32_t tag;
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
		int32_t unused;
		Aabb2 bounds;
		Matrix33 transform;
		bool flipped;
	};

	Ref< render::RenderTargetSet > m_renderTargetShapes;
	Ref< AccQuad > m_quad;
	AlignedVector< Cache > m_cache;
	AutoPtr< rbp::GuillotineBinPack > m_packer;
	int32_t m_renderIntoSlot;
	int32_t m_renderFromSlot;
};

	}
}

#endif	// traktor_flash_AccShapeRenderer_H
