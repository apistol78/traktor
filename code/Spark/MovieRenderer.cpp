/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Timer/Timer.h"
#include "Spark/Button.h"
#include "Spark/ButtonInstance.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"
#include "Spark/EditInstance.h"
#include "Spark/Font.h"
#include "Spark/MorphShapeInstance.h"
#include "Spark/MovieRenderer.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Text.h"
#include "Spark/TextInstance.h"
#include "Spark/IDisplayRenderer.h"
#include "Spark/TextLayout.h"

namespace traktor::spark
{
	namespace
	{

const ColorTransform c_cxWhite(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(1.0f, 1.0f, 1.0f, 1.0f));

Timer s_timer;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MovieRenderer", MovieRenderer, Object)

MovieRenderer::MovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

void MovieRenderer::render(
	SpriteInstance* movieInstance,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight
)
{
	const Color4f& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();
	const Aabb2 dirtyRegion = frameBounds;

	m_displayRenderer->begin(
		*movieInstance->getDictionary(),
		backgroundColor,
		frameBounds,
		frameTransform,
		viewWidth,
		viewHeight,
		dirtyRegion
	);

	const float fl = std::numeric_limits< float >::max();
	const Aabb2 clipBounds(
		Vector2(-fl, -fl),
		Vector2( fl,  fl)
	);

	renderSprite(
		movieInstance,
		Matrix33::identity(),
		clipBounds,
		movieInstance->getColorTransform(),
		false
	);

	m_displayRenderer->end();
}

void MovieRenderer::renderSprite(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	if (!spriteInstance->isVisible() && !renderAsMask)
		return;

	const Sprite* sprite = spriteInstance->getSprite();
	const Aabb2& scalingGrid = sprite->getScalingGrid();
	const uint8_t blendMode = spriteInstance->getBlendMode();

	if (blendMode == SbmLayer)
		renderSpriteLayered(spriteInstance, transform, clipBounds, cxTransform, renderAsMask);
	else if (!scalingGrid.empty())
		renderSpriteWithScalingGrid(spriteInstance, transform, clipBounds, cxTransform, renderAsMask);
	else
		renderSpriteDefault(spriteInstance, transform, clipBounds, cxTransform, renderAsMask);
}

void MovieRenderer::renderSpriteDefault(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	const uint8_t blendMode = spriteInstance->getBlendMode();

	m_displayRenderer->beginSprite(
		*spriteInstance,
		transform
	);

	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); )
	{
		const DisplayList::Layer& layer = i->second;
		if (!layer.instance)
		{
			++i;
			continue;
		}

		if (!layer.clipEnable)
		{
			renderCharacter(
				layer.instance,
				transform,
				clipBounds,
				cxTransform,
				renderAsMask,
				blendMode
			);
			++i;
		}
		else
		{
			// Increment stencil mask.
			m_displayRenderer->beginMask(true);
			renderCharacter(
				layer.instance,
				transform,
				clipBounds,
				cxTransform,
				true,
				blendMode
			);
			m_displayRenderer->endMask();

			// Render all layers which is clipped to new stencil mask.
			for (++i; i != layers.end(); ++i)
			{
				if (i->first > layer.clipDepth)
					break;

				const DisplayList::Layer& clippedLayer = i->second;
				if (!clippedLayer.instance)
					continue;

				renderCharacter(
					clippedLayer.instance,
					transform,
					clipBounds,
					cxTransform,
					renderAsMask,
					blendMode
				);
			}

			// Decrement stencil mask.
			m_displayRenderer->beginMask(false);
			renderCharacter(
				layer.instance,
				transform,
				clipBounds,
				cxTransform,
				true,
				blendMode
			);
			m_displayRenderer->endMask();
		}
	}

	Canvas* canvas = spriteInstance->getCanvas();
	if (canvas)
		m_displayRenderer->renderCanvas(
			transform,
			*canvas,
			cxTransform,
			blendMode
		);

	m_displayRenderer->endSprite(
		*spriteInstance,
		transform
	);
}

