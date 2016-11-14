#ifndef traktor_flash_FlashMovieRenderer_H
#define traktor_flash_FlashMovieRenderer_H

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ColorTransform;
class IDisplayRenderer;
class FlashDictionary;
class FlashSprite;
class FlashSpriteInstance;
class FlashCharacterInstance;
class FlashFrame;

/*! \brief Flash movie renderer.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieRenderer : public Object
{
	T_RTTI_CLASS;

public:
	FlashMovieRenderer(IDisplayRenderer* displayRenderer);

	void renderFrame(
		FlashSpriteInstance* movieInstance,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight
	);

private:
	struct State : public RefCountImpl< IRefCount >
	{
		Aabb2 bounds;
		bool visible;

		State();

		virtual ~State();
	};

	Ref< IDisplayRenderer > m_displayRenderer;
	bool m_wantDirtyRegion;
	static bool ms_forceRedraw;

	void renderSprite(
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteDefault(
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteLayered(
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteWithScalingGrid(
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderCharacter(
		FlashCharacterInstance* characterInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask,
		uint8_t blendMode
	);

	void calculateDirtyRegion(
		FlashCharacterInstance* characterInstance,
		const Matrix33& transform,
		bool visible,
		Aabb2& outDirtyRegion
	);
};

	}
}

#endif	// traktor_flash_FlashMovieRenderer_H
