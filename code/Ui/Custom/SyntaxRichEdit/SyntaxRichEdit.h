#ifndef traktor_ui_custom_SyntaxRichEdit_H
#define traktor_ui_custom_SyntaxRichEdit_H

#include <list>
#include "Ui/Custom/RichEdit/RichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxTypes.h"

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
		namespace custom
		{

class SyntaxLanguage;

/*! \brief RichEdit control with automatic syntax highlighting.
 * \ingroup UIC
 */
class T_DLLCLASS SyntaxRichEdit : public RichEdit
{
	T_RTTI_CLASS;

public:
	SyntaxRichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", int style = WsClientBorder);

	void setLanguage(SyntaxLanguage* language);

	void setErrorHighlight(int line);

	void getOutline(std::list< SyntaxOutline >& outOutline) const;

private:
	Ref< SyntaxLanguage > m_language;
	int32_t m_attributeDefault;
	int32_t m_attributeString;
	int32_t m_attributeNumber;
	int32_t m_attributeComment;
	int32_t m_attributeFunction;
	int32_t m_attributeType;
	int32_t m_attributeKeyword;
	int32_t m_attributeSpecial;
	int32_t m_attributeError;

	void updateLanguage(int fromLine, int toLine);

	void eventChange(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxRichEdit_H
