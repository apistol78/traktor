/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MovieRenderer_H
#define traktor_flash_MovieRenderer_H

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
class Dictionary;
class Sprite;
class SpriteInstance;
class CharacterInstance;
class Frame;

/*! \brief Flash movie renderer.
 * \ingroup Flash
 */
class T_DLLCLASS MovieRenderer : public Object
{
	T_RTTI_CLASS;

public:
	MovieRenderer(IDisplayRenderer* displayRenderer);

	void renderFrame(
		SpriteInstance* movieInstance,
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
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteDefault(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteLayered(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteWithScalingGrid(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderCharacter(
		CharacterInstance* characterInstance,
		const Matrix33& transform,
		const ColorTransform& cxTransform,
		bool renderAsMask,
		uint8_t blendMode
	);

	void calculateDirtyRegion(
		CharacterInstance* characterInstance,
		const Matrix33& transform,
		bool visible,
		Aabb2& outDirtyRegion
	);
};

	}
}

#endif	// traktor_flash_MovieRenderer_H
