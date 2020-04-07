#pragma once

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

/*! XML Pull parser.
 * \ingroup XML
 */
class T_DLLCLASS XmlPullParser : public Object
{
	T_RTTI_CLASS;

public:
	enum class EventType
	{
		Invalid,
		StartDocument,
		StartElement,
		Text,
		EndElement,
		EndDocument
	};

	typedef std::pair< std::wstring, std::wstring > Attribute;
	typedef std::vector< Attribute > Attributes;

	struct Event
	{
		EventType type;
		std::wstring value;
		Attributes attr;

		Event()
		:	type(EventType::Invalid)
		{
		}
	};

	XmlPullParser(IStream* stream, const std::wstring& name = L"");

	virtual ~XmlPullParser();

	EventType next();

	void push();

	const Event& getEvent() const;

private:
	XmlPullParserImpl* m_impl;	/**< Parser implementation. */
	Event m_event;				/**< Current event. */
	int32_t m_pushed;			/**< If current event is being pushed. */
};

	}
}

