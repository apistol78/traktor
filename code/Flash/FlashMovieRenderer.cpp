#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashText.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashFont.h"
#include "Flash/IDisplayRenderer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieRenderer", FlashMovieRenderer, Object)

FlashMovieRenderer::FlashMovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

void FlashMovieRenderer::renderFrame(FlashMovie* movie, FlashSpriteInstance* movieInstance)
{
	const SwfColor& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();
	m_displayRenderer->begin(*movie, backgroundColor);

	renderSprite(movie, movieInstance, Matrix33::identity());

	m_displayRenderer->end();
}

void FlashMovieRenderer::renderSprite(
	FlashMovie* movie,
	FlashSpriteInstance* spriteInstance,
	const Matrix33& transform
)
{
	if (!spriteInstance->isVisible())
		return;

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

		if (!layer.clipDepth)
		{
			renderCharacter(
				movie,
				layer.instance,
				transform * layer.instance->getTransform()
			);
			++i;
		}
		else
		{
			m_displayRenderer->beginMask(true);

			renderCharacter(
				movie,
				layer.instance,
				transform * layer.instance->getTransform()
			);

			m_displayRenderer->endMask();

			for (++i; i != layers.end() && i->first <= layer.clipDepth; ++i)
			{
				const FlashDisplayList::Layer& clippedLayer = i->second;
				if (!clippedLayer.instance)
					continue;

				renderCharacter(
					movie,
					clippedLayer.instance,
					transform * clippedLayer.instance->getTransform()
				);
			}

			m_displayRenderer->beginMask(false);

			renderCharacter(
				movie,
				layer.instance,
				transform * layer.instance->getTransform()
			);

			m_displayRenderer->endMask();
		}
	}
}

