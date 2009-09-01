#ifndef traktor_xml_Document_H
#define traktor_xml_Document_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace xml
	{

class Node;
class Element;

/*! \brief XML DOM Document.
 * \ingroup XML
 */
class T_DLLCLASS Document : public Object
{
	T_RTTI_CLASS(Document)

public:
	bool loadFromFile(const Path& fileName);
	
	bool loadFromStream(Stream* stream);
	
	bool loadFromText(const std::wstring& text);
	
	bool saveAsFile(const Path& fileName);

	bool saveIntoStream(Stream* stream);

	int get(const std::wstring& path, RefArray< Element >& elements);
	
	Element* getSingle(const std::wstring& path);
	
	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;
	
private:
	Ref< Element > m_docElement;
};
	
	}
}

#endif	// traktor_xml_Document_H