void MovieRenderer::renderSpriteLayered(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	const uint8_t blendMode = spriteInstance->getBlendMode();

	m_displayRenderer->beginSprite(
		*spriteInstance,
		transform
	);

	DisplayList::layer_map_t::const_iterator is = layers.begin();
	while (is != layers.end())
	{
		// Find alpha modifying layer.
		DisplayList::layer_map_t::const_iterator ie = is;
		for (; ie != layers.end(); ++ie)
		{
			const DisplayList::Layer& layer = ie->second;
			if (!layer.instance)
				continue;

			if (
				layer.instance->getBlendMode() == SbmAlpha ||
				layer.instance->getBlendMode() == SbmErase
			)
				break;
		}

		if (ie != layers.end())
		{
			// Render all shapes until modifying layer but with only alpha output to ensure
			// entire area beneath sprite has alpha prepared.
			for (DisplayList::layer_map_t::const_iterator i = is; i != ie; ++i)
			{
				const DisplayList::Layer& layer = i->second;
				if (!layer.instance)
					continue;

				T_ASSERT(!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
					clipBounds,
					cxTransform,
					renderAsMask,
					SbmAlpha
				);
			}

			// Then render alpha modifying (masking) layer, will replace alpha values.
			if (ie != layers.end())
			{
				T_ASSERT(ie->second.instance);

				renderCharacter(
					ie->second.instance,
					transform,
					clipBounds,
					cxTransform,
					renderAsMask,
					blendMode
				);
			}

			// Then finally render all layers until modifying layer using destination alpha for composition.
			for (DisplayList::layer_map_t::const_iterator i = is; i != ie; ++i)
			{
				const DisplayList::Layer& layer = i->second;
				if (!layer.instance)
					continue;

				T_ASSERT(!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
					clipBounds,
					cxTransform,
					renderAsMask,
					blendMode
				);
			}

			is = ie + 1;
		}
		else
		{
			// No complimentary blend layers found, render remaining layers with default blend.
			for (DisplayList::layer_map_t::const_iterator i = is; i != ie; ++i)
			{
				const DisplayList::Layer& layer = i->second;
				if (!layer.instance)
					continue;

				T_ASSERT(!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
					clipBounds,
					cxTransform,
					renderAsMask,
					SbmDefault
				);
			}
			break;
		}
	}

	Canvas* canvas = spriteInstance->getCanvas();
	if (canvas)
		m_displayRenderer->renderCanvas(
			transform,
			*canvas,
			cxTransform,
			blendMode
		);

	m_displayRenderer->endSprite(
		*spriteInstance,
		transform
	);
}

