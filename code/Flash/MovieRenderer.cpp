/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Misc/Split.h"
#include "Core/Timer/Timer.h"
#include "Flash/Dictionary.h"
#include "Flash/Button.h"
#include "Flash/ButtonInstance.h"
#include "Flash/Edit.h"
#include "Flash/EditInstance.h"
#include "Flash/Font.h"
#include "Flash/MorphShapeInstance.h"
#include "Flash/MovieRenderer.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Text.h"
#include "Flash/TextInstance.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const ColorTransform c_cxWhite(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(1.0f, 1.0f, 1.0f, 1.0f));

Timer s_timer;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieRenderer", MovieRenderer, Object)

bool MovieRenderer::ms_forceRedraw = false;

MovieRenderer::MovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
,	m_wantDirtyRegion(displayRenderer->wantDirtyRegion())
{
}

void MovieRenderer::renderFrame(
	SpriteInstance* movieInstance,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight
)
{
	const Color4f& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();

	Aabb2 dirtyRegion;
	if (m_wantDirtyRegion && !ms_forceRedraw)
	{
		calculateDirtyRegion(
			movieInstance,
			Matrix33::identity(),
			true,
			dirtyRegion
		);
	}
	else
		dirtyRegion = frameBounds;

	m_displayRenderer->begin(
		*movieInstance->getDictionary(),
		backgroundColor,
		frameBounds,
		frameTransform,
		viewWidth,
		viewHeight,
		dirtyRegion
	);

	renderSprite(
		movieInstance,
		Matrix33::identity(),
		movieInstance->getColorTransform(),
		false
	);

	m_displayRenderer->end();
	ms_forceRedraw = false;
}

void MovieRenderer::renderSprite(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	if (!spriteInstance->isVisible() && !renderAsMask)
		return;

	const Sprite* sprite = spriteInstance->getSprite();
	const Aabb2& scalingGrid = sprite->getScalingGrid();
	uint8_t blendMode = spriteInstance->getBlendMode();

	if (blendMode == SbmLayer)
		renderSpriteLayered(spriteInstance, transform, cxTransform, renderAsMask);
	else if (!scalingGrid.empty())
		renderSpriteWithScalingGrid(spriteInstance, transform, cxTransform, renderAsMask);
	else
		renderSpriteDefault(spriteInstance, transform, cxTransform, renderAsMask);
}

