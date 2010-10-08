#include <cmath>
#include "Core/Misc/Adler32.h"
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
{
}

Vertex::Vertex(uint32_t position)
:	m_position(position)
,	m_color(c_InvalidIndex)
,	m_normal(c_InvalidIndex)
,	m_tangent(c_InvalidIndex)
,	m_binormal(c_InvalidIndex)
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

void Vertex::clearTexCoords()
{
	m_texCoords.resize(0);
}

void Vertex::setTexCoord(uint32_t channel, uint32_t texCoord)
{
	while (channel >= uint32_t(m_texCoords.size()))
		m_texCoords.push_back(c_InvalidIndex);
	m_texCoords[channel] = texCoord;
}

uint32_t Vertex::getTexCoord(uint32_t channel) const
{
	if (channel < uint32_t(m_texCoords.size()))
		return m_texCoords[channel];
	else
		return c_InvalidIndex;
}

uint32_t Vertex::getTexCoordCount() const
{
	return m_texCoords.size();
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
	if (boneIndex < uint32_t(m_boneInfluences.size()))
		return m_boneInfluences[boneIndex];
	else
		return 0.0f;
}

uint32_t Vertex::getHash() const
{
	Adler32 adler;

	adler.begin();
	adler.feed(&m_position, sizeof(m_position));
	adler.feed(&m_color, sizeof(m_color));
	adler.feed(&m_normal, sizeof(m_normal));
	adler.feed(&m_tangent, sizeof(m_tangent));
	adler.feed(&m_binormal, sizeof(m_binormal));
	if (!m_texCoords.empty())
		adler.feed(&m_texCoords[0], m_texCoords.size() * sizeof(uint32_t));
	if (!m_boneInfluences.empty())
		adler.feed(&m_boneInfluences[0], m_boneInfluences.size() * sizeof(float));
	adler.end();

	return adler.get();
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
	if (m_texCoords.size() != m_texCoords.size())
		return false;
	if (m_boneInfluences.size() != r.m_boneInfluences.size())
		return false;

	for (size_t i = 0; i < m_texCoords.size(); ++i)
		if (m_texCoords[i] != r.m_texCoords[i])
			return false;

	for (size_t i = 0; i < m_boneInfluences.size(); ++i)
		if (std::fabs(m_boneInfluences[i] - r.m_boneInfluences[i]) > 0.0001f)
			return false;

	return true;
}

	}
}
