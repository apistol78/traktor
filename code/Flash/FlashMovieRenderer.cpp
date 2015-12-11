#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
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

const Vector2 c_marginDirtyRegion(2.0f * 20.0f, 2.0f * 20.0f);
const SwfCxTransform c_cxWhite = { { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } };

Timer s_timer;

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

FlashMovieRenderer::FlashMovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

void FlashMovieRenderer::renderFrame(
	FlashSpriteInstance* movieInstance,
	const Aabb2& frameBounds,
	float viewWidth,
	float viewHeight,
	const Vector4& viewOffset
)
{
	const SwfColor& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();

	Aabb2 dirtyRegion;
	calculateDirtyRegion(
		movieInstance,
		Matrix33::identity(),
		dirtyRegion
	);
	if (!dirtyRegion.empty())
	{
		dirtyRegion.mn -= c_marginDirtyRegion;
		dirtyRegion.mx += c_marginDirtyRegion;
	}

	m_displayRenderer->begin(
		*movieInstance->getDictionary(),
		backgroundColor,
		frameBounds,
		viewWidth,
		viewHeight,
		viewOffset,
		dirtyRegion
	);

	renderSprite(
		movieInstance,
		Matrix33::identity(),
		movieInstance->getColorTransform(),
		false
	);

	m_displayRenderer->end();
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

	m_displayRenderer->beginSprite(
		*spriteInstance,
		transform
	);

	FlashDictionary* dictionary = spriteInstance->getDictionary();
	T_ASSERT (dictionary);

	const FlashDisplayList& displayList = spriteInstance->getDisplayList();
	const FlashDisplayList::layer_map_t& layers = displayList.getLayers();

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

	// Render basic shapes.
	FlashShapeInstance* shapeInstance = dynamic_type_cast< FlashShapeInstance* >(characterInstance);
	if (shapeInstance)
	{
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
	FlashMorphShapeInstance* morphInstance = dynamic_type_cast< FlashMorphShapeInstance* >(characterInstance);
	if (morphInstance)
	{
		m_displayRenderer->renderMorphShape(
			*dictionary,
			transform * morphInstance->getTransform(),
			*morphInstance->getShape(),
			cxTransform2
		);
		return;
	}

	// Render static texts.
	FlashTextInstance* textInstance = dynamic_type_cast< FlashTextInstance* >(characterInstance);
	if (textInstance)
	{
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
	FlashEditInstance* editInstance = dynamic_type_cast< FlashEditInstance* >(characterInstance);
	if (editInstance)
	{
		if (!editInstance->isVisible())
			return;

		const Aabb2& textBounds = editInstance->getEdit()->getTextBounds();
		Matrix33 editTransform = transform * editInstance->getTransform();

		const TextLayout* layout = editInstance->getTextLayout();
		T_ASSERT (layout);

		m_displayRenderer->beginMask(true);
		m_displayRenderer->renderQuad(editTransform, textBounds, c_cxWhite);
		m_displayRenderer->endMask();

		const AlignedVector< TextLayout::Line >& lines = layout->getLines();
		const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();

		ActionContext* context = editInstance->getContext();
		T_ASSERT (context);
		
		bool haveFocus = bool(context->getFocus() == editInstance);
		bool showCaret = bool((int32_t(s_timer.getElapsedTime() * 2.0f) & 1) == 0);
		
		int32_t caret = editInstance->getCaret();
		Aabb2 caretBounds(Vector2(0.0f, 0.0f), Vector2(100.0f, layout->getFontHeight()));
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
								c_cxWhite
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
					c_cxWhite
				);
		}

		m_displayRenderer->beginMask(false);
		m_displayRenderer->renderQuad(editTransform, textBounds, c_cxWhite);
		m_displayRenderer->endMask();

		return;
	}

	// Render buttons.
	FlashButtonInstance* buttonInstance = dynamic_type_cast< FlashButtonInstance* >(characterInstance);
	if (buttonInstance)
	{
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

	// Render sprites.
	FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(characterInstance);
	if (spriteInstance)
	{
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
}

void FlashMovieRenderer::calculateDirtyRegion(FlashCharacterInstance* characterInstance, const Matrix33& transform, Aabb2& outDirtyRegion)
{
	Matrix33 T = transform * characterInstance->getTransform();

	if (FlashShapeInstance* shapeInstance = dynamic_type_cast< FlashShapeInstance* >(characterInstance))
	{
		Aabb2 bounds = T * shapeInstance->getShape()->getShapeBounds();
		State& s = m_states[shapeInstance->getCacheTag()];
		if (s.bounds != bounds)
		{
			outDirtyRegion.contain(s.bounds);
			outDirtyRegion.contain(bounds);
			s.bounds = bounds;
		}
	}

	if (FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(characterInstance))
	{
		FlashDictionary* dictionary = spriteInstance->getDictionary();
		T_ASSERT (dictionary);

		const FlashDisplayList& displayList = spriteInstance->getDisplayList();
		const FlashDisplayList::layer_map_t& layers = displayList.getLayers();

		for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const FlashDisplayList::Layer& layer = i->second;
			if (layer.instance)
				calculateDirtyRegion(
					layer.instance,
					T,
					outDirtyRegion
				);
		}
	}
}

	}
}
