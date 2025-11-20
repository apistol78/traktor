/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguage.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompletePopup.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"
#include "Ui/SyntaxRichEdit/Autocomplete/IAutocompleteProvider.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SyntaxRichEdit", SyntaxRichEdit, RichEdit)

SyntaxRichEdit::SyntaxRichEdit()
{
	m_attributeDefault[0] = m_attributeDefault[1] = 0;
	m_attributeString[0] = m_attributeString[1] = 0;
	m_attributeNumber[0] = m_attributeNumber[1] = 0;
	m_attributeSelf[0] = m_attributeSelf[1] = 0;
	m_attributeComment[0] = m_attributeComment[1] = 0;
	m_attributeFunction[0] = m_attributeFunction[1] = 0;
	m_attributeType[0] = m_attributeType[1] = 0;
	m_attributeKeyword[0] = m_attributeKeyword[1] = 0;
	m_attributeSpecial[0] = m_attributeSpecial[1] = 0;
	m_attributePreprocessor[0] = m_attributePreprocessor[1] = 0;
	m_attributeError[0] = m_attributeError[1] = 0;
}

bool SyntaxRichEdit::create(Widget* parent, const std::wstring& text, uint32_t style)
{
	if (!RichEdit::create(parent, text, style))
		return false;

	m_attributeDefault[0] = addTextAttribute(ColorReference(this, L"color-default"), false, false, false);
	m_attributeDefault[1] = addBackgroundAttribute(ColorReference(this, L"background-color-default"));

	m_attributeString[0] = addTextAttribute(ColorReference(this, L"color-string"), false, false, false);
	m_attributeString[1] = addBackgroundAttribute(ColorReference(this, L"background-color-string"));

	m_attributeNumber[0] = addTextAttribute(ColorReference(this, L"color-number"), false, false, false);
	m_attributeNumber[1] = addBackgroundAttribute(ColorReference(this, L"background-color-number"));

	m_attributeSelf[0] = addTextAttribute(ColorReference(this, L"color-self"), false, true, false);
	m_attributeSelf[1] = addBackgroundAttribute(ColorReference(this, L"background-color-self"));

	m_attributeComment[0] = addTextAttribute(ColorReference(this, L"color-comment"), false, true, false);
	m_attributeComment[1] = addBackgroundAttribute(ColorReference(this, L"background-color-comment"));

	m_attributeFunction[0] = addTextAttribute(ColorReference(this, L"color-function"), false, false, false);
	m_attributeFunction[1] = addBackgroundAttribute(ColorReference(this, L"background-color-function"));

	m_attributeType[0] = addTextAttribute(ColorReference(this, L"color-type"), false, false, false);
	m_attributeType[1] = addBackgroundAttribute(ColorReference(this, L"background-color-type"));

	m_attributeKeyword[0] = addTextAttribute(ColorReference(this, L"color-keyword"), false, false, false);
	m_attributeKeyword[1] = addBackgroundAttribute(ColorReference(this, L"background-color-keyword"));

	m_attributeSpecial[0] = addTextAttribute(ColorReference(this, L"color-special"), false, false, false);
	m_attributeSpecial[1] = addBackgroundAttribute(ColorReference(this, L"background-color-special"));

	m_attributePreprocessor[0] = addTextAttribute(ColorReference(this, L"color-preprocessor"), false, false, false);
	m_attributePreprocessor[1] = addBackgroundAttribute(ColorReference(this, L"background-color-preprocessor"));

	m_attributeError[0] = addTextAttribute(ColorReference(this, L"color-error"), false, false, false);
	m_attributeError[1] = addBackgroundAttribute(ColorReference(this, L"background-color-error"));

	addEventHandler< ContentChangeEvent >(this, &SyntaxRichEdit::eventChange);
	addEventHandler< KeyDownEvent >(this, &SyntaxRichEdit::eventKeyDown);
	addEventHandler< KeyEvent >(this, &SyntaxRichEdit::eventKey);
	addEventHandler< FocusEvent >(this, &SyntaxRichEdit::eventFocusLost);

	// Create autocomplete popup
	m_autocompletePopup = new AutocompletePopup();
	m_autocompletePopup->create(this);
	m_autocompletePopup->addEventHandler< AutocompleteSelectEvent >(this, &SyntaxRichEdit::eventAutocompleteSelect);
	m_autocompletePopup->hide();

	return true;
}

void SyntaxRichEdit::setLanguage(const SyntaxLanguage* language)
{
	m_language = language;
	updateLanguage();
}

const SyntaxLanguage* SyntaxRichEdit::getLanguage() const
{
	return m_language;
}

