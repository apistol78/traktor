#include "Core/RefArray.h"
#include "Core/Functor/Functor.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
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

XML::~XML()
{
	if (m_job)
	{
		ActionObject* self = getAsObject(m_context);
		T_ASSERT (self);

		m_context->removeFrameListener(self);

		m_job->stop();
		m_job->wait(5000);
		m_job = 0;
	}
}

bool XML::load(const std::wstring& url_)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	if (m_job)
		return false;

	m_job = JobManager::getInstance().add(makeFunctor< XML, std::wstring >(this, &XML::jobLoad, url_));
	if (!m_job)
		return false;

	self->setMember("onFrame", ActionValue(createNativeFunction(m_context, this, &XML::onFrame)));
	m_context->addFrameListener(self);

	return true;
}

void XML::onFrame(CallArgs& ca)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	if (m_job->wait(0))
	{
		m_job = 0;

		m_context->removeFrameListener(self);

		ActionValue memberOnLoad;
		if (self->getMember("onLoad", memberOnLoad))
		{
			Ref< ActionFunction > onLoad = memberOnLoad.getObject< ActionFunction >();
			if (onLoad)
				onLoad->call(self, ActionValueArray());
		}
	}
}

void XML::jobLoad(std::wstring url_)
{
	net::Url url(url_);
	if (!url.valid() || m_job->stopped())
		return;

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection || m_job->stopped())
		return;

	IStream* stream = connection->getStream();
	if (!stream || m_job->stopped())
		return;

	RefArray< XMLNode > elementStack;
	elementStack.push_back(this);

	xml::XmlPullParser parser(stream);
	while (!m_job->stopped())
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
				return;

			Ref< XMLNode > element = elementStack.back();
			elementStack.pop_back();

			if (elementStack.empty())
				return;

			elementStack.back()->appendChild(element);
		}
		else if (eventType == xml::XmlPullParser::EtText)
		{
			if (elementStack.empty())
				return;

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
}

	}
}