void MovieRenderer::renderSpriteDefault(
	SpriteInstance* spriteInstance,
	const Matrix33& transform,
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	
	uint8_t blendMode = spriteInstance->getBlendMode();

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
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	uint8_t blendMode = spriteInstance->getBlendMode();

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

				T_ASSERT (!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
					cxTransform,
					renderAsMask,
					SbmAlpha
				);
			}

			// Then render alpha modifying (masking) layer, will replace alpha values.
			if (ie != layers.end())
			{
				T_ASSERT (ie->second.instance);

				renderCharacter(
					ie->second.instance,
					transform,
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

				T_ASSERT (!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
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

				T_ASSERT (!layer.clipEnable);

				renderCharacter(
					layer.instance,
					transform,
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
	const ColorTransform& cxTransform,
	bool renderAsMask
)
{
	const Sprite* sprite = spriteInstance->getSprite();
	const DisplayList& displayList = spriteInstance->getDisplayList();
	const DisplayList::layer_map_t& layers = displayList.getLayers();
	const Aabb2& scalingGrid = sprite->getScalingGrid();
	uint8_t blendMode = spriteInstance->getBlendMode();

	Aabb2 localBounds = spriteInstance->getLocalBounds();
	Aabb2 globalBounds = spriteInstance->getBounds();

	float w0 = scalingGrid.mn.x - localBounds.mn.x;
	float w1 = localBounds.mx.x - scalingGrid.mx.x;

	float h0 = scalingGrid.mn.y - localBounds.mn.y;
	float h1 = localBounds.mx.y - scalingGrid.mx.y;

	float sfx0 = w0 / (localBounds.mx.x - localBounds.mn.x);
	float sfx1 = ((localBounds.mx.x - localBounds.mn.x) - w1) / (localBounds.mx.x - localBounds.mn.x);
	float sfy0 = h0 / (localBounds.mx.y - localBounds.mn.y);
	float sfy1 = ((localBounds.mx.y - localBounds.mn.y) - h1) / (localBounds.mx.y - localBounds.mn.y);

	Vector2 sourceGrid[][2] =
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

	float dfx0 = w0 / (globalBounds.mx.x - globalBounds.mn.x);
	float dfx1 = ((globalBounds.mx.x - globalBounds.mn.x) - w1) / (globalBounds.mx.x - globalBounds.mn.x);
	float dfy0 = h0 / (globalBounds.mx.y - globalBounds.mn.y);
	float dfy1 = ((globalBounds.mx.y - globalBounds.mn.y) - h1) / (globalBounds.mx.y - globalBounds.mn.y);

	Vector2 destinationGrid[][2] =
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
		Aabb2 sourceBounds(
			lerp(localBounds.mn, localBounds.mx, sourceGrid[i][0]),
			lerp(localBounds.mn, localBounds.mx, sourceGrid[i][1])
		);

		Aabb2 destinationBounds(
			lerp(localBounds.mn, localBounds.mx, destinationGrid[i][0]),
			lerp(localBounds.mn, localBounds.mx, destinationGrid[i][1])
		);

		// Calculate local scale transformation.
		Matrix33 Ts = scale((destinationBounds.mx - destinationBounds.mn) / (sourceBounds.mx - sourceBounds.mn));
		Matrix33 Tt0Inv = translate(-sourceBounds.mn);
		Matrix33 Tt1 = translate(destinationBounds.mn);
		Matrix33 T = transform * Tt1 * Ts * Tt0Inv;

		// Increment stencil mask.
		m_displayRenderer->beginMask(true);
		m_displayRenderer->renderQuad(transform, destinationBounds, c_cxWhite);
		m_displayRenderer->endMask();

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
					cxTransform,
					true,
					blendMode
				);
				m_displayRenderer->endMask();
			}
		}

		// Decrement stencil mask.
		m_displayRenderer->beginMask(false);
		m_displayRenderer->renderQuad(transform, destinationBounds, c_cxWhite);
		m_displayRenderer->endMask();
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
	T_ASSERT (dictionary);

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
				maskInstance->getColorTransform(),
				true
			);
			m_displayRenderer->endMask();
		}

		renderSprite(
			spriteInstance,
			transform * spriteInstance->getTransform(),
			cxTransform2,
			false
		);

		if (maskInstance)
		{
			m_displayRenderer->beginMask(false);
			renderSprite(
				maskInstance,
				transform * maskInstance->getTransform(),
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
		Matrix33 textTransform = transform * textInstance->getTransform() * text->getTextMatrix();

		const AlignedVector< Text::Char >& characters = text->getCharacters();
		for (AlignedVector< Text::Char >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		{
			const Font* font = dictionary->getFont(i->fontId);
			if (!font)
				continue;

			const Shape* glyph = font->getShape(i->glyphIndex);
			if (!glyph)
				continue;

			m_displayRenderer->renderGlyph(
				*dictionary,
				textTransform * translate(i->offsetX, i->offsetY),
				font,
				glyph,
				i->height,
				0,
				i->color,
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
		T_ASSERT (layout);

		m_displayRenderer->beginEdit(*editInstance, editTransform);

		const AlignedVector< TextLayout::Line >& lines = layout->getLines();
		const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();
		const RefArray< CharacterInstance >& characters = layout->getCharacters();

		ActionContext* context = editInstance->getContext();
		T_ASSERT (context);
		
		bool haveFocus = bool(context->getFocus() == editInstance);
		bool showCaret = bool((int32_t(s_timer.getElapsedTime() * 2.0f) & 1) == 0);
		
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
					T_ASSERT (characters[j->c - 1]);
					renderCharacter(
						characters[j->c - 1],
						editTransform * translate(textOffsetX + i->offset + i->x, textOffsetY + i->y),
						cxTransform,
						renderAsMask,
						blendMode
					);
				}

				const TextLayout::Attribute& attrib = attribs[j->a];
				const AlignedVector< TextLayout::Character >& chars = j->chars;

				float coordScale = attrib.font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
				float fontScale = coordScale * layout->getFontHeight();

				for (uint32_t k = 0; k < chars.size(); ++k)
				{
					caretEndPosition = textOffsetX + i->x + chars[k].x;

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

		Matrix33 buttonTransform = transform * buttonInstance->getTransform();
		uint8_t buttonState = buttonInstance->getState();

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
				cxTransform2,
				renderAsMask,
				buttonInstance->getBlendMode()
			);
		}

		return;
	}
}

void MovieRenderer::calculateDirtyRegion(CharacterInstance* characterInstance, const Matrix33& transform, bool visible, Aabb2& outDirtyRegion)
{
	ActionContext* context = characterInstance->getContext();
	T_ASSERT (context);

	bool instanceVisible = characterInstance->isVisible() && visible;
	if (&type_of(characterInstance) == &type_of< SpriteInstance >())
	{
		SpriteInstance* spriteInstance = static_cast< SpriteInstance* >(characterInstance);

		const Matrix33 T = transform * spriteInstance->getTransform();
		const DisplayList& displayList = spriteInstance->getDisplayList();
		const DisplayList::layer_map_t& layers = displayList.getLayers();

		for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const DisplayList::Layer& layer = i->second;
			if (layer.instance)
				calculateDirtyRegion(
					layer.instance,
					T,
					instanceVisible,
					outDirtyRegion
				);
		}
	}
	else if (characterInstance == context->getFocus())
	{
		Aabb2 bounds = transform * characterInstance->getBounds();
		outDirtyRegion.contain(bounds);
	}
	else
	{
		State* s = static_cast< State* >(characterInstance->getCacheObject());
		if (!s)
		{
			s = new State();
			characterInstance->setCacheObject(s);
		}

		// Compare state and add to dirty region if mismatch.
		Aabb2 bounds = transform * characterInstance->getBounds();
		if (s->visible != instanceVisible)
		{
			if (s->visible)
			{
				outDirtyRegion.contain(s->bounds);
				s->visible = false;
			}
			else
			{
				outDirtyRegion.contain(bounds);
				s->visible = true;
			}
		}
		else if (instanceVisible && s->bounds != bounds)
		{
			outDirtyRegion.contain(s->bounds);
			outDirtyRegion.contain(bounds);
			s->bounds = bounds;
		}
	}
}

MovieRenderer::State::State()
:	visible(false)
{
}

MovieRenderer::State::~State()
{
	ms_forceRedraw |= true; //visible;
}

	}
}