void FlashMovieRenderer::renderCharacter(
	FlashMovie* movie,
	FlashCharacterInstance* characterInstance,
	const Matrix33& transform
)
{
	// Render basic shapes.
	Ref< FlashShapeInstance > shapeInstance = dynamic_type_cast< FlashShapeInstance* >(characterInstance);
	if (shapeInstance)
	{
		m_displayRenderer->renderShape(
			*movie,
			transform,
			*shapeInstance->getShape(),
			characterInstance->getColorTransform()
		);
		return;
	}

	// Render morph shapes.
	Ref< FlashMorphShapeInstance > morphInstance = dynamic_type_cast< FlashMorphShapeInstance* >(characterInstance);
	if (morphInstance)
	{
		m_displayRenderer->renderMorphShape(
			*movie,
			transform,
			*morphInstance->getShape(),
			characterInstance->getColorTransform()
		);
		return;
	}

	// Render static texts.
	Ref< FlashTextInstance > textInstance = dynamic_type_cast< FlashTextInstance* >(characterInstance);
	if (textInstance)
	{
		Ref< const FlashText > text = textInstance->getText();

		const AlignedVector< FlashText::Character >& characters = text->getCharacters();
		for (AlignedVector< FlashText::Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		{
			Ref< const FlashFont > font = movie->getFont(i->fontId);
			if (!font)
				continue;

			const FlashShape* shape = font->getShape(i->glyphIndex);
			if (!shape)
				continue;

			float glyphReferenceX = 0.0f, glyphReferenceY = 0.0f;
			const SwfRect* glyphBounds = font->getBounds(i->glyphIndex);
			if (glyphBounds)
			{
				glyphReferenceX = glyphBounds->min.x;
				glyphReferenceY = glyphBounds->min.y;
			}

			float scaleHeight = 
				font->getCoordinateType() == FlashFont::CtTwips ? 
				1.0f / 1000.0f :
				1.0f / (20.0f * 1000.0f);

			float scaleOffset = i->height * scaleHeight;

			m_displayRenderer->renderGlyph(
				*movie,
				transform * text->getTextMatrix() * translate(i->offsetX - glyphReferenceX, i->offsetY - glyphReferenceY) * scale(scaleOffset, scaleOffset),
				*shape,
				i->color,
				characterInstance->getColorTransform()
			);
		}

		return;
	}

	// Render dynamic texts.
	Ref< FlashEditInstance > editInstance = dynamic_type_cast< FlashEditInstance* >(characterInstance);
	if (editInstance)
	{
		Ref< const FlashEdit > edit = editInstance->getEdit();
		Ref< const FlashFont > font = movie->getFont(edit->getFontId());
		if (!font)
			return;

		float fontScale =
			font->getCoordinateType() == FlashFont::CtTwips ? 
			1.0f / 1000.0f :
			1.0f / (20.0f * 1000.0f);
		float fontHeight = edit->getFontHeight();

		const SwfColor& color = edit->getTextColor();
		const FlashEditInstance::text_t& text = editInstance->getText();

		const float c_magicX = 32.0f * 20.0f;
		const float c_magicY = 8.0f * 20.0f;

		float offsetY = fontHeight - c_magicY;
		for (FlashEditInstance::text_t::const_iterator i = text.begin(); i != text.end(); ++i)
		{
			const std::wstring& line = *i;
			float offsetX = 0.0f;

			// Calculate origin offsets based on alignment.
			if (edit->getAlign() == FlashEdit::AnCenter || edit->getAlign() == FlashEdit::AnRight)
			{
				float width = 0.0f;
				for (uint32_t j = 0; j < line.length(); ++j)
				{
					uint16_t glyphIndex = font->lookupIndex(line[j]);
					int16_t glyphAdvance = font->getAdvance(glyphIndex);
					if (j < line.length() - 1)
					{
						const SwfKerningRecord* kerningRecord = font->lookupKerningRecord(line[j], line[j + 1]);
						if (kerningRecord)
							glyphAdvance += kerningRecord->adjustment;
					}
					width += (glyphAdvance - c_magicX);
				}

				width *= fontScale * fontHeight;

				const SwfRect& bounds = edit->getTextBounds();
				if (edit->getAlign() == FlashEdit::AnCenter)
					offsetX = (bounds.max.x - bounds.min.x - width) / 2.0f;
				else
					offsetX = bounds.max.x - bounds.min.x - width;
			}

			// Render each glyph.
			for (uint32_t j = 0; j < line.length(); ++j)
			{
				wchar_t ch = line[j];
				uint16_t glyphIndex = font->lookupIndex(ch);

				if (iswgraph(ch))
				{
					const FlashShape* glyphShape = font->getShape(glyphIndex);
					if (!glyphShape)
						continue;

					m_displayRenderer->renderGlyph(
						*movie,
						transform * translate(offsetX, offsetY) * scale(fontScale * fontHeight, fontScale * fontHeight),
						*glyphShape,
						color,
						characterInstance->getColorTransform()
					);
				}

				int16_t glyphAdvance = font->getAdvance(glyphIndex);
				if (j < line.length() - 1)
				{
					const SwfKerningRecord* kerningRecord = font->lookupKerningRecord(line[j], line[j + 1]);
					if (kerningRecord)
						glyphAdvance += kerningRecord->adjustment;
				}

				offsetX += (glyphAdvance - c_magicX) * fontScale * fontHeight;
			}

			offsetY += fontHeight;
		}

		return;
	}

	// Render buttons.
	Ref< FlashButtonInstance > buttonInstance = dynamic_type_cast< FlashButtonInstance* >(characterInstance);
	if (buttonInstance)
	{
		Ref< const FlashButton> button = buttonInstance->getButton();

		uint8_t buttonState = buttonInstance->getState();

		const FlashButton::button_layers_t& layers = button->getButtonLayers();
		for (FlashButton::button_layers_t::const_iterator j = layers.begin(); j != layers.end(); ++j)
		{
			if ((j->state & buttonState) == 0)
				continue;

			Ref< FlashCharacterInstance > referenceInstance = buttonInstance->getCharacterInstance(j->characterId);
			if (!referenceInstance)
				continue;

			renderCharacter(
				movie,
				referenceInstance,
				transform * j->placeMatrix
			);
		}

		return;
	}

	// Render sprites.
	Ref< FlashSpriteInstance > spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(characterInstance);
	if (spriteInstance)
	{
		renderSprite(
			movie,
			spriteInstance,
			transform
		);
		return;
	}
}

	}
}
