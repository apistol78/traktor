#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Misc/Split.h"
#include "Core/Timer/Timer.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashText.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const SwfCxTransform c_cxWhite = { { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } };

Timer s_timer;

SwfCxTransform convertColor(const SwfColor& c)
{
	SwfCxTransform cxtr =
	{
		{ 0.0f, c.red / 255.0f },
		{ 0.0f, c.green / 255.0f },
		{ 0.0f, c.blue / 255.0f },
		{ 0.0f, c.alpha / 255.0f },
	};
	return cxtr;
}

SwfCxTransform concateCxTransform(const SwfCxTransform& cxt1, const SwfCxTransform& cxt2)
{
	SwfCxTransform cxtr = 
	{
		{ cxt1.red[0]   * cxt2.red[0]  , clamp(cxt1.red[1]   * cxt2.red[0]   + cxt2.red[1],   0.0f, 1.0f) },
		{ cxt1.green[0] * cxt2.green[0], clamp(cxt1.green[1] * cxt2.green[0] + cxt2.green[1], 0.0f, 1.0f) },
		{ cxt1.blue[0]  * cxt2.blue[0] , clamp(cxt1.blue[1]  * cxt2.blue[0]  + cxt2.blue[1],  0.0f, 1.0f) },
		{ cxt1.alpha[0] * cxt2.alpha[0], clamp(cxt1.alpha[1] * cxt2.alpha[0] + cxt2.alpha[1], 0.0f, 1.0f) }
	};
	return cxtr;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieRenderer", FlashMovieRenderer, Object)

bool FlashMovieRenderer::ms_forceRedraw = false;

FlashMovieRenderer::FlashMovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
,	m_wantDirtyRegion(displayRenderer->wantDirtyRegion())
{
}

void FlashMovieRenderer::renderFrame(
	FlashSpriteInstance* movieInstance,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight
)
{
	const SwfColor& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();

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

void FlashMovieRenderer::renderSprite(
	FlashSpriteInstance* spriteInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform,
	bool renderAsMask
)
{
	if (!spriteInstance->isVisible() && !renderAsMask)
		return;

	const FlashSprite* sprite = spriteInstance->getSprite();
	const Aabb2& scalingGrid = sprite->getScalingGrid();

	FlashDictionary* dictionary = spriteInstance->getDictionary();
	T_ASSERT (dictionary);

	const FlashDisplayList& displayList = spriteInstance->getDisplayList();
	const FlashDisplayList::layer_map_t& layers = displayList.getLayers();

	if (scalingGrid.empty())
	{
		m_displayRenderer->beginSprite(
			*spriteInstance,
			transform
		);

		for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); )
		{
			const FlashDisplayList::Layer& layer = i->second;
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
					spriteInstance->getBlendMode()
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
					SbmDefault
				);
				m_displayRenderer->endMask();

				// Render all layers which is clipped to new stencil mask.
				for (++i; i != layers.end(); ++i)
				{
					if (i->first > layer.clipDepth)
						break;

					const FlashDisplayList::Layer& clippedLayer = i->second;
					if (!clippedLayer.instance)
						continue;

					renderCharacter(
						clippedLayer.instance,
						transform,
						cxTransform,
						spriteInstance->getBlendMode()
					);
				}

				// Decrement stencil mask.
				m_displayRenderer->beginMask(false);
				renderCharacter(
					layer.instance,
					transform,
					cxTransform,
					SbmDefault
				);
				m_displayRenderer->endMask();
			}
		}

		m_displayRenderer->endSprite(
			*spriteInstance,
			transform
		);
	}
	else
	{
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

			for (FlashDisplayList::layer_map_t::const_iterator j = layers.begin(); j != layers.end(); )
			{
				const FlashDisplayList::Layer& layer = j->second;
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
						spriteInstance->getBlendMode()
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
						SbmDefault
					);
					m_displayRenderer->endMask();

					// Render all layers which is clipped to new stencil mask.
					for (++j; j != layers.end(); ++j)
					{
						if (j->first > layer.clipDepth)
							break;

						const FlashDisplayList::Layer& clippedLayer = j->second;
						if (!clippedLayer.instance)
							continue;

						renderCharacter(
							clippedLayer.instance,
							T,
							cxTransform,
							spriteInstance->getBlendMode()
						);
					}

					// Decrement stencil mask.
					m_displayRenderer->beginMask(false);
					renderCharacter(
						layer.instance,
						T,
						cxTransform,
						SbmDefault
					);
					m_displayRenderer->endMask();
				}
			}

			// Decrement stencil mask.
			m_displayRenderer->beginMask(false);
			m_displayRenderer->renderQuad(transform, destinationBounds, c_cxWhite);
			m_displayRenderer->endMask();
		}
	}

	FlashCanvas* canvas = spriteInstance->getCanvas();
	if (canvas)
		m_displayRenderer->renderCanvas(
			*dictionary,
			transform,
			*canvas,
			cxTransform
		);
}

