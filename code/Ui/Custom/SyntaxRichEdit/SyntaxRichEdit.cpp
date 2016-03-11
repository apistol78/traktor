#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguage.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SyntaxRichEdit", SyntaxRichEdit, RichEdit)

SyntaxRichEdit::SyntaxRichEdit()
{
	m_attributeDefault[0] = m_attributeDefault[1] = 0;
	m_attributeString[0] = m_attributeString[1] = 0;
	m_attributeNumber[0] = m_attributeNumber[1] = 0;
	m_attributeComment[0] = m_attributeComment[1] = 0;
	m_attributeFunction[0] = m_attributeFunction[1] = 0;
	m_attributeType[0] = m_attributeType[1] = 0;
	m_attributeKeyword[0] = m_attributeKeyword[1] = 0;
	m_attributeSpecial[0] = m_attributeSpecial[1] = 0;
	m_attributePreprocessor[0] = m_attributePreprocessor[1] = 0;
	m_attributeError[0] = m_attributeError[1] = 0;
}

bool SyntaxRichEdit::create(Widget* parent, const std::wstring& text, int32_t style)
{
	if (!RichEdit::create(parent, text, style))
		return false;

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	m_attributeDefault[0] = addTextAttribute(ss->getColor(this, L"color-default"), false, false, false);
	m_attributeDefault[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-default"));

	m_attributeString[0] = addTextAttribute(ss->getColor(this, L"color-string"), false, false, false);
	m_attributeString[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-string"));

	m_attributeNumber[0] = addTextAttribute(ss->getColor(this, L"color-number"), false, false, false);
	m_attributeNumber[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-number"));

	m_attributeComment[0] = addTextAttribute(ss->getColor(this, L"color-comment"), false, true, false);
	m_attributeComment[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-comment"));

	m_attributeFunction[0] = addTextAttribute(ss->getColor(this, L"color-function"), false, false, false);
	m_attributeFunction[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-function"));

	m_attributeType[0] = addTextAttribute(ss->getColor(this, L"color-type"), false, false, false);
	m_attributeType[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-type"));

	m_attributeKeyword[0] = addTextAttribute(ss->getColor(this, L"color-keyword"), false, false, false);
	m_attributeKeyword[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-keyword"));

	m_attributeSpecial[0] = addTextAttribute(ss->getColor(this, L"color-special"), false, false, false);
	m_attributeSpecial[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-special"));

	m_attributePreprocessor[0] = addTextAttribute(ss->getColor(this, L"color-preprocessor"), false, false, false);
	m_attributePreprocessor[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-preprocessor"));

	m_attributeError[0] = addTextAttribute(ss->getColor(this, L"color-error"), false, false, false);
	m_attributeError[1] = addBackgroundAttribute(ss->getColor(this, L"background-color-error"));

	addEventHandler< ContentChangeEvent >(this, &SyntaxRichEdit::eventChange);
	return true;
}

void SyntaxRichEdit::setLanguage(const SyntaxLanguage* language)
{
	m_language = language;
	updateLanguage(0, getLineCount() - 1);
}

const SyntaxLanguage* SyntaxRichEdit::getLanguage() const
{
	return m_language;
}

void SyntaxRichEdit::setErrorHighlight(int32_t line)
{
	updateLanguage(0, getLineCount() - 1);
	if (line >= 0)
	{
		int32_t offset = getLineOffset(line);
		int32_t length = getLineLength(line);
		setAttributes(offset, length, m_attributeError[0], m_attributeError[1]);
	}
}

void SyntaxRichEdit::getOutline(std::list< SyntaxOutline >& outOutline) const
{
	if (!m_language)
		return;

	int32_t fromLine = 0;
	int32_t toLine = getLineCount();

	for (int32_t line = fromLine; line < toLine; ++line)
	{
		std::wstring text = getLine(line);
		m_language->outline(line, text, outOutline);
	}
}

void SyntaxRichEdit::updateLanguage(int32_t fromLine, int32_t toLine)
{
	if (!m_language)
		return;

	SyntaxLanguage::State currentState = SyntaxLanguage::StInvalid;
	int32_t startOffset = getLineOffset(fromLine);
	int32_t endOffset = startOffset;

	for (int32_t line = fromLine; line <= toLine; ++line)
	{
		std::wstring text = getLine(line);
		for (int32_t i = 0; i < int32_t(text.length()); )
		{
			SyntaxLanguage::State state = SyntaxLanguage::StInvalid;
			int32_t consumedChars = 0;

			if (!m_language->consume(text.substr(i), state, consumedChars))
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

					case SyntaxLanguage::StComment:
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

		case SyntaxLanguage::StComment:
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

void SyntaxRichEdit::setText(const std::wstring& text)
{
	RichEdit::setText(text);
	updateLanguage(0, getLineCount());
}

void SyntaxRichEdit::eventChange(ContentChangeEvent* event)
{
	int32_t caretOffset = getCaretOffset();
	int32_t caretLine = getLineFromOffset(caretOffset);
	updateLanguage(caretLine, caretLine);
}

		}
	}
}