void SyntaxRichEdit::setErrorHighlight(int32_t line)
{
	updateLanguage();
	if (line >= 0)
	{
		const int32_t offset = getLineOffset(line);
		const int32_t length = getLineLength(line);
		setAttributes(offset, length, m_attributeError[0], m_attributeError[1]);
	}
}

void SyntaxRichEdit::getOutline(std::list< SyntaxOutline >& outOutline) const
{
	if (!m_language)
		return;

	const int32_t fromLine = 0;
	const int32_t toLine = getLineCount();

	for (int32_t line = fromLine; line < toLine; ++line)
	{
		const std::wstring text = getLine(line);
		m_language->outline(line, text, outOutline);
	}
}

void SyntaxRichEdit::updateLanguage()
{
	if (!m_language)
		return;

	const int32_t fromLine = 0;
	const int32_t toLine = getLineCount() - 1;

	SyntaxLanguage::State currentState = SyntaxLanguage::StInvalid;
	int32_t startOffset = getLineOffset(fromLine);
	int32_t endOffset = startOffset;

	Ref< SyntaxLanguage::IContext > context = m_language->createContext();

	for (int32_t line = fromLine; line <= toLine; ++line)
	{
		const std::wstring text = getLine(line);
		for (int32_t i = 0; i < int32_t(text.length());)
		{
			SyntaxLanguage::State state = currentState;
			int32_t consumedChars = 0;

			if (!m_language->consume(context, text.substr(i), state, consumedChars))
				break;

			if (state != currentState)
			{
				if (endOffset > startOffset)
				{
					switch (currentState)
					{
					case SyntaxLanguage::StDefault:
						setAttributes(startOffset, endOffset - startOffset, m_attributeDefault[0], m_attributeDefault[1]);
						break;

					case SyntaxLanguage::StString:
						setAttributes(startOffset, endOffset - startOffset, m_attributeString[0], m_attributeString[1]);
						break;

					case SyntaxLanguage::StNumber:
						setAttributes(startOffset, endOffset - startOffset, m_attributeNumber[0], m_attributeNumber[1]);
						break;

					case SyntaxLanguage::StSelf:
						setAttributes(startOffset, endOffset - startOffset, m_attributeSelf[0], m_attributeSelf[1]);
						break;

					case SyntaxLanguage::StLineComment:
						setAttributes(startOffset, endOffset - startOffset, m_attributeComment[0], m_attributeComment[1]);
						break;

					case SyntaxLanguage::StBlockComment:
						setAttributes(startOffset, endOffset - startOffset, m_attributeComment[0], m_attributeComment[1]);
						break;

					case SyntaxLanguage::StFunction:
						setAttributes(startOffset, endOffset - startOffset, m_attributeFunction[0], m_attributeFunction[1]);
						break;

					case SyntaxLanguage::StType:
						setAttributes(startOffset, endOffset - startOffset, m_attributeType[0], m_attributeType[1]);
						break;

					case SyntaxLanguage::StKeyword:
						setAttributes(startOffset, endOffset - startOffset, m_attributeKeyword[0], m_attributeKeyword[1]);
						break;

					case SyntaxLanguage::StSpecial:
						setAttributes(startOffset, endOffset - startOffset, m_attributeSpecial[0], m_attributeSpecial[1]);
						break;

					case SyntaxLanguage::StPreprocessor:
						setAttributes(startOffset, endOffset - startOffset, m_attributePreprocessor[0], m_attributePreprocessor[1]);
						break;

					default:
						break;
					}

					currentState = state;
					startOffset = endOffset;
					endOffset += consumedChars;
				}
				else
				{
					currentState = state;
					endOffset += consumedChars;
				}
			}
			else
			{
				endOffset += consumedChars;
			}

			i += consumedChars;
		}
	}

	if (endOffset > startOffset)
	{
		switch (currentState)
		{
		case SyntaxLanguage::StDefault:
			setAttributes(startOffset, endOffset - startOffset, m_attributeDefault[0], m_attributeDefault[1]);
			break;

		case SyntaxLanguage::StString:
			setAttributes(startOffset, endOffset - startOffset, m_attributeString[0], m_attributeString[1]);
			break;

		case SyntaxLanguage::StNumber:
			setAttributes(startOffset, endOffset - startOffset, m_attributeNumber[0], m_attributeNumber[1]);
			break;

		case SyntaxLanguage::StSelf:
			setAttributes(startOffset, endOffset - startOffset, m_attributeSelf[0], m_attributeSelf[1]);
			break;

		case SyntaxLanguage::StLineComment:
			setAttributes(startOffset, endOffset - startOffset, m_attributeComment[0], m_attributeComment[1]);
			break;

		case SyntaxLanguage::StBlockComment:
			setAttributes(startOffset, endOffset - startOffset, m_attributeComment[0], m_attributeComment[1]);
			break;

		case SyntaxLanguage::StFunction:
			setAttributes(startOffset, endOffset - startOffset, m_attributeFunction[0], m_attributeFunction[1]);
			break;

		case SyntaxLanguage::StType:
			setAttributes(startOffset, endOffset - startOffset, m_attributeType[0], m_attributeType[1]);
			break;

		case SyntaxLanguage::StKeyword:
			setAttributes(startOffset, endOffset - startOffset, m_attributeKeyword[0], m_attributeKeyword[1]);
			break;

		case SyntaxLanguage::StSpecial:
			setAttributes(startOffset, endOffset - startOffset, m_attributeSpecial[0], m_attributeSpecial[1]);
			break;

		case SyntaxLanguage::StPreprocessor:
			setAttributes(startOffset, endOffset - startOffset, m_attributePreprocessor[0], m_attributePreprocessor[1]);
			break;

		default:
			break;
		}
	}

	setAttributes(endOffset, 0, m_attributeDefault[0], m_attributeDefault[1]);
}

