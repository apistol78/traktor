#pragma once

#include "Xml/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::xml
{

/*! XML Text.
 * \ingroup XML
 */
class T_DLLCLASS Text : public Node
{
	T_RTTI_CLASS;

public:
	Text(const std::wstring& text);

	virtual std::wstring getValue() const override final;

	virtual void setValue(const std::wstring& value) override final;

	virtual void write(OutputStream& os) const override final;

	Ref< Text > clone() const;

protected:
	virtual Ref< Node > cloneUntyped() const override final;

private:
	std::wstring m_text;
};

}
