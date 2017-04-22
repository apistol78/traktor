#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/FilePipelinePutStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.FilePipelineCache", FilePipelineCache, IPipelineCache)

FilePipelineCache::FilePipelineCache()
:	m_accessRead(true)
,	m_accessWrite(true)
{
}

bool FilePipelineCache::create(const PropertyGroup* settings)
{
	m_accessRead = settings->getProperty< bool >(L"Pipeline.FileCache.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.FileCache.Write", true);	
	m_path = settings->getProperty< std::wstring >(L"Pipeline.FileCache.Path");
	
	if (!FileSystem::getInstance().makeAllDirectories(m_path))
		return false;
	
	return true;
}

void FilePipelineCache::destroy()
{
}

Ref< IStream > FilePipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessRead)
		return 0;

	StringOutputStream ss;
	ss << m_path << L"/" << guid.format() << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";
	
	Ref< IStream > fileStream = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (!fileStream)
		return 0;

	return new BufferedStream(fileStream);
}

Ref< IStream > FilePipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessWrite)
		return 0;

	StringOutputStream ss;
	ss << m_path << L"/" << guid.format() << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";
	
	Ref< IStream > fileStream = FileSystem::getInstance().open(ss.str() + L"~", File::FmWrite);
	if (!fileStream)
		return 0;

	return new FilePipelinePutStream(
		new BufferedStream(fileStream),
		ss.str()
	);
}

	}
}
