#include "Core/Io/FileSystem.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/ProgramCache.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCache", ProgramCache, Object)

ProgramCache::ProgramCache(
	const Path& cachePath,
	const IProgramCompiler* compiler,
	const PropertyGroup* settings
)
:	m_cachePath(cachePath)
,	m_compiler(compiler)
,	m_settings(settings)
{
	m_settingsHash = DeepHash(settings).get();
}

Ref< ProgramResource > ProgramCache::get(const ShaderGraph* shaderGraph, const std::wstring& name)
{
	Ref< IStream > f;

	// Calculate hash of shader graph.
	uint32_t shaderGraphHash = ShaderGraphHash(false).calculate(shaderGraph);

	// Generate file name of cached program.
	Path cachedFileName = m_cachePath.getPathName() + L"/" + type_name(m_compiler) + L"/" + toString(shaderGraphHash) + L"_" + toString(m_settingsHash) + L".bin";

	// Try to read pre-compiled resource from cache.
	if ((f = FileSystem::getInstance().open(cachedFileName, File::FmRead)) != nullptr)
	{
		BufferedStream bufferedStream(f);
		Ref< ProgramResource > cachedResource = BinarySerializer(&bufferedStream).readObject< ProgramResource >();
		bufferedStream.close();
		if (cachedResource)
			return cachedResource;
	}

	// No cached pre-compiled resource found; need to compile resource.
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, m_settings, name, nullptr);
	if (!resource)
		return nullptr;

	// Store resource in cache.
	FileSystem::getInstance().makeAllDirectories(cachedFileName.getPathOnly());
	if ((f = FileSystem::getInstance().open(cachedFileName, File::FmWrite)) != nullptr)
	{
		BufferedStream bufferedStream(f);
		BinarySerializer(&bufferedStream).writeObject(resource);
		bufferedStream.close();
	}

	return resource;
}

	}
}
