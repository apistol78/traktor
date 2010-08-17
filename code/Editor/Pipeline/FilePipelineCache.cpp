#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
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

bool FilePipelineCache::create(const Settings* settings)
{
	m_accessRead = settings->getProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", true);
	m_accessWrite = settings->getProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", true);	
	m_path = settings->getProperty< PropertyString >(L"Pipeline.FileCache.Path");
	
	if (!FileSystem::getInstance().makeAllDirectories(m_path))
		return false;
	
	return true;
}

void FilePipelineCache::destroy()
{
}

Ref< IStream > FilePipelineCache::get(const Guid& guid, uint32_t hash, int32_t version)
{
	if (!m_accessRead)
		return 0;

	StringOutputStream ss;
	ss << m_path << L"/" << guid.format() << L"_" << hash << L"_" << version << L".cache";
	
	return FileSystem::getInstance().open(ss.str(), File::FmRead);
}

Ref< IStream > FilePipelineCache::put(const Guid& guid, uint32_t hash, int32_t version)
{
	if (!m_accessWrite)
		return 0;

	StringOutputStream ss;
	ss << m_path << L"/" << guid.format() << L"_" << hash << L"_" << version << L".cache";
	
	Ref< IStream > file = FileSystem::getInstance().open(ss.str() + L"~", File::FmWrite);
	if (!file)
		return 0;
		
	return new FilePipelinePutStream(file, ss.str());
}

	}
}
