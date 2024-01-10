/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <mikktspace.h>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CalculateTangents", CalculateTangents, IModelOperation)

CalculateTangents::CalculateTangents(bool replace)
:	m_replace(replace)
{
}

bool CalculateTangents::apply(Model& model) const
{
	struct UserData
	{
		Model* model;
	} ud;

	ud.model = &model;

	SMikkTSpaceInterface itf = { 0 };
	itf.m_getNumFaces = [](const SMikkTSpaceContext * pContext) -> int {
		UserData* ud = (UserData*)pContext->m_pUserData;
		return (int)ud->model->getPolygonCount();
	};
	itf.m_getNumVerticesOfFace = [](const SMikkTSpaceContext * pContext, const int iFace) -> int {
		UserData* ud = (UserData*)pContext->m_pUserData;
		return (int)ud->model->getPolygon(iFace).getVertexCount();
	};
	itf.m_getPosition = [](const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert) -> void {
		UserData* ud = (UserData*)pContext->m_pUserData;
		const Polygon& polygon = ud->model->getPolygon(iFace);
		const Vector4& position = ud->model->getVertexPosition(polygon.getVertex(iVert));
		fvPosOut[0] = position.x();
		fvPosOut[1] = position.y();
		fvPosOut[2] = position.z();
	};
	itf.m_getNormal = [](const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert) -> void {
		UserData* ud = (UserData*)pContext->m_pUserData;
		const Polygon& polygon = ud->model->getPolygon(iFace);
		const Vertex& vertex = ud->model->getVertex(polygon.getVertex(iVert));
		if (vertex.getNormal() != c_InvalidIndex)
		{
			const Vector4& normal = ud->model->getNormal(vertex.getNormal());
			fvNormOut[0] = normal.x();
			fvNormOut[1] = normal.y();
			fvNormOut[2] = normal.z();
		}
		else
		{
			fvNormOut[0] = 0.0f;
			fvNormOut[1] = 0.0f;
			fvNormOut[2] = 0.0f;
		}
	};
	itf.m_getTexCoord = [](const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert) -> void {
		UserData* ud = (UserData*)pContext->m_pUserData;
		const Polygon& polygon = ud->model->getPolygon(iFace);
		const Vertex& vertex = ud->model->getVertex(polygon.getVertex(iVert));
		if (vertex.getTexCoordCount() > 0 && vertex.getTexCoord(0) != c_InvalidIndex)
		{
			const Vector2& texCoord = ud->model->getTexCoord(vertex.getTexCoord(0));
			fvTexcOut[0] = texCoord.x;
			fvTexcOut[1] = texCoord.y;
		}
		else
		{
			fvTexcOut[0] = 0.0f;
			fvTexcOut[1] = 0.0f;
		}
	};
	itf.m_setTSpace = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT, const tbool bIsOrientationPreserving, const int iFace, const int iVert) -> void {
		UserData* ud = (UserData*)pContext->m_pUserData;
		const Polygon& polygon = ud->model->getPolygon(iFace);

		const float* ft = fvTangent;
		const float* fn = fvBiTangent;

		const Vector4 tangent(ft[0], ft[1], ft[2], 0.0f);
		const Vector4 binormal(fn[0], fn[1], fn[2], 0.0f);

		Vertex vertex = ud->model->getVertex(polygon.getVertex(iVert));
		if (vertex.getNormal() == c_InvalidIndex)
		{
			const Vector4 normal = cross(tangent, binormal).normalized();
			vertex.setNormal(ud->model->addUniqueNormal(normal));
		}
		vertex.setTangent(ud->model->addUniqueNormal(tangent));
		vertex.setBinormal(ud->model->addUniqueNormal(binormal));
		ud->model->setVertex(polygon.getVertex(iVert), vertex);
	};

	SMikkTSpaceContext cx;
	cx.m_pInterface = &itf;
	cx.m_pUserData = &ud;

	genTangSpaceDefault(&cx);
	return true;
}

}
