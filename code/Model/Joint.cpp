#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Model/Joint.h"
#include "Model/Types.h"

namespace traktor
{
	namespace model
	{

Joint::Joint()
:	m_parent(c_InvalidIndex)
,	m_transform(Transform::identity())
,	m_length(1.0f)
{
}

Joint::Joint(const std::wstring& name)
:	m_parent(c_InvalidIndex)
,	m_name(name)
,	m_transform(Transform::identity())
,	m_length(1.0f)
{
}

Joint::Joint(uint32_t parent, const std::wstring& name, const Transform& transform, float length)
:	m_parent(parent)
,	m_name(name)
,	m_transform(transform)
,	m_length(length)
{
}

void Joint::setParent(uint32_t parent)
{
	m_parent = parent;
}

uint32_t Joint::getParent() const
{
	return m_parent;
}

void Joint::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Joint::getName() const
{
	return m_name;
}

void Joint::setTransform(const Transform& transform)
{
	m_transform = transform;
}

const Transform& Joint::getTransform() const
{
	return m_transform;
}

void Joint::setLength(float length)
{
	m_length = length;
}

float Joint::getLength() const
{
	return m_length;
}

void Joint::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"parent", m_parent);
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< float >(L"length", m_length);
}

bool Joint::operator == (const Joint& rh) const
{
	return
		m_parent == rh.m_parent &&
		m_name == rh.m_name &&
		m_transform == rh.m_transform &&
		m_length == rh.m_length;
}

	}
}
