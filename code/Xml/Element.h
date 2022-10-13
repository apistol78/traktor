#pragma once

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

}

namespace traktor::xml
{

class Attribute;

/*! XML Element.
 * \ingroup XML
 */
class T_DLLCLASS Element : public Node
{
	T_RTTI_CLASS;

public:
	Element(const std::wstring& name);

	virtual std::wstring getName() const override final;

	virtual void setName(const std::wstring& name) override final;

	virtual std::wstring getValue() const override final;

	virtual void setValue(const std::wstring& value) override final;

	virtual void write(OutputStream& os) const override final;

	int32_t get(const std::wstring& path, RefArray< Element >& outElements) const;

	Element* getSingle(const std::wstring& path) const;

	std::wstring getPath() const;

	bool match(const std::wstring& condition);

	bool hasAttribute(const std::wstring& name) const;

	void setAttribute(const std::wstring& name, const std::wstring& value);

	Attribute* getFirstAttribute() const;

	Attribute* getLastAttribute() const;

	Attribute* getAttribute(const std::wstring& name) const;

	/*! Get attribute by name, will always return an attribute.
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
	virtual Ref< Node > cloneUntyped() const override final;

private:
	std::wstring m_name;
	Ref< Attribute > m_firstAttribute;
	Ref< Attribute > m_lastAttribute;
};

}
