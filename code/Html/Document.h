/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_html_Document_H
#define traktor_html_Document_H

#include <string>
#include "Core/Object.h"
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IEncoding;

	namespace html
	{

class Element;

/*! \brief HTML DOM Document.
 * \ingroup HTML
 */
class T_DLLCLASS Document : public Object
{
	T_RTTI_CLASS;

public:
	Document(bool parseComments = true);

	bool loadFromFile(const std::wstring& filename, const IEncoding* encoding);
	
	bool loadFromStream(IStream* stream, const IEncoding* encoding);
	
	bool loadFromText(const std::wstring& text);
	
	bool saveAsFile(const std::wstring& filename);
	
	bool saveIntoStream(IStream* stream);
	
	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;

	void toString(OutputStream& os) const;
	
private:
	bool m_parseComments;
	Ref< Element > m_docElement;
};

	}
}

#endif	// traktor_html_Document_H
