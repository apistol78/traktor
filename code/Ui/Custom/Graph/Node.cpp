/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Event.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/NodeShape.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Node", Node, Object)

Node::Node(const std::wstring& title, const std::wstring& info, const Point& position, NodeShape* shape)
:	m_title(title)
,	m_info(info)
,	m_state(0)
,	m_position(position)
,	m_size(0, 0)
,	m_selected(false)
,	m_shape(shape)
{
}

void Node::setTitle(const std::wstring& title)
{
	T_ASSERT (m_shape);
	m_title = title;
	m_size = m_shape->calculateSize(this);
}

const std::wstring& Node::getTitle() const
{
	return m_title;
}

void Node::setInfo(const std::wstring& info)
{
	T_ASSERT (m_shape);
	m_info = info;
	m_size = m_shape->calculateSize(this);
}

const std::wstring& Node::getInfo() const
{
	return m_info;
}

void Node::setComment(const std::wstring& comment)
{
	T_ASSERT (m_shape);
	m_comment = comment;
	m_size = m_shape->calculateSize(this);
}

const std::wstring& Node::getComment() const
{
	return m_comment;
}

void Node::setImage(IBitmap* image)
{
	T_ASSERT (m_shape);
	if (m_image != image)
	{
		m_image = image;
		m_size = m_shape->calculateSize(this);
	}
}

IBitmap* Node::getImage() const
{
	return m_image;
}

void Node::setState(int32_t state)
{
	T_ASSERT (m_shape);
	if (m_state != state)
	{
		m_state = state;
		m_size = m_shape->calculateSize(this);
	}
}

int32_t Node::getState() const
{
	return m_state;
}

void Node::setPosition(const Point& position)
{
	T_ASSERT (m_shape);
	if (m_position != position)
	{
		m_position = position;
		m_size = m_shape->calculateSize(this);
	}
}

Point Node::getPosition() const
{
	return m_position;
}

void Node::setSelected(bool selected)
{
	T_ASSERT (m_shape);
	if (m_selected != selected)
	{
		m_selected = selected;
		m_size = m_shape->calculateSize(this);
	}
}

bool Node::isSelected() const
{
	return m_selected;
}

Ref< Pin > Node::createInputPin(const std::wstring& name, bool mandatory)
{
	T_ASSERT (m_shape);

	Ref< Pin > pin = new Pin(this, name, Pin::DrInput, mandatory);
	m_inputPins.push_back(pin);

	m_size = m_shape->calculateSize(this);

	return pin;
}

const RefArray< Pin >& Node::getInputPins() const
{
	return m_inputPins;
}

Pin* Node::findInputPin(const std::wstring& name) const
{
	for (RefArray< Pin >::const_iterator i = m_inputPins.begin(); i != m_inputPins.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

Ref< Pin > Node::createOutputPin(const std::wstring& name)
{
	T_ASSERT (m_shape);

	Ref< Pin > pin = new Pin(this, name, Pin::DrOutput, false);
	m_outputPins.push_back(pin);

	m_size = m_shape->calculateSize(this);

	return pin;
}

const RefArray< Pin >& Node::getOutputPins() const
{
	return m_outputPins;
}

Pin* Node::findOutputPin(const std::wstring& name) const
{
	for (RefArray< Pin >::const_iterator i = m_outputPins.begin(); i != m_outputPins.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

bool Node::hit(const Point& p) const
{
	return calculateRect().inside(p);
}

Point Node::getPinPosition(const Pin* pin) const
{
	T_ASSERT (m_shape);
	return m_shape->getPinPosition(this, pin);
}

Pin* Node::getPinAt(const Point& p) const
{
	T_ASSERT (m_shape);
	return m_shape->getPinAt(this, p);
}

void Node::paint(GraphCanvas* canvas, const Size& offset) const
{
	T_ASSERT (m_shape);
	m_shape->paint(this, canvas, offset);
}

Rect Node::calculateRect() const
{
	return Rect(m_position, m_size);
}

		}
	}
}