void MovieRenderer::renderSpriteWithScalingGrid(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const Sprite* sprite = spriteInstance->getSprite();
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	const Aabb2& scalingGrid = sprite->getScalingGrid();
	const uint8_t blendMode = spriteInstance->getBlendMode();

	const Aabb2 localBounds = spriteInstance->getLocalBounds();
	const Aabb2 globalBounds = spriteInstance->getBounds();

	const float w0 = scalingGrid.mn.x - localBounds.mn.x;
	const float w1 = localBounds.mx.x - scalingGrid.mx.x;

	const float h0 = scalingGrid.mn.y - localBounds.mn.y;
	const float h1 = localBounds.mx.y - scalingGrid.mx.y;

	const float sfx0 = w0 / (localBounds.mx.x - localBounds.mn.x);
	const float sfx1 = ((localBounds.mx.x - localBounds.mn.x) - w1) / (localBounds.mx.x - localBounds.mn.x);
	const float sfy0 = h0 / (localBounds.mx.y - localBounds.mn.y);
	const float sfy1 = ((localBounds.mx.y - localBounds.mn.y) - h1) / (localBounds.mx.y - localBounds.mn.y);

	const Vector2 sourceGrid[][2] =
	{
		{ Vector2(0.0f, 0.0f), Vector2(sfx0, sfy0) },
		{ Vector2(sfx0, 0.0f), Vector2(sfx1, sfy0) },
		{ Vector2(sfx1, 0.0f), Vector2(1.0f, sfy0) },
		{ Vector2(0.0f, sfy0), Vector2(sfx0, sfy1) },
		{ Vector2(sfx0, sfy0), Vector2(sfx1, sfy1) },
		{ Vector2(sfx1, sfy0), Vector2(1.0f, sfy1) },
		{ Vector2(0.0f, sfy1), Vector2(sfx0, 1.0f) },
		{ Vector2(sfx0, sfy1), Vector2(sfx1, 1.0f) },
		{ Vector2(sfx1, sfy1), Vector2(1.0f, 1.0f) }
	};

	const float dfx0 = w0 / (globalBounds.mx.x - globalBounds.mn.x);
	const float dfx1 = ((globalBounds.mx.x - globalBounds.mn.x) - w1) / (globalBounds.mx.x - globalBounds.mn.x);
	const float dfy0 = h0 / (globalBounds.mx.y - globalBounds.mn.y);
	const float dfy1 = ((globalBounds.mx.y - globalBounds.mn.y) - h1) / (globalBounds.mx.y - globalBounds.mn.y);

	const Vector2 destinationGrid[][2] =
	{
		{ Vector2(0.0f, 0.0f), Vector2(dfx0, dfy0) },
		{ Vector2(dfx0, 0.0f), Vector2(dfx1, dfy0) },
		{ Vector2(dfx1, 0.0f), Vector2(1.0f, dfy0) },
		{ Vector2(0.0f, dfy0), Vector2(dfx0, dfy1) },
		{ Vector2(dfx0, dfy0), Vector2(dfx1, dfy1) },
		{ Vector2(dfx1, dfy0), Vector2(1.0f, dfy1) },
		{ Vector2(0.0f, dfy1), Vector2(dfx0, 1.0f) },
		{ Vector2(dfx0, dfy1), Vector2(dfx1, 1.0f) },
		{ Vector2(dfx1, dfy1), Vector2(1.0f, 1.0f) }
	};

	m_displayRenderer->beginSprite(
		*spriteInstance,
		transform
	);

	for (int32_t i = 0; i < sizeof_array(destinationGrid); ++i)
	{
		const Aabb2 sourceBounds(
			lerp(localBounds.mn, localBounds.mx, sourceGrid[i][0]),
			lerp(localBounds.mn, localBounds.mx, sourceGrid[i][1])
		);

		const Aabb2 destinationBounds(
			lerp(localBounds.mn, localBounds.mx, destinationGrid[i][0]),
			lerp(localBounds.mn, localBounds.mx, destinationGrid[i][1])
		);

		// Calculate local scale transformation.
		const Matrix33 Ts = scale((destinationBounds.mx - destinationBounds.mn) / (sourceBounds.mx - sourceBounds.mn));
		const Matrix33 Tt0Inv = translate(-sourceBounds.mn);
		const Matrix33 Tt1 = translate(destinationBounds.mn);
		const Matrix33 T = transform * Tt1 * Ts * Tt0Inv;

		const Aabb2 subClipBounds(
			sourceBounds.mn,
			sourceBounds.mx
		);

		//// Increment stencil mask.
		//m_displayRenderer->beginMask(true);
		//m_displayRenderer->renderQuad(transform, destinationBounds, c_cxWhite);
		//m_displayRenderer->endMask();

		for (DisplayList::layer_map_t::const_iterator j = layers.begin(); j != layers.end(); )
		{
			const DisplayList::Layer& layer = j->second;
			if (!layer.instance)
			{
				++j;
				continue;
			}

			if (!layer.clipEnable)
			{
				renderCharacter(
					layer.instance,
					T,
					subClipBounds,
					cxTransform,
					renderAsMask,
					blendMode
				);
				++j;
			}
			else
			{
				// Increment stencil mask.
				m_displayRenderer->beginMask(true);
				renderCharacter(
					layer.instance,
					T,
					subClipBounds,
					cxTransform,
					true,
					blendMode
				);
				m_displayRenderer->endMask();

				// Render all layers which is clipped to new stencil mask.
				for (++j; j != layers.end(); ++j)
				{
					if (j->first > layer.clipDepth)
						break;

					const DisplayList::Layer& clippedLayer = j->second;
					if (!clippedLayer.instance)
						continue;

					renderCharacter(
						clippedLayer.instance,
						T,
						subClipBounds,
						cxTransform,
						renderAsMask,
						blendMode
					);
				}

				// Decrement stencil mask.
				m_displayRenderer->beginMask(false);
				renderCharacter(
					layer.instance,
					T,
					subClipBounds,
					cxTransform,
					true,
					blendMode
				);
				m_displayRenderer->endMask();
			}
		}

		//// Decrement stencil mask.
		//m_displayRenderer->beginMask(false);
		//m_displayRenderer->renderQuad(transform, destinationBounds, c_cxWhite);
		//m_displayRenderer->endMask();
	}

	Canvas* canvas = spriteInstance->getCanvas();
	if (canvas)
		m_displayRenderer->renderCanvas(
			transform,
			*canvas,
			cxTransform,
			blendMode
		);

	m_displayRenderer->endSprite(
		*spriteInstance,
		transform
	);
}

