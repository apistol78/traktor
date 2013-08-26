#ifndef traktor_html_Element_H
#define traktor_html_Element_H

#include "Html/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

class Attribute;

/*! \brief HTML DOM Element.
 * \ingroup HTML
 */
class T_DLLCLASS Element : public Node
{
	T_RTTI_CLASS;

public:
	Element(const std::wstring& name);

	virtual std::wstring getName() const;
	
	virtual std::wstring getValue() const;

	bool hasAttribute(const std::wstring& name) const;
	
	void setAttribute(const std::wstring& name, const std::wstring& value);
	
	Attribute* getFirstAttribute() const;
	
	Attribute* getLastAttribute() const;
	
	Attribute* getAttribute(const std::wstring& name) const;

	Element* getPreviousElementSibling() const;
	
	Element* getNextElementSibling() const;
	
	Element* getFirstElementChild() const;
	
	Element* getLastElementChild() const;

	virtual void writeHtml(IStream* stream);

private:
	std::wstring m_name;
	Ref< Attribute > m_firstAttribute;
	Ref< Attribute > m_lastAttribute;
};

	}
}

#endif	// traktor_html_Element_H
