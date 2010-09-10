#include "Core/Io/FileSystem.h"
#include "Core/Thread/Mutex.h"

using namespace traktor;

namespace
{

const Guid c_guidParentTerminate(L"{3EDFEF6C-3728-47fd-B784-131D0FD9AB6A}");

}

int main(int argc, const char** argv)
{
	// Wait until calling process has been terminated.
	Mutex postMutex(c_guidParentTerminate);
	if (!postMutex.wait())
		return 1;

	// Move updated files into place.
	RefArray< File > files;
	FileSystem::getInstance().find(L"*.updated", files);

	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		std::wstring filePath = (*i)->getPath().getPathName();
		if (filePath.length() > 8)
		{
			FileSystem::getInstance().move(
				filePath.substr(0, filePath.length() - 8),
				filePath,
				true
			);
		}
	}

	return 0;
}
