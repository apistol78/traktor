/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Misc/Murmur3.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"

namespace traktor::model
{
	namespace
	{

uint32_t polygonHash(const Polygon& p)
{
	Murmur3 hash;
	hash.begin();
	hash.feed(p.getMaterial());
	hash.feed(p.getNormal());
	hash.feed(p.getSmoothGroup());
	for (const auto vertex : p.getVertices())
		hash.feed(vertex);
	hash.end();
	return hash.get();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CleanDuplicates", CleanDuplicates, IModelOperation)

CleanDuplicates::CleanDuplicates(float positionDistance)
:	m_positionDistance(positionDistance)
{
}

bool CleanDuplicates::apply(Model& model) const
{
	SmallSet< uint32_t > addedPolygonHashes;
	Model cleaned;
	uint32_t id;

	for (const auto& material : model.getMaterials())
		cleaned.addMaterial(material);

	for (const auto& channel : model.getTexCoordChannels())
		cleaned.addUniqueTexCoordChannel(channel);

	for (const auto& polygon : model.getPolygons())
	{
		Polygon cleanedPolygon;
		cleanedPolygon.setMaterial(polygon.getMaterial());

		id = polygon.getNormal();
		if (id != c_InvalidIndex)
			cleanedPolygon.setNormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

		for (auto vertexId : polygon.getVertices())
		{
			if (vertexId == c_InvalidIndex)
				continue;

			const Vertex& vertex = model.getVertex(vertexId);
			Vertex cleanedVertex;

			id = vertex.getPosition();
			if (id != c_InvalidIndex)
				cleanedVertex.setPosition(cleaned.addUniquePosition(model.getPosition(id), m_positionDistance));

			id = vertex.getColor();
			if (id != c_InvalidIndex)
				cleanedVertex.setColor(cleaned.addUniqueColor(model.getColor(id)));

			id = vertex.getNormal();
			if (id != c_InvalidIndex)
				cleanedVertex.setNormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			id = vertex.getTangent();
			if (id != c_InvalidIndex)
				cleanedVertex.setTangent(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			id = vertex.getBinormal();
			if (id != c_InvalidIndex)
				cleanedVertex.setBinormal(cleaned.addUniqueNormal(model.getNormal(id).normalized()));

			const uint32_t texCoordCount = vertex.getTexCoordCount();
			for (uint32_t k = 0; k < texCoordCount; ++k)
			{
				id = vertex.getTexCoord(k);
				if (id != c_InvalidIndex)
					cleanedVertex.setTexCoord(k, cleaned.addUniqueTexCoord(model.getTexCoord(id)));
			}

			const uint32_t influenceCount = vertex.getJointInfluenceCount();
			for (uint32_t k = 0; k < influenceCount; ++k)
			{
				const float influence = vertex.getJointInfluence(k);
				if (influence > FUZZY_EPSILON)
					cleanedVertex.setJointInfluence(k, influence);
			}

			cleanedPolygon.addVertex(cleaned.addUniqueVertex(cleanedVertex));
		}

		const uint32_t hash = polygonHash(cleanedPolygon);
		if (addedPolygonHashes.insert(hash))
			cleaned.addPolygon(cleanedPolygon);
	}

	for (const auto& joint : model.getJoints())
		cleaned.addJoint(joint);

	for (const auto& animation : model.getAnimations())
		cleaned.addAnimation(animation);

	model = cleaned;
	return true;
}

}
