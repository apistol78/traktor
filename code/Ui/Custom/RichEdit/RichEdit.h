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

	virtual void setText(const std::wstring& text);
	
	virtual std::wstring getText() const;

	int32_t addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline);

	void setAttribute(int32_t start, int32_t length, int32_t attribute);

	int32_t addImage(Bitmap* image, uint32_t imageCount);

	void setImage(int32_t line, int32_t image);

	void clear(bool attributes, bool images, bool content);

	void insert(const std::wstring& text);

	int32_t getCaretOffset() const;

	int32_t getLineFromOffset(int32_t offset) const;

	int32_t getLineCount() const;

	int32_t getLineOffset(int32_t line) const;

	int32_t getLineLength(int32_t line) const;

	void setLine(int32_t line, const std::wstring& text);

	std::wstring getLine(int32_t line) const;

	int32_t getSelectionStartOffset() const;

	int32_t getSelectionStopOffset() const;

	std::wstring getSelectedText() const;

	bool scrollToLine(int32_t line);

	bool showLine(int32_t line);

	void placeCaret(int32_t offset);

	bool redo();

	bool undo();

	bool copy();

	bool paste();

	void addChangeEventHandler(EventHandler* eventHandler);

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
	int32_t m_caret;
	int32_t m_selectionStart;
	int32_t m_selectionStop;

#if defined(_DEBUG)
	std::vector< Line > m_linesLastGood;
#endif

	void updateScrollBars();

	void deleteCharacters();

	void insertCharacter(wchar_t ch);

	void scrollToCaret();

	int32_t getCharacterStops(const std::wstring& text, std::vector< int32_t >& outStops) const;

	Rect getEditRect() const;

	void eventKeyDown(Event* event);

	void eventKey(Event* event);

	void eventButtonDown(Event* event);

	void eventMouseWheel(Event* event);

	void eventPaint(Event* event);

	void eventSize(Event* event);

	void eventScroll(Event* event);

#if defined(_DEBUG)
	void checkConsistency();
#endif
};

		}
	}
}

#endif	// traktor_ui_custom_RichEdit_H