void SyntaxRichEdit::contentModified()
{
	updateLanguage();
}

void SyntaxRichEdit::eventChange(ContentChangeEvent* event)
{
	updateLanguage();

	// Update autocomplete content
	if (m_autocompleteProvider)
		m_autocompleteProvider->updateContent(getText());

	// Update autocomplete suggestions
	if (m_autocompleteEnabled)
		updateAutocomplete();
}

void SyntaxRichEdit::setAutocompleteProvider(IAutocompleteProvider* provider)
{
	m_autocompleteProvider = provider;
	if (m_autocompleteProvider)
		m_autocompleteProvider->updateContent(getText());
}

IAutocompleteProvider* SyntaxRichEdit::getAutocompleteProvider() const
{
	return m_autocompleteProvider;
}

void SyntaxRichEdit::setAutocompleteEnabled(bool enabled)
{
	m_autocompleteEnabled = enabled;
	if (!enabled)
		hideAutocomplete();
}

bool SyntaxRichEdit::getAutocompleteEnabled() const
{
	return m_autocompleteEnabled;
}

void SyntaxRichEdit::eventKeyDown(KeyDownEvent* event)
{
	if (!m_autocompleteEnabled || !m_autocompletePopup)
		return;

	// Handle autocomplete popup navigation
	if (m_autocompletePopup->isVisible(true))
	{
		if (event->getVirtualKey() == VkEscape)
		{
			hideAutocomplete();
			event->consume();
			return;
		}
		else if (event->getVirtualKey() == VkDown)
		{
			m_autocompletePopup->selectNext();
			event->consume();
			return;
		}
		else if (event->getVirtualKey() == VkUp)
		{
			m_autocompletePopup->selectPrevious();
			event->consume();
			return;
		}
		else if (event->getVirtualKey() == VkReturn || event->getVirtualKey() == VkTab)
		{
			const AutocompleteSuggestion* suggestion = m_autocompletePopup->getSelectedSuggestion();
			if (suggestion)
			{
				insertAutocompleteSuggestion(*suggestion);
			}
			// Always consume Tab/Enter when autocomplete is visible to prevent inserting tab/newline
			// Set flag so the following KeyEvent also gets consumed
			m_consumeNextKeyEvent = true;
			event->consume();
			return;
		}
	}
}

void SyntaxRichEdit::eventKey(KeyEvent* event)
{
	if (!m_autocompleteEnabled || !m_autocompletePopup)
		return;

	// Check if we need to consume this key event (set by KeyDownEvent handler)
	if (m_consumeNextKeyEvent)
	{
		const wchar_t ch = event->getCharacter();
		if (ch == L'\t' || ch == L'\r' || ch == L'\n')
		{
			m_consumeNextKeyEvent = false;
			event->consume();
			return;
		}
		m_consumeNextKeyEvent = false;
	}

	// Also consume Tab and Enter character events when autocomplete is currently visible
	if (m_autocompletePopup->isVisible(true))
	{
		const wchar_t ch = event->getCharacter();
		if (ch == L'\t' || ch == L'\r' || ch == L'\n')
		{
			event->consume();
			return;
		}
	}
}

void SyntaxRichEdit::eventFocusLost(FocusEvent* event)
{
	hideAutocomplete();
}

void SyntaxRichEdit::insertAutocompleteSuggestion(const AutocompleteSuggestion& suggestion)
{
	// Insert the selected suggestion
	const int32_t caretOffset = getCaretOffset();
	int32_t wordStart = 0;
	const std::wstring currentWord = extractCurrentWord(caretOffset, wordStart);

	// Replace current word with suggestion by manipulating text directly
	if (!currentWord.empty())
	{
		std::wstring text = getText();
		text.erase(wordStart, currentWord.length());
		text.insert(wordStart, suggestion.name);
		const int32_t newCaretPos = wordStart + (int32_t)suggestion.name.length();
		setText(text);
		placeCaret(newCaretPos);
	}

	hideAutocomplete();
}

