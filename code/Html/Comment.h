#ifndef traktor_html_Comment_H
#define traktor_html_Comment_H

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

/*! \brief HTML DOM Comment.
 * \ingroup HTML
 */
class T_DLLCLASS Comment : public Node
{
	T_RTTI_CLASS;

public:
	Comment(const std::wstring& text);

	virtual std::wstring getName() const;

	virtual std::wstring getValue() const;

	virtual void writeHtml(IStream* stream);

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_html_Comment_H
