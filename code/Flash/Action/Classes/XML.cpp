#include "Core/RefArray.h"
#include "Core/Misc/String.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/Classes/XML.h"
#include "Flash/Action/Classes/XMLNode.h"
#include "Net/Url.h"
#include "Net/UrlConnection.h"
#include "Xml/XmlPullParser.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.XML", XML, XMLNode)

XML::XML(ActionContext* context)
:	XMLNode("XML", XMLNode::NtElement, L"", L"")
,	m_context(context)
{
}

bool XML::load(const std::wstring& url_)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	net::Url url(url_);
	if (!url.valid())
		return false;

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection)
		return false;

	IStream* stream = connection->getStream();
	if (!stream)
		return false;

	RefArray< XMLNode > elementStack;
	elementStack.push_back(this);

	xml::XmlPullParser parser(stream);
	for (;;)
	{
		xml::XmlPullParser::EventType eventType = parser.next();

		if (eventType == xml::XmlPullParser::EtEndDocument)
			break;
		else if (eventType == xml::XmlPullParser::EtInvalid)
			break;

		if (eventType == xml::XmlPullParser::EtStartElement)
		{
			Ref< XMLNode > element = new XMLNode(
				"XMLNode",
				XMLNode::NtElement,
				parser.getEvent().value,
				L""
			);

			elementStack.push_back(element);
		}
		else if (eventType == xml::XmlPullParser::EtEndElement)
		{
			if (elementStack.empty())
				return false;

			Ref< XMLNode > element = elementStack.back();
			elementStack.pop_back();

			if (elementStack.empty())
				return false;

			elementStack.back()->appendChild(element);
		}
		else if (eventType == xml::XmlPullParser::EtText)
		{
			if (elementStack.empty())
				return false;

			std::wstring value = trim(parser.getEvent().value);
			if (!value.empty())
			{
				Ref< XMLNode > text = new XMLNode(
					"XMLNode",
					XMLNode::NtText,
					L"",
					value
				);
				elementStack.back()->appendChild(text);
			}
		}
	}

	// Trigger onLoad callback.
	ActionValue memberOnLoad;
	if (self->getMember("onLoad", memberOnLoad))
	{
		Ref< ActionFunction > onLoad = memberOnLoad.getObject< ActionFunction >();
		if (onLoad)
			onLoad->call(self, ActionValueArray());
	}

	return true;
}

	}
}
