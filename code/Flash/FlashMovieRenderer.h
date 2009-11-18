#ifndef traktor_flash_FlashMovieRenderer_H
#define traktor_flash_FlashMovieRenderer_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class DisplayRenderer;
class FlashMovie;
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
	FlashMovieRenderer(DisplayRenderer* displayRenderer);

	void renderFrame(FlashMovie* movie, FlashSpriteInstance* movieInstance);

private:
	Ref< DisplayRenderer > m_displayRenderer;

	void renderSprite(
		FlashMovie* movie,
		FlashSpriteInstance* spriteInstance,
		const Matrix33& transform
	);

	void renderCharacter(
		FlashMovie* movie,
		FlashCharacterInstance* characterInstance,
		const Matrix33& transform
	);
};

	}
}

#endif	// traktor_flash_FlashMovieRenderer_H
