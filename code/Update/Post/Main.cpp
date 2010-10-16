#include "Core/Io/FileSystem.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Update/Bundle.h"
#include "Update/PostLaunch.h"
#include "Update/Resource.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

const Guid c_guidParentTerminate(L"{3EDFEF6C-3728-47fd-B784-131D0FD9AB6A}");

bool traverseRenameUpdated(const Path& path)
{
	// Move updated files into place.
	RefArray< File > files;
	FileSystem::getInstance().find(path.getPathName() + L"/*.*", files);

	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if ((*i)->isDirectory())
		{
			Path directoryPath = (*i)->getPath();
			if (
				directoryPath.getFileName() != L"." &&
				directoryPath.getFileName() != L".."
			)
			{
				if (!traverseRenameUpdated(directoryPath))
					return false;
			}
		}
		else
		{
			Path filePath = (*i)->getPath();
			if (compareIgnoreCase< std::wstring >(filePath.getExtension(), L"updated") == 0)
			{
				std::wstring targetFilePath = filePath.getPathName();
				targetFilePath = targetFilePath.substr(0, targetFilePath.length() - 8);

				bool result = true;
				for (int32_t tries = 0; tries < 10; ++tries)
				{
					result = FileSystem::getInstance().move(
						targetFilePath,
						filePath,
						true
					);
					if (result)
						break;

					ThreadManager::getInstance().getCurrentThread()->sleep(1000);
				}
				if (!result)
					return false;
			}
		}
	}

	return true;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	T_FORCE_LINK_REF(update::Resource);
	T_FORCE_LINK_REF(update::PostLaunch);

	// Wait until calling process has been terminated.
	Mutex postMutex(c_guidParentTerminate);
	if (!postMutex.wait())
		return 1;

	// Rename all files ending with ".updated".
	if (!traverseRenameUpdated(L"."))
		return 2;

	// Perform post update actions.
	if (cmdLine.getCount() > 0)
	{
		Ref< IStream > bundleStream = FileSystem::getInstance().open(cmdLine.getString(0), File::FmRead);
		if (bundleStream)
		{
			Ref< update::Bundle > bundle = xml::XmlDeserializer(bundleStream).readObject< update::Bundle >();
			if (bundle)
			{
				const RefArray< update::IPostAction >& postActions = bundle->getPostActions();
				for (RefArray< update::IPostAction >::const_iterator i = postActions.begin(); i != postActions.end(); ++i)
					(*i)->execute();
			}
			else
				return 3;
		}
		else
			return 4;
	}

	return 0;
}
