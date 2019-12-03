#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderTargetSet;
class RenderContext;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class AccShape;
class AccQuad;
class ColorTransform;
class Packer;

/*! \brief
 */
class AccShapeRenderer : public Object
{
	T_RTTI_CLASS;

public:
	AccShapeRenderer();

	bool create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager);

	void destroy();

	void beginFrame();

	void endFrame();

	void beginSprite(
		render::RenderContext* renderContext,
		const SpriteInstance& sprite,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const Vector4& viewSize,
		const Matrix33& transform,
		uint8_t maskReference
	);

	void endSprite(
		render::RenderContext* renderContext,
		const SpriteInstance& sprite,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const Matrix33& transform,
		uint8_t maskReference
	);

	void render(
		render::RenderContext* renderContext,
		AccShape* shape,
		int32_t tag,
		const ColorTransform& cxform,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const Matrix33& transform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference,
		uint8_t blendMode
	);

	bool shouldCull() const;

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
	};

	Ref< render::IRenderTargetSet > m_renderTargetShapes;
	Ref< AccQuad > m_quad;
	AlignedVector< Cache > m_cache;
	Ref< Packer > m_packer;
	int32_t m_renderIntoSlot;
	int32_t m_renderFromSlot;
	int32_t m_cacheAsBitmap;

	void beginCacheAsBitmap(
		render::RenderContext* renderContext,
		const SpriteInstance& spriteInstance,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const Vector4& viewSize,
		const Matrix33& transform,
		uint8_t maskReference
	);

	void endCacheAsBitmap(
		render::RenderContext* renderContext,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const Matrix33& transform,
		uint8_t maskReference
	);
};

	}
}

