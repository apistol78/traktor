#include <android/native_activity.h>
#include "Core/Io/Android/AssetsStream.h"
#include "Core/Io/Android/AssetsVolume.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AssetsVolume", AssetsVolume, IVolume)

AssetsVolume::AssetsVolume(ANativeActivity* activity)
:	m_activity(activity)
{
}

std::wstring AssetsVolume::getDescription() const
{
	return L"Android Assets Volume";
}

Ref< File > AssetsVolume::get(const Path& path)
{
	return 0;
}

int AssetsVolume::find(const Path& mask, RefArray< File >& out)
{
	return 0;
}

bool AssetsVolume::modify(const Path& fileName, uint32_t flags)
{
	return 0;
}

Ref< IStream > AssetsVolume::open(const Path& filename, uint32_t mode)
{
	AAssetManager* assetManager = m_activity->assetManager;
	T_FATAL_ASSERT (assetManager);

	AAsset* assetFile = AAssetManager_open(assetManager, wstombs(filename.getPathNameNoVolume()).c_str(), AASSET_MODE_STREAMING);
	if (!assetFile)
	{
		log::error << L"Unable to open asset \"" << filename.getPathNameNoVolume() << L"\"; AAssetManager_open return null." << Endl; 
		return 0;
	}

	return new AssetsStream(assetFile);
}

bool AssetsVolume::exist(const Path& filename)
{
	return false;
}

bool AssetsVolume::remove(const Path& filename)
{
	return false;
}

bool AssetsVolume::move(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool AssetsVolume::copy(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool AssetsVolume::makeDirectory(const Path& directory)
{
	return false;
}

bool AssetsVolume::removeDirectory(const Path& directory)
{
	return false;
}

bool AssetsVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool AssetsVolume::setCurrentDirectory(const Path& directory)
{
	return true;
}

Path AssetsVolume::getCurrentDirectory() const
{
	return L"";
}

}
