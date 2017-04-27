/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_xml_XmlPullParser_H
#define traktor_xml_XmlPullParser_H

#include <string>
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

class IStream;

	namespace xml
	{

class XmlPullParserImpl;

/*! \brief XML Pull parser.
 * \ingroup XML
 */
class T_DLLCLASS XmlPullParser : public Object
{
	T_RTTI_CLASS;

public:
	enum EventType
	{
		EtInvalid,
		EtStartDocument,
		EtStartElement,
		EtText,
		EtEndElement,
		EtEndDocument
	};

	typedef std::pair< std::wstring, std::wstring > Attribute;
	typedef std::vector< Attribute > Attributes;

	struct Event
	{
		EventType type;
		std::wstring value;
		Attributes attr;

		Event()
		:	type(EtInvalid)
		{
		}
	};

	XmlPullParser(IStream* stream);

	virtual ~XmlPullParser();

	EventType next();

	void push();

	const Event& getEvent() const;

private:
	XmlPullParserImpl* m_impl;	/**< Parser implementation. */
	Event m_event;				/**< Current event. */
	int m_pushed;				/**< If current event is being pushed. */
};

	}
}

#endif	// traktor_xml_XmlPullParser_H
