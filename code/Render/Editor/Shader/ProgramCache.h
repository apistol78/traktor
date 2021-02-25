#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/StringMap.h"
#include "Core/Io/Path.h"

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class IProgramCompiler;
class ShaderGraph;

/*!
 * \ingroup Render
 */
class ProgramCache : public Object
{
	T_RTTI_CLASS;

public:
	ProgramCache(
		const Path& cachePath,
		const IProgramCompiler* compiler,
		const PropertyGroup* settings
	);

	Ref< ProgramResource > get(const ShaderGraph* shaderGraph, const std::wstring& name);

private:
	Path m_cachePath;
	Ref< const IProgramCompiler > m_compiler;
	Ref< const PropertyGroup > m_settings;
	uint32_t m_settingsHash;
	StringMap< Ref< ProgramResource >, std::wstring > m_programs;
};

	}
}