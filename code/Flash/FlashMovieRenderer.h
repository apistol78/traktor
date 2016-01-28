#ifndef traktor_flash_FlashMovieRenderer_H
#define traktor_flash_FlashMovieRenderer_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"

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

struct SwfCxTransform;

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
	struct State
	{
		Aabb2 bounds;
		bool visible;

		State()
		:	visible(false)
		{
		}
	};

	Ref< IDisplayRenderer > m_displayRenderer;
	SmallMap< int32_t, State > m_states[2];
	int32_t m_count;

	void renderSprite(
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform,
		const SwfCxTransform& cxTransform,
		bool renderAsMask
	);

	void renderCharacter(
		FlashCharacterInstance* characterInstance,
		const Matrix33& transform,
		const SwfCxTransform& cxTransform,
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
