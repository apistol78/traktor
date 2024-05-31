/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Resource/IResourceManager.h"
#include "Spark/Acc/AccShapeResources.h"

namespace traktor::spark
{
	namespace
	{

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"));
const resource::Id< render::Shader > c_idShaderLine(Guid(L"{8CAA6CFE-014A-3F4A-80C1-BD8A21B9A8C4}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"));

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AccShapeResources", AccShapeResources, Object)

bool AccShapeResources::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_idShaderSolid, m_shaderSolid))
		return false;
	if (!resourceManager->bind(c_idShaderTextured, m_shaderTextured))
		return false;
	if (!resourceManager->bind(c_idShaderLine, m_shaderLine))
		return false;
	if (!resourceManager->bind(c_idShaderIncrementMask, m_shaderIncrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderDecrementMask, m_shaderDecrementMask))
		return false;

	return true;
}

void AccShapeResources::destroy()
{
	m_shaderSolid.clear();
	m_shaderTextured.clear();
	m_shaderLine.clear();
	m_shaderIncrementMask.clear();
	m_shaderDecrementMask.clear();
}

}
