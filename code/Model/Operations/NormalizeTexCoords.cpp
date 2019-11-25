#include <cmath>
#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Model/Operations/NormalizeTexCoords.h"

namespace traktor
{
	namespace model
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
    for (auto& vertex : model.getVertices())
    {
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
    }

	return true;
}

void NormalizeTexCoords::serialize(ISerializer& s)
{
    s >> Member< uint32_t >(L"channel", m_channel);
    s >> Member< float >(L"marginU", m_marginU);
    s >> Member< float >(L"marginV", m_marginV);
    s >> Member< float >(L"stepU", m_stepU);
    s >> Member< float >(L"stepV", m_stepV);
}

	}
}
