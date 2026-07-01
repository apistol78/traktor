/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Aabb2.h"
#include "Model/Model.h"
#include "Model/Operations/NormalizeTexCoords.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.NormalizeTexCoords", NormalizeTexCoords, IModelOperation)

NormalizeTexCoords::NormalizeTexCoords(
	uint32_t channel,
	float marginU,
	float marginV,
	float stepU,
	float stepV
)
:   m_channel(channel)
,   m_marginU(marginU)
,   m_marginV(marginV)
,	m_stepU(stepU)
,	m_stepV(stepV)
{
}

bool NormalizeTexCoords::apply(Model& model) const
{
    Aabb2 bbox;

    // Calculate bounding box of texcoords in given channel.
    for (const auto& vertex : model.getVertices())
    {
        uint32_t texCoord = vertex.getTexCoord(m_channel);
        if (texCoord != c_InvalidIndex)
            bbox.contain(model.getTexCoord(texCoord));
    }
    if (bbox.empty())
        return true;

    // Reiterate and normalize texcoords in same channel.
    for (uint32_t i = 0; i < model.getVertexCount(); ++i)
    {
        Vertex vertex = model.getVertex(i);
        uint32_t texCoord = vertex.getTexCoord(m_channel);
        if (texCoord == c_InvalidIndex)
            continue;

        Vector2 uv = model.getTexCoord(texCoord);

        //uv = (uv - bbox.mn) / (bbox.mx - bbox.mn);
        //uv = (uv + Vector2(m_marginU, m_marginV)) * Vector2(1.0f - m_marginU * 2.0f, 1.0f - m_marginV * 2.0f);

		uv.x = std::floor(uv.x / m_stepU) * m_stepU;
		uv.y = std::floor(uv.y / m_stepV) * m_stepV;

        texCoord = model.addUniqueTexCoord(uv);
        vertex.setTexCoord(m_channel, texCoord);

        model.setVertex(i, vertex);
    }

	return true;
}

}
