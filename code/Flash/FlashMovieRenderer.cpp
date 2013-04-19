#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Split.h"
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
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

SwfCxTransform concateCxTransform(const SwfCxTransform& cxt1, const SwfCxTransform& cxt2)
{
	SwfCxTransform cxtr = 
	{
		{ cxt1.red[0]   * cxt2.red[0]  , clamp(cxt1.red[1]   + cxt1.red[0]   * cxt2.red[1],   0.0f, 1.0f) },
		{ cxt1.green[0] * cxt2.green[0], clamp(cxt1.green[1] + cxt1.green[0] * cxt2.green[1], 0.0f, 1.0f) },
		{ cxt1.blue[0]  * cxt2.blue[0] , clamp(cxt1.blue[1]  + cxt1.blue[0]  * cxt2.blue[1],  0.0f, 1.0f) },
		{ cxt1.alpha[0] * cxt2.alpha[0], clamp(cxt1.alpha[1] + cxt1.alpha[0] * cxt2.alpha[1], 0.0f, 1.0f) }
	};
	return cxtr;
}

bool isWhiteSpace(wchar_t ch)
{
	return ch == 0 || ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieRenderer", FlashMovieRenderer, Object)

FlashMovieRenderer::FlashMovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

void FlashMovieRenderer::renderFrame(
	FlashSpriteInstance* movieInstance,
	const SwfRect& frameBounds,
	float viewWidth,
	float viewHeight,
	const Vector4& viewOffset
)
{
	const SwfColor& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();
	m_displayRenderer->begin(
		*movieInstance->getContext()->getDictionary(),
		backgroundColor,
		frameBounds,
		viewWidth,
		viewHeight,
		viewOffset
	);

	renderSprite(
		movieInstance->getContext()->getDictionary(),
		movieInstance,
		Matrix33::identity(),
		movieInstance->getColorTransform(),
		false
	);

	m_displayRenderer->end();
}

void FlashMovieRenderer::renderSprite(
	FlashDictionary* dictionary,
	FlashSpriteInstance* spriteInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform,
	bool renderAsMask
)
{
	if (!spriteInstance->isVisible() && !renderAsMask)
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
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);
			++i;
		}
		else
		{
			m_displayRenderer->beginMask(true);

			renderCharacter(
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);

			m_displayRenderer->endMask();

			for (++i; i != layers.end(); ++i)
			{
				if (layer.clipDepth > 0 && i->first > layer.clipDepth)
					break;

				const FlashDisplayList::Layer& clippedLayer = i->second;
				if (!clippedLayer.instance)
					continue;

				renderCharacter(
					dictionary,
					clippedLayer.instance,
					transform,
					cxTransform
				);
			}

			m_displayRenderer->beginMask(false);

			renderCharacter(
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);

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
	FlashDictionary* dictionary,
	FlashCharacterInstance* characterInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform
)
{
	// Don't render completely transparent shapes.
	if (cxTransform.alpha[0] + cxTransform.alpha[1] <= FUZZY_EPSILON)
		return;

	// Render basic shapes.
	FlashShapeInstance* shapeInstance = dynamic_type_cast< FlashShapeInstance* >(characterInstance);
	if (shapeInstance)
	{
		m_displayRenderer->renderShape(
			*dictionary,
			transform * shapeInstance->getTransform(),
			*shapeInstance->getShape(),
			concateCxTransform(cxTransform, characterInstance->getColorTransform())
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
			concateCxTransform(cxTransform, characterInstance->getColorTransform())
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
				*shape,
				i->color,
				concateCxTransform(cxTransform, characterInstance->getColorTransform())
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

		const FlashEdit* edit = editInstance->getEdit();
		const FlashFont* font = dictionary->getFont(edit->getFontId());
		if (!font)
			return;

		Matrix33 editTransform = transform * editInstance->getTransform();

		float fontScale =
			font->getCoordinateType() == FlashFont::CtTwips ? 
			1.0f / 1000.0f :
			1.0f / (20.0f * 1000.0f);
		float fontHeight = edit->getFontHeight();

		const SwfRect& bounds = edit->getTextBounds();
		bool wordWrap = edit->wordWrap();

		const SwfColor& color = editInstance->getTextColor();
		float letterSpacing = editInstance->getLetterSpacing() * 200.0f * 2000.0f / fontHeight;
		FlashEditInstance::text_t text = editInstance->getText();

		const float c_magicX = 32.0f * 20.0f;

		float offsetY = fontHeight;

		// Get space width.
		uint16_t spaceGlyphIndex = font->lookupIndex(L' ');
		int16_t spaceWidth = font->getAdvance(spaceGlyphIndex);

		// Render text lines.
		std::vector< std::wstring > words;
		std::vector< float > widths;
		for (FlashEditInstance::text_t::const_iterator i = text.begin(); i != text.end(); ++i)
		{
			words.resize(0);
			Split< std::wstring >::any(*i, L" \t", words);

			// Calculate width of each word.
			widths.resize(words.size());
			for (uint32_t j = 0; j < words.size(); ++j)
			{
				const std::wstring& word = words[j];
				uint32_t wordLength = word.length();

				float wordWidth = 0.0f;
				for (uint32_t k = 0; k < wordLength; ++k)
				{
					uint16_t glyphIndex = font->lookupIndex(word[k]);
					int16_t glyphAdvance = font->getAdvance(glyphIndex);
					if (k < wordLength - 1)
						glyphAdvance += font->lookupKerning(word[k], word[k + 1]);
					wordWidth += glyphAdvance - c_magicX + letterSpacing;
				}

				widths[j] = wordWidth * fontScale * fontHeight;
			}

			// Pack as many words as fits in bounds (only if word wrap enabled); then render each line.
			if (words.empty())
				offsetY += fontHeight;

			uint32_t wordOffsetStart = 0;
			uint32_t wordOffsetEnd = 0;

			while (wordOffsetStart < words.size())
			{
				float lineWidth = 0.0f;
				while (wordOffsetEnd < words.size())
				{
					float wordWidth = widths[wordOffsetEnd];

					if (wordOffsetStart >= wordOffsetEnd)
						lineWidth = wordWidth;
					else
					{
						wordWidth += spaceWidth * fontScale * fontHeight;
						if (wordWrap && lineWidth + wordWidth >= bounds.max.x - bounds.min.x)
							break;
						lineWidth += wordWidth;
					}

					wordOffsetEnd++;
				}

				// Calculate line horizontal offset.
				float offsetX = 0.0f;
				if (edit->getAlign() == FlashEdit::AnCenter)
					offsetX = (bounds.max.x - bounds.min.x - lineWidth) / 2.0f;
				else if (edit->getAlign() == FlashEdit::AnRight)
					offsetX = bounds.max.x - bounds.min.x - lineWidth;

				// Render each word.
				while (wordOffsetStart < wordOffsetEnd)
				{
					const std::wstring& word = words[wordOffsetStart++];
					uint32_t wordLength = word.length();

					for (uint32_t i = 0; i < wordLength; ++i)
					{
						wchar_t ch = word[i];
						uint16_t glyphIndex = font->lookupIndex(ch);

						if (!isWhiteSpace(ch))
						{
							const FlashShape* glyphShape = font->getShape(glyphIndex);
							if (!glyphShape)
								continue;

							m_displayRenderer->renderGlyph(
								*dictionary,
								editTransform * translate(offsetX, offsetY) * scale(fontScale * fontHeight, fontScale * fontHeight),
								*glyphShape,
								color,
								concateCxTransform(cxTransform, characterInstance->getColorTransform())
							);
						}

						int16_t glyphAdvance = font->getAdvance(glyphIndex);
						if (i < wordLength - 1)
							glyphAdvance += font->lookupKerning(word[i], word[i + 1]);

						offsetX += (glyphAdvance - c_magicX + letterSpacing) * fontScale * fontHeight;
					}

					offsetX += spaceWidth * fontScale * fontHeight;
				}

				offsetY += fontHeight;
			}
		}

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
		for (int32_t j = int32_t(layers.size() - 1); j >= 0; --j)
		{
			const FlashButton::ButtonLayer& layer = layers[j];
			if ((layer.state & buttonState) == 0)
				continue;

			FlashCharacterInstance* referenceInstance = buttonInstance->getCharacterInstance(layer.characterId);
			if (!referenceInstance)
				continue;

			renderCharacter(
				dictionary,
				referenceInstance,
				buttonTransform * layer.placeMatrix,
				concateCxTransform(cxTransform, buttonInstance->getColorTransform())
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
				dictionary,
				maskInstance,
				transform * maskInstance->getTransform(),
				maskInstance->getColorTransform(),
				true
			);

			m_displayRenderer->endMask();
		}

		renderSprite(
			dictionary,
			spriteInstance,
			transform * spriteInstance->getTransform(),
			concateCxTransform(cxTransform, spriteInstance->getColorTransform()),
			false
		);

		if (maskInstance)
		{
			m_displayRenderer->beginMask(false);

			renderSprite(
				dictionary,
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

	}
}
