/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderModuleResolve.h"

#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderModule.h"

namespace traktor::render
{
namespace
{

template < typename T >
void pushUnique(AlignedVector< T >& inoutArr, const T& value)
{
	auto it = std::find(inoutArr.begin(), inoutArr.end(), value);
	if (it == inoutArr.end())
		inoutArr.push_back(value);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderModuleResolve", ShaderModuleResolve, Object)

ShaderModuleResolve::ShaderModuleResolve(const fn_reader_t& readObject)
	: m_readObject(readObject)
{
}

Ref< ShaderModule > ShaderModuleResolve::resolve(const ShaderGraph* shaderGraph) const
{
	AlignedVector< Guid > ids;
	for (auto script : shaderGraph->findNodesOf< Script >())
		for (const Guid& incl : script->getIncludes())
			if (!collectModulesInDependencyOrder(incl, ids))
				return nullptr;

	StringOutputStream text;
	SmallMap< std::wstring, SamplerState > samplers;
	SmallMap< std::wstring, StructDeclaration > structs;

	for (const Guid& id : ids)
	{
		Ref< const ShaderModule > shaderModule = dynamic_type_cast< const ShaderModule* >(m_readObject(id).second);
		if (!shaderModule)
			return nullptr;

		const std::wstring unprocessedText = shaderModule->escape([&](const Guid& g) -> std::wstring {
			return g.format();
		});

		std::wstring processedText;
		std::set< std::wstring > usings;
		if (!m_preprocessor.evaluate(unprocessedText, processedText, usings))
			return nullptr;

		text << processedText << Endl;
		samplers.insert(shaderModule->getSamplers().begin(), shaderModule->getSamplers().end());

		for (const Guid& structId : shaderModule->getStructDeclarations())
		{
			const named_decl_t n = m_readObject(structId);

			Ref< const StructDeclaration > structDecl = dynamic_type_cast< const StructDeclaration* >(n.second);
			if (!structDecl)
				return nullptr;

			structs.insert(n.first, *structDecl);
		}
	}

	return new ShaderModule(text.str(), samplers, structs);
}

bool ShaderModuleResolve::collectModulesInDependencyOrder(const Guid& id, AlignedVector< Guid >& inoutIds) const
{
	if (!id.isValid() || id.isNull())
		return false;

	auto it = std::find(inoutIds.begin(), inoutIds.end(), id);
	if (it != inoutIds.end())
		return true;

	Ref< const ShaderModule > shaderModule = dynamic_type_cast< const ShaderModule* >(m_readObject(id).second);
	if (!shaderModule)
		return false;

	const std::wstring unprocessedText = shaderModule->escape([&](const Guid& g) -> std::wstring {
		return g.format();
	});

	std::wstring text;
	std::set< std::wstring > usings;
	if (!m_preprocessor.evaluate(unprocessedText, text, usings))
	{
		log::error << L"Shader pipeline failed; unable to preprocess module " << id.format() << L"." << Endl;
		return false;
	}

	for (const auto& u : usings)
	{
		const Guid usingId(u);
		if (!collectModulesInDependencyOrder(usingId, inoutIds))
			return false;
	}

	for (const auto& u : usings)
	{
		const Guid usingId(u);
		pushUnique(inoutIds, usingId);
	}

	pushUnique(inoutIds, id);
	return true;
}

}
