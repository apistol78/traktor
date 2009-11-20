#include <cmath>
#include "Model/Vertex.h"

namespace traktor
{
	namespace model
	{

Vertex::Vertex()
:	m_position(c_InvalidIndex)
,	m_color(c_InvalidIndex)
,	m_normal(c_InvalidIndex)
,	m_tangent(c_InvalidIndex)
,	m_binormal(c_InvalidIndex)
,	m_texCoord(c_InvalidIndex)
{
}

Vertex::Vertex(uint32_t position)
:	m_position(position)
,	m_color(c_InvalidIndex)
,	m_normal(c_InvalidIndex)
,	m_tangent(c_InvalidIndex)
,	m_binormal(c_InvalidIndex)
,	m_texCoord(c_InvalidIndex)
{
}

void Vertex::setPosition(uint32_t position)
{
	m_position = position;
}

uint32_t Vertex::getPosition() const
{
	return m_position;
}

void Vertex::setColor(uint32_t color)
{
	m_color = color;
}

uint32_t Vertex::getColor() const
{
	return m_color;
}

void Vertex::setNormal(uint32_t normal)
{
	m_normal = normal;
}

uint32_t Vertex::getNormal() const
{
	return m_normal;
}

void Vertex::setTangent(uint32_t tangent)
{
	m_tangent = tangent;
}

uint32_t Vertex::getTangent() const
{
	return m_tangent;
}

void Vertex::setBinormal(uint32_t binormal)
{
	m_binormal = binormal;
}

uint32_t Vertex::getBinormal() const
{
	return m_binormal;
}

void Vertex::setTexCoord(uint32_t texCoord)
{
	m_texCoord = texCoord;
}

uint32_t Vertex::getTexCoord() const
{
	return m_texCoord;
}

void Vertex::clearBoneInfluences()
{
	m_boneInfluences.resize(0);
}

void Vertex::setBoneInfluence(uint32_t boneIndex, float influence)
{
	while (boneIndex >= uint32_t(m_boneInfluences.size()))
		m_boneInfluences.push_back(0.0f);
	m_boneInfluences[boneIndex] = influence;
}

float Vertex::getBoneInfluence(uint32_t boneIndex) const
{
	if (boneIndex < 0 || boneIndex >= uint32_t(m_boneInfluences.size()))
		return 0.0f;

	return m_boneInfluences[boneIndex];
}

bool Vertex::operator == (const Vertex& r) const
{
	if (m_position != r.m_position)
		return false;
	if (m_color != r.m_color)
		return false;
	if (m_normal != r.m_normal)
		return false;
	if (m_tangent != r.m_tangent)
		return false;
	if (m_binormal != r.m_binormal)
		return false;
	if (m_texCoord != r.m_texCoord)
		return false;
	if (m_boneInfluences.size() != r.m_boneInfluences.size())
		return false;
	for (size_t i = 0; i < m_boneInfluences.size(); ++i)
		if (std::fabs(m_boneInfluences[i] - r.m_boneInfluences[i]) > 0.0001f)
			return false;
	return true;
}

	}
}
