/*
 * TRAKTOR
 * Copyright (c) 2024-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/UniformLinker.h"

#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/StructDeclaration.h"
#include "Render/Editor/Shader/UniformDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformLinker", UniformLinker, Object)

UniformLinker::UniformLinker(const fn_reader_t& declarationReader)
	: m_declarationReader(declarationReader)
{
}

bool UniformLinker::resolve(ShaderGraph* shaderGraph) const
{
	for (auto uniform : shaderGraph->findNodesOf< Uniform >())
	{
		if (uniform->getDeclaration().isNotNull())
		{
			named_decl_t nd = m_declarationReader(uniform->getDeclaration());
			if (!nd.second)
				return false;

			const UniformDeclaration* decl = mandatory_non_null_type_cast< const UniformDeclaration* >(nd.second);

			uniform->setParameterName(nd.first);
			uniform->setParameterType(decl->getParameterType());
			uniform->setFrequency(decl->getFrequency());
		}
	}

	for (auto indexedUniform : shaderGraph->findNodesOf< IndexedUniform >())
	{
		if (indexedUniform->getDeclaration().isNotNull())
		{
			named_decl_t nd = m_declarationReader(indexedUniform->getDeclaration());
			if (!nd.second)
				return false;

			const UniformDeclaration* decl = mandatory_non_null_type_cast< const UniformDeclaration* >(nd.second);

			indexedUniform->setParameterName(nd.first);
			indexedUniform->setParameterType(decl->getParameterType());
			indexedUniform->setFrequency(decl->getFrequency());
			indexedUniform->setLength(decl->getLength());
		}
	}

	for (auto strukt : shaderGraph->findNodesOf< Struct >())
	{
		if (strukt->m_structDeclaration.isNotNull())
		{
			named_decl_t nd = m_declarationReader(strukt->m_structDeclaration);
			if (!nd.second)
				return false;

			strukt->m_declType = nd.first;
			strukt->m_decl = *mandatory_non_null_type_cast< const StructDeclaration* >(nd.second);
		}
	}

	return true;
}

}