void FlashMovieRenderer::renderCharacter(
	FlashCharacterInstance* characterInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform,
	uint8_t blendMode
)
{
	SwfCxTransform cxTransform2 = concateCxTransform(cxTransform, characterInstance->getColorTransform());

	// Don't render completely transparent shapes.
	if (cxTransform2.alpha[0] + cxTransform2.alpha[1] <= FUZZY_EPSILON)
		return;

	FlashDictionary* dictionary = characterInstance->getDictionary();
	T_ASSERT (dictionary);

	const TypeInfo& characterType = type_of(characterInstance);

	// Render sprites.
	if (&characterType == &type_of< FlashSpriteInstance >())
	{
		FlashSpriteInstance* spriteInstance = static_cast< FlashSpriteInstance* >(characterInstance);

		FlashSpriteInstance* maskInstance = spriteInstance->getMask();
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
	if (&characterType == &type_of< FlashShapeInstance >())
	{
		FlashShapeInstance* shapeInstance = static_cast< FlashShapeInstance* >(characterInstance);
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
	if (&characterType == &type_of< FlashMorphShapeInstance >())
	{
		FlashMorphShapeInstance* morphInstance = static_cast< FlashMorphShapeInstance* >(characterInstance);
		m_displayRenderer->renderMorphShape(
			*dictionary,
			transform * morphInstance->getTransform(),
			*morphInstance->getShape(),
			cxTransform2
		);
		return;
	}

	// Render static texts.
	if (&characterType == &type_of< FlashTextInstance >())
	{
		FlashTextInstance* textInstance = static_cast< FlashTextInstance* >(characterInstance);
		if (!textInstance->isVisible())
			return;

		const FlashText* text = textInstance->getText();
		Matrix33 textTransform = transform * textInstance->getTransform() * text->getTextMatrix();

		const AlignedVector< FlashText::Character >& characters = text->getCharacters();
		for (AlignedVector< FlashText::Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		{
			const FlashFont* font = dictionary->getFont(i->fontId);
			if (!font)
				continue;

			const FlashShape* shape = font->getShape(i->glyphIndex);
			if (!shape)
				continue;

			float scaleHeight = 
				font->getCoordinateType() == FlashFont::CtTwips ? 
				1.0f / 1000.0f :
				1.0f / (20.0f * 1000.0f);

			float scaleOffset = i->height * scaleHeight;

			m_displayRenderer->renderGlyph(
				*dictionary,
				textTransform * translate(i->offsetX, i->offsetY) * scale(scaleOffset, scaleOffset),
				font->getMaxDimension(),
				*shape,
				i->color,
				cxTransform2,
				textInstance->getFilter(),
				textInstance->getFilterColor()
			);
		}

		return;
	}

	// Render dynamic texts.
	if (&characterType == &type_of< FlashEditInstance >())
	{
		FlashEditInstance* editInstance = static_cast< FlashEditInstance* >(characterInstance);
		if (!editInstance->isVisible())
			return;

		const Aabb2& textBounds = editInstance->getTextBounds();
		Matrix33 editTransform = transform * editInstance->getTransform();

		const TextLayout* layout = editInstance->getTextLayout();
		T_ASSERT (layout);

#if !defined(__ANDROID__)
		m_displayRenderer->beginMask(true);
		m_displayRenderer->renderQuad(editTransform, textBounds, c_cxWhite);
		m_displayRenderer->endMask();
#endif

		const AlignedVector< TextLayout::Line >& lines = layout->getLines();
		const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();

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
				const TextLayout::Attribute& attrib = attribs[j->attrib];
				const AlignedVector< TextLayout::Character >& chars = j->chars;

				float coordScale = attrib.font->getCoordinateType() == FlashFont::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
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
								convertColor(editInstance->getTextColor())
							);
					}

					if (chars[k].ch != 0)
					{
						uint16_t glyphIndex = attrib.font->lookupIndex(chars[k].ch);

						const FlashShape* glyphShape = attrib.font->getShape(glyphIndex);
						if (!glyphShape)
							continue;

						m_displayRenderer->renderGlyph(
							*dictionary,
							editTransform * translate(textOffsetX + i->x + chars[k].x, textOffsetY + i->y) * scale(fontScale, fontScale),
							attrib.font->getMaxDimension(),
							*glyphShape,
							attrib.color,
							cxTransform2,
							editInstance->getFilter(),
							editInstance->getFilterColor()
						);
					}

					caretEndPosition = textOffsetX + i->x + chars[k].x + chars[k].w;
				}
			}
		}

		if (haveFocus && caret >= 0)
		{
			if (showCaret)
				m_displayRenderer->renderQuad(
					editTransform * translate(caretEndPosition + 50.0f, 0.0f),
					caretBounds,
					convertColor(editInstance->getTextColor())
				);
		}

