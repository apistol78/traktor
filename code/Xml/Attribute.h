#ifndef traktor_xml_Attribute_H
#define traktor_xml_Attribute_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

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

/*! \brief XML Attribute.
 * \ingroup XML
 */
class T_DLLCLASS Attribute : public Object
{
	T_RTTI_CLASS(Attribute)

public:
	Attribute(const std::wstring& name, const std::wstring& value = L"");

	virtual const std::wstring& getName() const;

	virtual void setName(const std::wstring& name);

	virtual const std::wstring& getValue() const;

	virtual void setValue(const std::wstring& value);

	Ref< Attribute > getPrevious() const;
	
	Ref< Attribute > getNext() const;

private:
	friend class Element;

	std::wstring m_name;
	std::wstring m_value;
	Ref< Attribute > m_previous;
	Ref< Attribute > m_next;
};

	}
}

#endif	// traktor_xml_Attribute_H