void SyntaxRichEdit::eventAutocompleteSelect(AutocompleteSelectEvent* event)
{
	const AutocompleteSuggestion& suggestion = event->getSuggestion();
	insertAutocompleteSuggestion(suggestion);
}

void SyntaxRichEdit::showAutocomplete()
{
	if (!m_autocompletePopup || !m_autocompleteProvider)
		return;

	const int32_t caretOffset = getCaretOffset();
	int32_t wordStart = 0;
	const std::wstring currentWord = extractCurrentWord(caretOffset, wordStart);

	if (currentWord.empty())
	{
		hideAutocomplete();
		return;
	}

	// Get current line and column
	const int32_t line = getLineFromOffset(caretOffset);
	const int32_t lineOffset = getLineOffset(line);
	const int32_t column = caretOffset - lineOffset;

	// Build autocomplete context
	AutocompleteContext context;
	context.text = getText();
	context.caretOffset = caretOffset;
	context.currentWord = currentWord;
	context.line = line;
	context.column = column;

	// Get suggestions from provider
	std::vector< AutocompleteSuggestion > suggestions;
	if (!m_autocompleteProvider->getSuggestions(context, suggestions) || suggestions.empty())
	{
		hideAutocomplete();
		return;
	}

	// Calculate caret position in client coordinates
	const Rect innerRect = getInnerRect();
	const int32_t scrollLine = getScrollLine();
	const FontMetric fm = getFontMetric();
	const int32_t lineHeight = fm.getHeight() + pixel(Unit(1));

	// Y position: bottom of current line (to position popup one line below caret)
	const int32_t caretY = innerRect.top + (line - scrollLine + 1) * lineHeight;

	// X position: calculate from word start position and save it (don't recalculate as user types)
	if (!m_autocompletePopup->isVisible(true))
	{
		// First time showing - calculate X position from word start
		int32_t wordStart = 0;
		extractCurrentWord(caretOffset, wordStart);
		const int32_t wordStartLine = getLineFromOffset(wordStart);
		const int32_t wordStartLineOffset = getLineOffset(wordStartLine);
		const int32_t marginWidth = getMarginWidth();
		// Use getAccumulatedWidth to properly handle tabs and special characters
		const int32_t textWidth = getAccumulatedWidth(wordStartLineOffset, wordStart);
		m_autocompletePopupX = innerRect.left + marginWidth + 2 + textWidth;
	}

	// Popup is a child widget, so use client coordinates (not screen coordinates)
	const int32_t popupWidth = 400;
	const int32_t popupHeight = 200;

	m_autocompletePopup->setRect(Rect(m_autocompletePopupX, caretY, m_autocompletePopupX + popupWidth, caretY + popupHeight));
	m_autocompletePopup->setSuggestions(m_autocompleteProvider, suggestions);
	m_autocompletePopup->show();
}

void SyntaxRichEdit::hideAutocomplete()
{
	if (m_autocompletePopup)
		m_autocompletePopup->hide();
}

void SyntaxRichEdit::updateAutocomplete()
{
	if (!m_autocompleteProvider)
		return;

	const int32_t caretOffset = getCaretOffset();
	int32_t wordStart = 0;
	const std::wstring currentWord = extractCurrentWord(caretOffset, wordStart);

	// Show autocomplete if word is different or new
	if (currentWord != m_lastWord || wordStart != m_lastWordOffset)
	{
		m_lastWord = currentWord;
		m_lastWordOffset = wordStart;

		if (currentWord.length() >= 2)
			showAutocomplete();
		else
			hideAutocomplete();
	}
}

std::wstring SyntaxRichEdit::extractCurrentWord(int32_t caretOffset, int32_t& outWordStart) const
{
	const std::wstring text = getText();
	if (caretOffset <= 0 || caretOffset > (int32_t)text.length())
	{
		outWordStart = caretOffset;
		return L"";
	}

	// Find word start (alphanumeric or underscore)
	int32_t wordStart = caretOffset - 1;
	while (wordStart > 0 && (iswalnum(text[wordStart]) || text[wordStart] == L'_'))
		wordStart--;

	if (!iswalnum(text[wordStart]) && text[wordStart] != L'_')
		wordStart++;

	// Extract word
	outWordStart = wordStart;
	if (wordStart >= caretOffset)
		return L"";

	return text.substr(wordStart, caretOffset - wordStart);
}

}