#if !defined(__ANDROID__)
		m_displayRenderer->beginMask(false);
		m_displayRenderer->renderQuad(editTransform, textBounds, c_cxWhite);
		m_displayRenderer->endMask();
#endif
		return;
	}

	// Render buttons.
	if (&characterType == &type_of< FlashButtonInstance >())
	{
		FlashButtonInstance* buttonInstance = static_cast< FlashButtonInstance* >(characterInstance);
		const FlashButton* button = buttonInstance->getButton();

		Matrix33 buttonTransform = transform * buttonInstance->getTransform();
		uint8_t buttonState = buttonInstance->getState();

		const FlashButton::button_layers_t& layers = button->getButtonLayers();
		for (int32_t j = 0; j < int32_t(layers.size()); ++j)
		{
			const FlashButton::ButtonLayer& layer = layers[j];
			if ((layer.state & buttonState) == 0)
				continue;

			FlashCharacterInstance* referenceInstance = buttonInstance->getCharacterInstance(layer.characterId);
			if (!referenceInstance)
				continue;

			renderCharacter(
				referenceInstance,
				buttonTransform * layer.placeMatrix,
				cxTransform2,
				buttonInstance->getBlendMode()
			);
		}

		return;
	}
}

void FlashMovieRenderer::calculateDirtyRegion(FlashCharacterInstance* characterInstance, const Matrix33& transform, bool visible, Aabb2& outDirtyRegion)
{
	ActionContext* context = characterInstance->getContext();
	T_ASSERT (context);

	bool instanceVisible = characterInstance->isVisible() && visible;
	if (&type_of(characterInstance) == &type_of< FlashSpriteInstance >())
	{
		FlashSpriteInstance* spriteInstance = static_cast< FlashSpriteInstance* >(characterInstance);
		FlashDictionary* dictionary = spriteInstance->getDictionary();
		T_ASSERT (dictionary);

		const Matrix33 T = transform * spriteInstance->getTransform();
		const FlashDisplayList& displayList = spriteInstance->getDisplayList();
		const FlashDisplayList::layer_map_t& layers = displayList.getLayers();

		for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const FlashDisplayList::Layer& layer = i->second;
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

FlashMovieRenderer::State::State()
:	visible(false)
{
}

FlashMovieRenderer::State::~State()
{
	ms_forceRedraw |= visible;
}

	}
}
