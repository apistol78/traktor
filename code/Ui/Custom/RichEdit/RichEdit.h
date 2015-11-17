#ifndef traktor_ui_custom_RichEdit_H
#define traktor_ui_custom_RichEdit_H

#include <list>
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class ScrollBar;

		namespace custom
		{

/*! \brief Rich, multi-line, text editor.
 * \ingroup UIC
 */
class T_DLLCLASS RichEdit : public Widget
{
	T_RTTI_CLASS;

public:
	RichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", int32_t style = WsClientBorder | WsDoubleBuffer);

	/*! \brief Set text content of text editor. */
	virtual void setText(const std::wstring& text);
	
	/*! \brief Get textual content of text editor. */
	virtual std::wstring getText() const;

	/*! \brief Define a new attribute. */
	int32_t addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline);

	/*! \brief Use attribute on a selection of the text. */
	void setAttribute(int32_t start, int32_t length, int32_t attribute);

	/*! \brief Define new image. */
	int32_t addImage(Bitmap* image, uint32_t imageCount);

	/*! \brief Use image on a line of text. */
	void setImage(int32_t line, int32_t image);

	/*! \brief Clear attributes, images or content of text. */
	void clear(bool attributes, bool images, bool content);

	/*! \brief Insert text at caret's current position. */
	void insert(const std::wstring& text);

	/*! \brief Get caret's current position. */
	int32_t getCaretOffset() const;

	/*! \brief Get line index of position. */
	int32_t getLineFromOffset(int32_t offset) const;

	/*! \brief Get number of textual lines. */
	int32_t getLineCount() const;

	/*! \brief Get starting offset of line. */
	int32_t getLineOffset(int32_t line) const;

	/*! \brief Get length of line. */
	int32_t getLineLength(int32_t line) const;

	/*! \brief Replace text at a specific line. */
	void setLine(int32_t line, const std::wstring& text);

	/*! \brief Get a single line of text. */
	std::wstring getLine(int32_t line) const;

	/*! \brief Set user object with specific line. */
	void setLineData(int32_t line, Object* data);

	/*! \brief Get user object of specific line. */
	Object* getLineData(int32_t line) const;

	/*! \brief Get selection start offset. */
	int32_t getSelectionStartOffset() const;

	/*! \brief Get selection end offset. */
	int32_t getSelectionStopOffset() const;

	/*! \brief Get selected text. */
	std::wstring getSelectedText() const;

	/*! \brief Scroll text view to a certain line. */
	bool scrollToLine(int32_t line);

	/*! \brief Get current scrolled offset. */
	int32_t getScrollLine() const;

	/*! \brief Ensure line is visible. */
	bool showLine(int32_t line);

	/*! \brief Place caret at a specified offset. */
	void placeCaret(int32_t offset);

	/*! \brief Copy selected text into clipboard. */
	bool copy();

	/*! \brief Paste text from clipboard at caret position. */
	bool paste();

private:
	struct Attribute
	{
		Color4ub textColor;
		Color4ub backColor;
		bool bold;
		bool italic;
		bool underline;

		Attribute()
		:	bold(false)
		,	italic(false)
		,	underline(false)
		{
		}
	};

	struct Line
	{
		int32_t start;
		int32_t stop;
		int32_t image;
		Ref< Object > data;

		Line()
		:	start(0)
		,	stop(0)
		,	image(-1)
		{
		}
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	std::vector< Attribute > m_attributes;
	Ref< Bitmap > m_image;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	std::vector< Line > m_lines;
	std::vector< wchar_t > m_text;
	std::vector< uint16_t > m_meta;
	int32_t m_charWidth;
	int32_t m_caret;
	int32_t m_selectionStart;
	int32_t m_selectionStop;
	int32_t m_lineMargin;
	int32_t m_lineOffsetH;
	int32_t m_widestLineWidth;

#if defined(_DEBUG)
	std::vector< Line > m_linesLastGood;
#endif

	void updateScrollBars();

	void updateWidestLine();

	void deleteCharacters();

	void insertCharacter(wchar_t ch);

	void insertAt(int32_t offset, wchar_t ch);

	void scrollToCaret();

	int32_t getCharacterStops(const std::wstring& text, std::vector< int32_t >& outStops) const;

	Rect getEditRect() const;

	void eventKeyDown(KeyDownEvent* event);

	void eventKey(KeyEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventScroll(ScrollEvent* event);

#if defined(_DEBUG)
	void checkConsistency();
#endif
};

		}
	}
}

#endif	// traktor_ui_custom_RichEdit_H
