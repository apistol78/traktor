#ifndef traktor_xml_Element_H
#define traktor_xml_Element_H

#include <vector>
#include "Core/RefArray.h"
#include "Xml/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class OutputStream;

	namespace xml
	{
	
class Attribute;

/*! \brief XML Element.
 * \ingroup XML
 */
class T_DLLCLASS Element : public Node
{
	T_RTTI_CLASS;

public:
	Element(const std::wstring& name);
	
	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual void setName(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual std::wstring getValue() const T_OVERRIDE T_FINAL;

	virtual void setValue(const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual void write(OutputStream& os) const T_OVERRIDE T_FINAL;

	int32_t get(const std::wstring& path, RefArray< Element >& outElements) const;
	
	Element* getSingle(const std::wstring& path) const;

	std::wstring getPath() const;

	bool match(const std::wstring& condition);
	
	bool hasAttribute(const std::wstring& name) const;
	
	void setAttribute(const std::wstring& name, const std::wstring& value);
	
	Attribute* getFirstAttribute() const;
	
	Attribute* getLastAttribute() const;
	
	Attribute* getAttribute(const std::wstring& name) const;

	/*! \brief Get attribute by name, will always return an attribute.
	 * This method will always return an attribute, if named attribute
	 * doesn't exist a temporary one will be created with given value.
	 *
	 * \note
	 * In case of an temporary attribute it will not get linked
	 * with other attributes in the element thus getPrevious and getNext
	 * will always return null.
	 * 
	 * \param name Name of attribute.
	 * \return Attribute object.
	 */
	Ref< Attribute > getAttribute(const std::wstring& name, const std::wstring& defaultValue) const;
	
	Element* getChildElementByName(const std::wstring& name);

	Ref< Element > clone() const;

protected:
	virtual Ref< Node > cloneUntyped() const T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	Ref< Attribute > m_firstAttribute;
	Ref< Attribute > m_lastAttribute;
};
	
	}
}

#endif	// traktor_xml_Element_H
