#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/MethodHandler.h"
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
,	m_attributeError(0)
{
}

bool SyntaxRichEdit::create(Widget* parent, const std::wstring& text, int style)
{
	if (!RichEdit::create(parent, text/*, style*/))
		return false;

	m_attributeDefault = addAttribute(Color4ub(0, 0, 0), Color4ub(255, 255, 255), false, false, false);		// Default
	m_attributeString = addAttribute(Color4ub(120, 120, 120), Color4ub(255, 255, 255), false, false, false);	// String
	m_attributeNumber = addAttribute(Color4ub(0, 0, 120), Color4ub(255, 255, 255), false, false, false);		// Number
	m_attributeComment = addAttribute(Color4ub(40, 120, 40), Color4ub(255, 255, 255), false, true, false);	// Comment
	m_attributeFunction = addAttribute(Color4ub(120, 40, 40), Color4ub(255, 255, 255), false, false, false);	// Function
	m_attributeType = addAttribute(Color4ub(160, 40, 255), Color4ub(255, 255, 255), false, false, false);	// Type
	m_attributeKeyword = addAttribute(Color4ub(0, 0, 255), Color4ub(255, 255, 255), false, false, false);	// Keyword
	m_attributeSpecial = addAttribute(Color4ub(255, 140, 0), Color4ub(255, 255, 255), false, false, false);	// Special
	m_attributeError = addAttribute(Color4ub(255, 255, 255), Color4ub(255, 0, 0), false, false, false);		// Error

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

void SyntaxRichEdit::updateLanguage(int fromLine, int toLine)
{
	if (!m_language)
		return;

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
			
		default:
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
