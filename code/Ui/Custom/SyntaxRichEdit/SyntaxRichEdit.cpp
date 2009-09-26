#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguage.h"
#include "Ui/MethodHandler.h"
#include "Core/Misc/StringUtilities.h"
#include "Core/Log/Log.h"

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
,	m_attributeKeyword(0)
,	m_attributeError(0)
{
}

bool SyntaxRichEdit::create(Widget* parent, const std::wstring& text, int style)
{
	if (!RichEdit::create(parent, text, style))
		return false;

	m_attributeDefault = addAttribute(Color(0, 0, 0), Color(255, 255, 255), false, false, false);		// Default
	m_attributeString = addAttribute(Color(120, 120, 120), Color(255, 255, 255), false, false, false);	// String
	m_attributeNumber = addAttribute(Color(0, 0, 120), Color(255, 255, 255), false, false, false);		// Number
	m_attributeComment = addAttribute(Color(40, 120, 40), Color(255, 255, 255), false, true, false);	// Comment
	m_attributeKeyword = addAttribute(Color(0, 0, 255), Color(255, 255, 255), false, false, false);		// Keyword
	m_attributeError = addAttribute(Color(255, 255, 255), Color(255, 0, 0), false, false, false);		// Error

	addChangeEventHandler(createMethodHandler(this, &SyntaxRichEdit::eventChange));
	return true;
}

void SyntaxRichEdit::setLanguage(SyntaxLanguage* language)
{
	m_language = language;
	updateLanguage(0, getLineCount() - 1);
}

void SyntaxRichEdit::setErrorHighlight(int line)
{
	updateLanguage(0, getLineCount() - 1);
	if (line >= 0)
	{
		int offset = getLineOffset(line);
		int length = getLineLength(line);
		setAttribute(offset, length, m_attributeError);
	}
}

void SyntaxRichEdit::updateLanguage(int fromLine, int toLine)
{
	if (!m_language)
		return;

	m_language->begin();

	SyntaxLanguage::State currentState = SyntaxLanguage::StInvalid;
	int startOffset = getLineOffset(fromLine);
	int endOffset = startOffset;

	for (int line = fromLine; line <= toLine; ++line)
	{
		std::wstring text = getLine(line);
		for (int i = 0; i < int(text.length()); )
		{
			SyntaxLanguage::State state = SyntaxLanguage::StInvalid;
			int consumedChars = 0;

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

					case SyntaxLanguage::StKeyword:
						setAttribute(startOffset, endOffset - startOffset, m_attributeKeyword);
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

		m_language->newLine();
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

		case SyntaxLanguage::StKeyword:
			setAttribute(startOffset, endOffset - startOffset, m_attributeKeyword);
			break;
		}
	}

	setAttribute(endOffset, 0, m_attributeDefault);
}

void SyntaxRichEdit::eventChange(Event* event)
{
	int caretOffset = getCaretOffset();
	int caretLine = getLineFromOffset(caretOffset);
	updateLanguage(caretLine, caretLine);
}

		}
	}
}
