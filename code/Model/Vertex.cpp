/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Misc/Murmur3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberStaticVector.h"
#include "Model/Vertex.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Vertex", 0, Vertex, ISerializable)

Vertex::Vertex(uint32_t position)
:	m_position(position)
{
}

Vertex::Vertex(uint32_t position, uint32_t normal)
:	m_position(position)
,	m_normal(normal)
{
}

Vertex::Vertex(uint32_t position, uint32_t normal, uint32_t texCoord)
:	m_position(position)
,	m_normal(normal)
{
	m_texCoords.push_back(texCoord);
}

void Vertex::clearTexCoords()
{
	m_texCoords.clear();
}

void Vertex::setTexCoord(uint32_t channel, uint32_t texCoord)
{
	if (channel < m_texCoords.capacity())
	{
		while (channel >= uint32_t(m_texCoords.size()))
			m_texCoords.push_back(c_InvalidIndex);
		m_texCoords[channel] = texCoord;
	}
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
	return uint32_t(m_texCoords.size());
}

void Vertex::clearJointInfluences()
{
	m_jointInfluences.clear();
}

void Vertex::setJointInfluence(uint32_t jointIndex, float influence)
{
	while (jointIndex >= uint32_t(m_jointInfluences.size()))
		m_jointInfluences.push_back(0.0f);
	m_jointInfluences[jointIndex] = influence;
}

float Vertex::getJointInfluence(uint32_t jointIndex) const
{
	if (jointIndex < uint32_t(m_jointInfluences.size()))
		return m_jointInfluences[jointIndex];
	else
		return 0.0f;
}

uint32_t Vertex::getJointInfluenceCount() const
{
	return uint32_t(m_jointInfluences.size());
}

uint32_t Vertex::getHash() const
{
	Murmur3 adler;

	adler.begin();
	adler.feed(&m_position, sizeof(m_position));
	adler.feed(&m_color, sizeof(m_color));
	adler.feed(&m_normal, sizeof(m_normal));
	adler.feed(&m_tangent, sizeof(m_tangent));
	adler.feed(&m_binormal, sizeof(m_binormal));
	if (!m_texCoords.empty())
		adler.feed(&m_texCoords[0], m_texCoords.size() * sizeof(uint32_t));
	if (!m_jointInfluences.empty())
		adler.feed(&m_jointInfluences[0], m_jointInfluences.size() * sizeof(float));
	adler.end();

	return adler.get();
}

void Vertex::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"position", m_position);
	s >> Member< uint32_t >(L"color", m_color);
	s >> Member< uint32_t >(L"normal", m_normal);
	s >> Member< uint32_t >(L"tangent", m_tangent);
	s >> Member< uint32_t >(L"binormal", m_binormal);
	s >> MemberStaticVector< uint32_t, 4 >(L"texCoords", m_texCoords);
	s >> MemberAlignedVector< float >(L"jointInfluences", m_jointInfluences);
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
	if (m_texCoords.size() != r.m_texCoords.size())
		return false;
	if (m_jointInfluences.size() != r.m_jointInfluences.size())
		return false;

	for (size_t i = 0; i < m_texCoords.size(); ++i)
		if (m_texCoords[i] != r.m_texCoords[i])
			return false;

	for (size_t i = 0; i < m_jointInfluences.size(); ++i)
		if (std::fabs(m_jointInfluences[i] - r.m_jointInfluences[i]) > 0.0001f)
			return false;

	return true;
}

	}
}
