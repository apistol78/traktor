/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_xml_Text_H
#define traktor_xml_Text_H

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
	namespace xml
	{

/*! \brief XML Text.
 * \ingroup XML
 */
class T_DLLCLASS Text : public Node
{
	T_RTTI_CLASS;

public:
	Text(const std::wstring& text);
	
	virtual std::wstring getValue() const T_OVERRIDE T_FINAL;

	virtual void setValue(const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual void write(OutputStream& os) const T_OVERRIDE T_FINAL;

	Ref< Text > clone() const;

protected:
	virtual Ref< Node > cloneUntyped() const T_OVERRIDE T_FINAL;

private:
	std::wstring m_text;
};
	
	}
}

#endif	// traktor_xml_Text_H
