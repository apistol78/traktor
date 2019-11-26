#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
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
	const IProgramCompiler* compiler
)
:	m_cachePath(cachePath)
,	m_compiler(compiler)
{
}

Ref< ProgramResource > ProgramCache::get(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings
)
{
	Ref< IStream > f;

	// Calculate hash of shader graph.
	uint32_t shaderGraphHash = ShaderGraphHash::calculate(shaderGraph);

	// Calculate hash of settings.
	uint32_t settingsHash = DeepHash(settings).get();

	// Generate file name of cached program.
	Path cachedFileName = m_cachePath.getPathName() + L"/" + type_name(m_compiler) + L"/" + toString(shaderGraphHash) + L"_" + toString(settingsHash) + L".bin";

	// Try to read pre-compiled resource from cache.
	if ((f = FileSystem::getInstance().open(cachedFileName, File::FmRead)) != nullptr)
	{
		Ref< ProgramResource > cachedResource = BinarySerializer(f).readObject< ProgramResource >();
		if (cachedResource)
			return cachedResource;
		safeClose(f);
	}

	// No cached pre-compiled resource found; need to compile resource.
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, settings, L"", true, false, nullptr);
	if (!resource)
		return nullptr;

	// Store resource in cache.
	FileSystem::getInstance().makeAllDirectories(cachedFileName.getPathOnly());
	if ((f = FileSystem::getInstance().open(cachedFileName, File::FmWrite)) != nullptr)
	{
		BinarySerializer(f).writeObject(resource);
		safeClose(f);
	}

	return resource;
}

	}
}
