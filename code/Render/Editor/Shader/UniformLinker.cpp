/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/UniformDeclaration.h"
#include "Render/Editor/Shader/UniformLinker.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformLinker", UniformLinker, Object)

UniformLinker::UniformLinker(const fn_reader_t& declarationReader)
:	m_declarationReader(declarationReader)
{
}

Ref< ShaderGraph > UniformLinker::resolve(const ShaderGraph* shaderGraph) const
{
	Ref< ShaderGraph > mutableShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	T_FATAL_ASSERT(mutableShaderGraph != nullptr);

	for (auto uniform : mutableShaderGraph->findNodesOf< Uniform >())
	{
		if (uniform->getDeclaration().isNotNull())
		{
			named_decl_t nd = m_declarationReader(uniform->getDeclaration());
			if (!nd.second)
				return nullptr;

			uniform->setParameterName(nd.first);
			uniform->setParameterType(nd.second->getParameterType());
			uniform->setFrequency(nd.second->getFrequency());
		}
	}

	for (auto indexedUniform : mutableShaderGraph->findNodesOf< IndexedUniform >())
	{
		if (indexedUniform->getDeclaration().isNotNull())
		{
			named_decl_t nd = m_declarationReader(indexedUniform->getDeclaration());
			if (!nd.second)
				return nullptr;

			indexedUniform->setParameterName(nd.first);
			indexedUniform->setParameterType(nd.second->getParameterType());
			indexedUniform->setFrequency(nd.second->getFrequency());
			indexedUniform->setLength(nd.second->getLength());
		}
	}

	return mutableShaderGraph;
}

}
