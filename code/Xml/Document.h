/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_xml_Document_H
#define traktor_xml_Document_H

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	bool loadFromFile(const Path& fileName);
	
	bool loadFromStream(IStream* stream);
	
	bool loadFromText(const std::wstring& text);
	
	bool saveAsFile(const Path& fileName);

	bool saveIntoStream(IStream* stream);

	int32_t get(const std::wstring& path, RefArray< Element >& outElements) const;
	
	Element* getSingle(const std::wstring& path) const;
	
	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;

	Ref< Document > clone() const;
	
private:
	Ref< Element > m_docElement;
};
	
	}
}

#endif	// traktor_xml_Document_H
