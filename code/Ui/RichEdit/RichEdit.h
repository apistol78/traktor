/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Containers/SmallMap.h"
#include "Ui/ColorReference.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;
class ScrollBar;
class SearchControl;
class SearchEvent;

/*! Rich, multi-line, text editor.
 * \ingroup UI
 */
class T_DLLCLASS RichEdit : public Widget
{
	T_RTTI_CLASS;

public:
	enum
	{
		WsNoClipboard = WsUser
	};

	struct ISpecialCharacter : public IRefCount
	{
		virtual int32_t measureWidth(const RichEdit* richEdit) const = 0;

		virtual void draw(Canvas& canvas, const Rect& rc) const = 0;

		virtual void mouseButtonDown(MouseButtonDownEvent* event) const = 0;

		virtual void mouseButtonUp(MouseButtonUpEvent* event) const = 0;

		virtual void mouseDoubleClick(MouseDoubleClickEvent* event) const = 0;
	};

	RichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", uint32_t style = WsDoubleBuffer);

	/*! Set text content of text editor. */
	virtual void setText(const std::wstring& text) override;

	/*! Get textual content of text editor. */
	virtual std::wstring getText() const override;

	/*! Get textual content of text editor. */
	std::wstring getText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const;

	/*! Set font. */
	virtual void setFont(const Font& font) override;

	/*! Define a new text attribute. */
	int32_t addTextAttribute(const ColorReference& textColor, bool bold, bool italic, bool underline);

	/*! Define a new background attribute. */
	int32_t addBackgroundAttribute(const ColorReference& backColor);

	/*! Use text attribute on a selection of the text. */
	void setTextAttribute(int32_t start, int32_t length, int32_t attribute);

	/*! Use background attribute on a selection of the text. */
	void setBackgroundAttribute(int32_t start, int32_t length, int32_t attribute);

	/*! Use background attribute on an entire line. */
	void setBackgroundAttribute(int32_t line, int32_t attribute);

	/*! Use attributes on a selection of the text. */
	void setAttributes(int32_t start, int32_t length, int32_t textAttribute, int32_t backgroundAttribute);

	/*! Define new image. */
	int32_t addImage(IBitmap* image, uint32_t imageCount);

	/*! Use image on a line of text. */
	void setImage(int32_t line, int32_t image);

	/*! Add special character with substiute value. */
	wchar_t addSpecialCharacter(const ISpecialCharacter* specialCharacter);

	/*! Clear attributes, images or content of text. */
	void clear(bool attributes, bool images, bool content, bool specialCharacters);

	/*! Delete selected text. */
	void deleteSelection();

	/*! Insert text at caret's current position. */
	void insert(const std::wstring& text);

	/*! Get offset from client position. */
	int32_t getOffsetFromPosition(const Point& position);

	/*! Get caret's current position. */
	int32_t getCaretOffset() const;

	/*! Get line from client Y position. */
	int32_t getLineFromPosition(int32_t position);

	/*! Get line index of position. */
	int32_t getLineFromOffset(int32_t offset) const;

	/*! Get number of textual lines. */
	int32_t getLineCount() const;

	/*! Get starting offset of line. */
	int32_t getLineOffset(int32_t line) const;

	/*! Get length of line. */
	int32_t getLineLength(int32_t line) const;

	/*! Replace text at a specific line. */
	void setLine(int32_t line, const std::wstring& text);

	/*! Get a single line of text. */
	std::wstring getLine(int32_t line) const;

	/*! Set user object with specific line. */
	void setLineData(int32_t line, Object* data);

	/*! Get user object of specific line. */
	Object* getLineData(int32_t line) const;

	/*! Get selection start offset. */
	int32_t getSelectionStartOffset() const;

	/*! Get selection end offset. */
	int32_t getSelectionStopOffset() const;

	/*! Get selected text. */
	std::wstring getSelectedText() const;

	/*! Get selected text. */
	std::wstring getSelectedText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const;

	/*! Scroll text view to a certain line. */
	bool scrollToLine(int32_t line);

	/*! Get current scrolled offset. */
	int32_t getScrollLine() const;

	/*! Ensure line is visible. */
	bool showLine(int32_t line);

	/*! Place caret at a specified offset. */
	void placeCaret(int32_t offset, bool showLine = false);

	/*! Select all. */
	void selectAll();

	/*! Unselect */
	void unselect();

	/*! Copy selected text into clipboard. */
	bool copy();

	/*! Cut selected text into clipboard. */
	bool cut();

	/*! Paste text from clipboard at caret position. */
	bool paste();

	/*! Show and focus find. */
	bool find();

	/*! Find next occurance of entered search term. */
	bool findNext();

	/*! Show and focus replace. */
	bool replace();

	/*! Replace all occurances of entered replace terms. */
	bool replaceAll();

	/*! Get text inner rectangle. */
	Rect getEditRect() const;

	/*! Get margin width. */
	int32_t getMarginWidth() const;

protected:
	virtual void contentModified();

private:
	struct TextAttribute
	{
		ColorReference textColor;
		bool bold = false;
		bool italic = false;
		bool underline = false;
	};

	struct BackgroundAttribute
	{
		ColorReference backColor;
	};

	struct Line
	{
		int32_t start = 0;
		int32_t stop = 0;
		int32_t image = -1;
		uint16_t attrib = 0xffff;
		Ref< Object > data;
	};

	struct Character
	{
		wchar_t ch = 0;
		uint16_t tai = 0;
		uint16_t bgai = 0;
		uint16_t width = 0;

		Character() = default;

		explicit Character(wchar_t _ch)
		:	ch(_ch)
		,	tai(0)
		,	bgai(0)
		,	width(0)
		{
		}
	};

	struct FindResult
	{
		int32_t line;
		int32_t offset;
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	Ref< SearchControl > m_searchControl;
	AlignedVector< TextAttribute > m_textAttributes;
	AlignedVector< BackgroundAttribute > m_backgroundAttributes;
	Ref< IBitmap > m_image;
	uint32_t m_imageWidth = 0;
	uint32_t m_imageHeight = 0;
	uint32_t m_imageCount = 0;
	AlignedVector< Line > m_lines;
	AlignedVector< Character > m_text;
	SmallMap< wchar_t, Ref< const ISpecialCharacter > > m_specialCharacters;
	bool m_clipboard = true;
	int32_t m_caret = 0;
	bool m_caretBlink = true;
	int32_t m_selectionStart = -1;
	int32_t m_selectionStop = -1;
	int32_t m_lineMargin = 0;
	int32_t m_lineOffsetH = 0;
	int32_t m_widestLineWidth = 0;
	int32_t m_fromCaret = 0;
	wchar_t m_nextSpecialCharacter = 0xff00;
	int32_t m_foundLineAttribute = 0;

	void updateScrollBars();

	void updateCharacterWidths();

	void deleteCharacters();

	void insertCharacter(wchar_t ch, bool issueEvents, int32_t keyState);

	void deleteAt(int32_t offset);

	void insertAt(int32_t offset, wchar_t ch);

	void scrollToCaret();

	void santiyCheck();

	void updateFindPreview() const;

	bool findFirstLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const;

	bool findNextLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const;

	void eventKeyDown(KeyDownEvent* event);

	void eventKey(KeyEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventTimer(TimerEvent* event);

	void eventScroll(ScrollEvent* event);

	void eventSearch(SearchEvent* event);
};

}
