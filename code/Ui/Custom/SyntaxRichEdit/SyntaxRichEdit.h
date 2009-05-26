#ifndef traktor_ui_custom_SyntaxRichEdit_H
#define traktor_ui_custom_SyntaxRichEdit_H

#include "Ui/RichEdit.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class SyntaxLanguage;

/*! \brief RichEdit control with automatic syntax highlighting.
 * \ingroup UIC
 */
class T_DLLCLASS SyntaxRichEdit : public RichEdit
{
	T_RTTI_CLASS(SyntaxRichEdit)

public:
	SyntaxRichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", int style = WsClientBorder);

	void setLanguage(SyntaxLanguage* language);

	void setErrorHighlight(int line);

private:
	Ref< SyntaxLanguage > m_language;
	int m_attributeDefault;
	int m_attributeString;
	int m_attributeNumber;
	int m_attributeComment;
	int m_attributeKeyword;
	int m_attributeError;

	void updateLanguage(int fromLine, int toLine);

	void eventChange(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxRichEdit_H
