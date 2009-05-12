#ifndef traktor_html_Document_H
#define traktor_html_Document_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Io/Stream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Encoding;

	namespace html
	{

class Element;

class T_DLLCLASS Document : public Object
{
	T_RTTI_CLASS(Document)

public:
	Document(bool parseComments = true);

	bool loadFromFile(const std::wstring& filename, const Encoding* encoding);
	
	bool loadFromStream(Stream* stream, const Encoding* encoding);
	
	bool loadFromText(const std::wstring& text);
	
	bool saveAsFile(const std::wstring& filename);
	
	bool saveIntoStream(Stream* stream);
	
	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;
	
private:
	bool m_parseComments;
	Ref< Element > m_docElement;
};

	}
}

#endif	// traktor_html_Document_H