void MovieRenderer::renderCharacter(
	CharacterInstance* characterInstance,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const ColorTransform& cxTransform,
	bool renderAsMask,
	uint8_t blendMode
)
{
	ColorTransform cxTransform2 = cxTransform * characterInstance->getColorTransform();

	// Don't render completely transparent, non-mask, shapes.
	if (!renderAsMask && cxTransform2.mul.getAlpha() + cxTransform2.add.getAlpha() <= FUZZY_EPSILON)
		return;

	Dictionary* dictionary = characterInstance->getDictionary();
	T_ASSERT(dictionary);

	const TypeInfo& characterType = type_of(characterInstance);

	// Render sprites.
	if (&characterType == &type_of< SpriteInstance >())
	{
		SpriteInstance* spriteInstance = static_cast< SpriteInstance* >(characterInstance);

		SpriteInstance* maskInstance = spriteInstance->getMask();
		if (maskInstance)
		{
			m_displayRenderer->beginMask(true);
			renderSprite(
				maskInstance,
				transform * maskInstance->getTransform(),
				clipBounds,
				maskInstance->getColorTransform(),
				true
			);
			m_displayRenderer->endMask();
		}

		renderSprite(
			spriteInstance,
			transform * spriteInstance->getTransform(),
			clipBounds,
			cxTransform2,
			false
		);

		if (maskInstance)
		{
			m_displayRenderer->beginMask(false);
			renderSprite(
				maskInstance,
				transform * maskInstance->getTransform(),
				clipBounds,
				maskInstance->getColorTransform(),
				true
			);
			m_displayRenderer->endMask();
		}
		return;
	}

	// Render basic shapes.
	if (&characterType == &type_of< ShapeInstance >())
	{
		ShapeInstance* shapeInstance = static_cast< ShapeInstance* >(characterInstance);
		m_displayRenderer->renderShape(
			*dictionary,
			transform * shapeInstance->getTransform(),
			clipBounds,
			*shapeInstance->getShape(),
			cxTransform2,
			blendMode
		);
		return;
	}

	// Render morph shapes.
	if (&characterType == &type_of< MorphShapeInstance >())
	{
		MorphShapeInstance* morphInstance = static_cast< MorphShapeInstance* >(characterInstance);
		m_displayRenderer->renderMorphShape(
			*dictionary,
			transform * morphInstance->getTransform(),
			clipBounds,
			*morphInstance->getShape(),
			cxTransform2
		);
		return;
	}

	// Render static texts.
	if (&characterType == &type_of< TextInstance >())
	{
		TextInstance* textInstance = static_cast< TextInstance* >(characterInstance);
		if (!textInstance->isVisible())
			return;

		const Text* text = textInstance->getText();
		const Matrix33 textTransform = transform * textInstance->getTransform() * text->getTextMatrix();

		for (const auto& character : text->getCharacters())
		{
			const Font* font = dictionary->getFont(character.fontId);
			if (!font)
				continue;

			const Shape* glyph = font->getShape(character.glyphIndex);
			if (!glyph)
				continue;

			m_displayRenderer->renderGlyph(
				*dictionary,
				textTransform * translate(character.offsetX, character.offsetY),
				clipBounds,
				font,
				glyph,
				character.height,
				0,
				character.color,
				cxTransform2,
				textInstance->getFilter(),
				textInstance->getFilterColor()
			);
		}

		return;
	}

	// Render dynamic texts.
	if (&characterType == &type_of< EditInstance >())
	{
		EditInstance* editInstance = static_cast< EditInstance* >(characterInstance);
		if (!editInstance->isVisible())
			return;

		Matrix33 editTransform = transform * editInstance->getTransform();

		const TextLayout* layout = editInstance->getTextLayout();
		T_ASSERT(layout);

		m_displayRenderer->beginEdit(*editInstance, editTransform);

		const AlignedVector< TextLayout::Line >& lines = layout->getLines();
		const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();
		const RefArray< CharacterInstance >& characters = layout->getCharacters();

		Context* context = editInstance->getContext();
		T_ASSERT(context);

		const bool haveFocus = bool(context->getFocus() == editInstance);
		const bool showCaret = bool((int32_t(s_timer.getElapsedTime() * 2.0f) & 1) == 0);

		int32_t caret = editInstance->getCaret();
		Aabb2 caretBounds(Vector2::zero(), Vector2(100.0f, layout->getFontHeight()));
		float caretEndPosition = 0.0f;
		float textOffsetX = 0.0f;
		float textOffsetY = -(layout->getFontHeight() + layout->getLeading()) * editInstance->getScroll();

		// Calculate edit offset; ie scroll text to left while editing.
		if (haveFocus && lines.size() == 1 && layout->getAlignment() == StaLeft)
		{
			const Aabb2& clipBounds = layout->getBounds();
			float clipWidth = clipBounds.mx.x - clipBounds.mn.x;
			if (lines[0].width > clipWidth)
				textOffsetX = -(lines[0].width - clipWidth);
		}

		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			for (AlignedVector< TextLayout::Word >::const_iterator j = i->words.begin(); j != i->words.end(); ++j)
			{
				if (j->c > 0)
				{
					T_ASSERT(characters[j->c - 1]);
					renderCharacter(
						characters[j->c - 1],
						editTransform * translate(textOffsetX + i->offset + i->x, textOffsetY + i->y),
						clipBounds,
						cxTransform,
						renderAsMask,
						blendMode
					);
				}

				const TextLayout::Attribute& attrib = attribs[j->a];
				const AlignedVector< TextLayout::Character >& chars = j->chars;

				for (uint32_t k = 0; k < chars.size(); ++k)
				{
					caretEndPosition = textOffsetX + i->offset + i->x + chars[k].x;

					if (haveFocus && caret-- == 0)
					{
						if (showCaret)
							m_displayRenderer->renderQuad(
								editTransform * translate(caretEndPosition + 50.0f, 0.0f),
								caretBounds,
								ColorTransform(editInstance->getTextColor())
							);
					}

					if (chars[k].ch != 0)
					{
						uint16_t glyphIndex = attrib.font->lookupIndex(chars[k].ch);
						const Shape* glyph = attrib.font->getShape(glyphIndex);

						m_displayRenderer->renderGlyph(
							*dictionary,
							editTransform * translate(textOffsetX + i->offset + i->x + chars[k].x, textOffsetY + i->y),
							clipBounds,
							attrib.font,
							glyph,
							layout->getFontHeight(),
							chars[k].ch,
							attrib.color,
							cxTransform2,
							editInstance->getFilter(),
							editInstance->getFilterColor()
						);
					}

					caretEndPosition = textOffsetX + i->offset + i->x + chars[k].x + chars[k].w;
				}
			}
		}

		if (haveFocus && caret >= 0)
		{
			if (showCaret)
				m_displayRenderer->renderQuad(
					editTransform * translate(caretEndPosition + 50.0f, 0.0f),
					caretBounds,
					ColorTransform(editInstance->getTextColor())
				);
		}

		m_displayRenderer->endEdit(*editInstance, editTransform);
		return;
	}

	// Render buttons.
	if (&characterType == &type_of< ButtonInstance >())
	{
		ButtonInstance* buttonInstance = static_cast< ButtonInstance* >(characterInstance);
		const Button* button = buttonInstance->getButton();

		const Matrix33 buttonTransform = transform * buttonInstance->getTransform();
		const uint8_t buttonState = buttonInstance->getState();

		const Button::button_layers_t& layers = button->getButtonLayers();
		for (int32_t j = 0; j < int32_t(layers.size()); ++j)
		{
			const Button::ButtonLayer& layer = layers[j];
			if ((layer.state & buttonState) == 0)
				continue;

			CharacterInstance* referenceInstance = buttonInstance->getCharacterInstance(layer.characterId);
			if (!referenceInstance)
				continue;

			renderCharacter(
				referenceInstance,
				buttonTransform * layer.placeMatrix,
				clipBounds,
				cxTransform2,
				renderAsMask,
				buttonInstance->getBlendMode()
			);
		}

		return;
	}
}

}
