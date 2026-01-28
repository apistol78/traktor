/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ParameterLinker.h"

#include "Core/Log/Log.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/StructDeclaration.h"
#include "Render/Editor/Shader/ParameterDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ParameterLinker", ParameterLinker, Object)

ParameterLinker::ParameterLinker(const fn_reader_t& declarationReader)
	: m_declarationReader(declarationReader)
{
}

bool ParameterLinker::resolve(ShaderGraph* shaderGraph) const
{
	for (auto parameter : shaderGraph->findNodesOf< Parameter >())
	{
		if (parameter->m_parameterDeclaration.isNotNull())
		{
			const named_decl_t nd = m_declarationReader(parameter->m_parameterDeclaration);
			if (!nd.second)
			{
				log::error << L"Unable to read parameter declaration " << parameter->m_parameterDeclaration.format() << L"." << Endl;
				return false;
			}

			ParameterDeclaration decl = *mandatory_non_null_type_cast< const ParameterDeclaration* >(nd.second);
			if (decl.getLength() > 1 && decl.getParameterType() >= ParameterType::Matrix)
			{
				log::error << L"Invalid parameter declaration " << parameter->m_parameterDeclaration.format() << L"; only scalar types can be an array." << Endl;
				return false;
			}

			if (decl.m_structDeclaration.isNotNull())
			{
				const named_decl_t snd = m_declarationReader(decl.m_structDeclaration);
				if (!snd.second)
				{
					log::warning << L"Unable to read struct declaration " << decl.m_structDeclaration.format() << L"." << Endl;
					return false;
				}

				decl.m_structType = snd.first;
				decl.m_declaration = *mandatory_non_null_type_cast< const StructDeclaration* >(snd.second);
			}

			parameter->m_parameterName = nd.first;
			parameter->m_declaration = decl;
		}

		// If we reach here then the linkage of this parameter has been successful.
		parameter->m_linked = true;
	}

	return true;
}

}
