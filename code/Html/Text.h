#ifndef traktor_html_Text_H
#define traktor_html_Text_H

#include "Html/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

/*! \brief HTML DOM Text.
 * \ingroup HTML
 */
class T_DLLCLASS Text : public Node
{
	T_RTTI_CLASS(Text)

public:
	Text(const std::wstring& text);

	virtual std::wstring getName() const;

	virtual std::wstring getValue() const;

	virtual void writeHtml(Stream* stream);

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_html_Text_H
