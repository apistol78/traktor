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
:	m_attributeDefault(0)
,	m_attributeString(0)
,	m_attributeNumber(0)
,	m_attributeComment(0)
,	m_attributeFunction(0)
,	m_attributeType(0)
,	m_attributeKeyword(0)
,	m_attributeSpecial(0)
,	m_attributePreprocessor(0)
,	m_attributeError(0)
{
}

bool SyntaxRichEdit::create(Widget* parent, const std::wstring& text, int32_t style)
{
	if (!RichEdit::create(parent, text, style))
		return false;

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	m_attributeDefault = addAttribute(ss->getColor(this, L"color-default"), ss->getColor(this, L"background-color-default"), false, false, false);
	m_attributeString = addAttribute(ss->getColor(this, L"color-string"), ss->getColor(this, L"background-color-string"), false, false, false);
	m_attributeNumber = addAttribute(ss->getColor(this, L"color-number"), ss->getColor(this, L"background-color-number"), false, false, false);
	m_attributeComment = addAttribute(ss->getColor(this, L"color-comment"), ss->getColor(this, L"background-color-comment"), false, true, false);
	m_attributeFunction = addAttribute(ss->getColor(this, L"color-function"), ss->getColor(this, L"background-color-function"), false, false, false);
	m_attributeType = addAttribute(ss->getColor(this, L"color-type"), ss->getColor(this, L"background-color-type"), false, false, false);
	m_attributeKeyword = addAttribute(ss->getColor(this, L"color-keyword"), ss->getColor(this, L"background-color-keyword"), false, false, false);
	m_attributeSpecial = addAttribute(ss->getColor(this, L"color-special"), ss->getColor(this, L"background-color-special"), false, false, false);
	m_attributePreprocessor = addAttribute(ss->getColor(this, L"color-preprocessor"), ss->getColor(this, L"background-color-preprocessor"), false, false, false);
	m_attributeError = addAttribute(ss->getColor(this, L"color-error"), ss->getColor(this, L"background-color-error"), false, false, false);

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
		setAttribute(offset, length, m_attributeError);
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
						setAttribute(startOffset, endOffset - startOffset, m_attributeDefault);
						break;

					case SyntaxLanguage::StString:
						setAttribute(startOffset, endOffset - startOffset, m_attributeString);
						break;

					case SyntaxLanguage::StNumber:
						setAttribute(startOffset, endOffset - startOffset, m_attributeNumber);
						break;

					case SyntaxLanguage::StComment:
						setAttribute(startOffset, endOffset - startOffset, m_attributeComment);
						break;

					case SyntaxLanguage::StFunction:
						setAttribute(startOffset, endOffset - startOffset, m_attributeFunction);
						break;

					case SyntaxLanguage::StType:
						setAttribute(startOffset, endOffset - startOffset, m_attributeType);
						break;

					case SyntaxLanguage::StKeyword:
						setAttribute(startOffset, endOffset - startOffset, m_attributeKeyword);
						break;

					case SyntaxLanguage::StSpecial:
						setAttribute(startOffset, endOffset - startOffset, m_attributeSpecial);
						break;

					case SyntaxLanguage::StPreprocessor:
						setAttribute(startOffset, endOffset - startOffset, m_attributePreprocessor);
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
			setAttribute(startOffset, endOffset - startOffset, m_attributeDefault);
			break;

		case SyntaxLanguage::StString:
			setAttribute(startOffset, endOffset - startOffset, m_attributeString);
			break;

		case SyntaxLanguage::StNumber:
			setAttribute(startOffset, endOffset - startOffset, m_attributeNumber);
			break;

		case SyntaxLanguage::StComment:
			setAttribute(startOffset, endOffset - startOffset, m_attributeComment);
			break;

		case SyntaxLanguage::StFunction:
			setAttribute(startOffset, endOffset - startOffset, m_attributeFunction);
			break;

		case SyntaxLanguage::StType:
			setAttribute(startOffset, endOffset - startOffset, m_attributeType);
			break;

		case SyntaxLanguage::StKeyword:
			setAttribute(startOffset, endOffset - startOffset, m_attributeKeyword);
			break;
			
		case SyntaxLanguage::StSpecial:
			setAttribute(startOffset, endOffset - startOffset, m_attributeSpecial);
			break;

		case SyntaxLanguage::StPreprocessor:
			setAttribute(startOffset, endOffset - startOffset, m_attributePreprocessor);
			break;

		default:
			break;
		}
	}

	setAttribute(endOffset, 0, m_attributeDefault);
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
