#include "Core/Io/IStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/UrlConnection.h"
#include "Update/AvailableDialog.h"
#include "Update/Bundle.h"
#include "Update/Resource.h"
#include "Update/Process.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

// Resources
#if defined(_WIN32)
#	include "Resources/Traktor.Update.Post.h"
#endif

namespace traktor
{
	namespace update
	{
		namespace
		{

const Guid c_guidParentTerminate(L"{3EDFEF6C-3728-47fd-B784-131D0FD9AB6A}");
const wchar_t c_postUpdateFileName[] = L"Traktor.Update.Post.exe";

struct BundleDownloader : public ui::custom::BackgroundWorkerStatus
{
	void execute(const Bundle* bundle, const net::Url& bundleUrl)
	{
		const RefArray< Item >& bundleItems = bundle->getItems();
		int32_t itemIndex = 0;

		setSteps(int32_t(bundleItems.size()));

		for (RefArray< Item >::const_iterator i = bundleItems.begin(); i != bundleItems.end(); ++i, ++itemIndex)
		{
			const Resource* resource = dynamic_type_cast< const Resource* >(*i);
			if (resource)
			{
				net::Url bundleItemUrl = resource->getUrl();
				if (!bundleItemUrl.valid())
					continue;

				std::wstring fileName = bundleItemUrl.getFile();

				notify(itemIndex, fileName);

				// Skip downloading resource if it already exists and have same checksum.
				if (FileSystem::getInstance().exist(fileName))
				{
					Ref< IStream > targetStream = FileSystem::getInstance().open(fileName, File::FmRead);
					if (targetStream)
					{
						MD5 targetMD5;
						targetMD5.begin();

						uint8_t buffer[4096];
						int32_t nread;
						while ((nread = targetStream->read(buffer, sizeof(buffer))) > 0)
							targetMD5.feed(buffer, nread);

						targetMD5.end();

						targetStream->close();

						if (targetMD5 == resource->getMD5())
						{
							log::info << L"Item \"" << bundleItemUrl.getFile() << L"\" already up-to-date; skipping" << Endl;
							continue;
						}
						else
						{
							log::info << targetMD5.format() << Endl;
						}
					}
				}

				Ref< net::UrlConnection > connection = net::UrlConnection::open(bundleItemUrl);
				if (!connection)
					continue;

				Ref< IStream > sourceStream = connection->getStream();
				if (!sourceStream)
					continue;

				Ref< IStream > targetStream = FileSystem::getInstance().open(fileName + L".updated", File::FmWrite);
				if (!targetStream)
					continue;

				StreamCopy(targetStream, sourceStream).execute();

				targetStream->close();
				sourceStream->close();
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.update.Process", Process, Object)

Process::CheckResult Process::check(const net::Url& bundleUrl)
{
	Ref< net::UrlConnection > connection = net::UrlConnection::open(bundleUrl);
	if (!connection)
	{
		log::info << L"Unable to connect to \"" << bundleUrl.getString() << L"\"; assuming up-to-date" << Endl;
		return CrUpToDate;
	}

	Ref< IStream > bundleStream = connection->getStream();
	if (!bundleStream)
	{
		log::error << L"Unable to download \"" << bundleUrl.getString() << L"\"; unable to open stream" << Endl;
		return CrFailed;
	}

	Ref< Bundle > bundle = xml::XmlDeserializer(bundleStream).readObject< Bundle >();
	if (!bundle)
	{
		log::error << L"Unable to download \"" << bundleUrl.getString() << L"\"; bundle corrupt" << Endl;
		return CrFailed;
	}

	bundleStream = 0;
	connection = 0;

	// Check if downloaded bundle is same as we already have installed.
	Ref< IStream > installedBundleStream = FileSystem::getInstance().open(L"Installed.bundle", File::FmRead);
	if (installedBundleStream)
	{
		Ref< Bundle > installedBundle = xml::XmlDeserializer(installedBundleStream).readObject< Bundle >();
		if (installedBundle)
		{
			if (DeepHash(installedBundle) == DeepHash(bundle))
			{
				log::info << L"Installed bundle already up-to-date; skipping" << Endl;
				return CrUpToDate;
			}
		}
		installedBundleStream->close();
		installedBundleStream = 0;
	}
	
	// Show update notice.
	AvailableDialog availDialog;
	if (!availDialog.create(bundle))
	{
		log::error << L"Unable to create \"update available\" dialog" << Endl;
		return CrFailed;
	}

	int32_t result = availDialog.showModal();
	availDialog.destroy();

	if (result != ui::DrOk)
	{
		log::warning << L"Update aborted by user" << Endl;
		return CrAborted;
	}

	// Download bundle items.
	Ref< BundleDownloader > bundleDownloader = new BundleDownloader();

	Thread* threadDownload = ThreadManager::getInstance().create(
		makeFunctor< BundleDownloader, const Bundle*, const net::Url& >(bundleDownloader, &BundleDownloader::execute, bundle, bundleUrl),
		L"Download thread"
	);
	if (!threadDownload || !threadDownload->start())
	{
		log::error << L"Unable to issue download thread" << Endl;
		return CrFailed;
	}

	ui::custom::BackgroundWorkerDialog progressDialog;
	if (progressDialog.create(0, L"Downloading", L"Downloading update..."))
	{
		progressDialog.execute(threadDownload, bundleDownloader);
		progressDialog.destroy();
	}

	threadDownload->stop();
	ThreadManager::getInstance().destroy(threadDownload);

	// Update installed bundle descriptor.
	installedBundleStream = FileSystem::getInstance().open(L"Installed.bundle.updated", File::FmWrite);
	if (installedBundleStream)
	{
		if (!xml::XmlSerializer(installedBundleStream).writeObject(bundle))
			log::error << L"Unable to update \"Installed.bundle.updated\"; xml serialization failed" << Endl;
		installedBundleStream->close();
		installedBundleStream = 0;
	}
	else
		log::error << L"Unable to update \"Installed.bundle.updated\"; failed to create file" << Endl;

#if defined(_WIN32)

	// Extract post binary and launch it.
	Ref< IStream > streamPost = FileSystem::getInstance().open(c_postUpdateFileName, File::FmWrite);
	if (streamPost)
	{
		streamPost->write(c_ResourceTraktor_Update_Post, sizeof(c_ResourceTraktor_Update_Post));
		streamPost->close();

		// Create and acquire named mutex; this is used by post binary
		// to wait until this process has been terminated before
		// replacing files.

		Mutex postMutex(c_guidParentTerminate);
		if (!postMutex.wait())
		{
			log::error << L"Unable to create post synchronization named mutex" << Endl;
			return CrFailed;
		}

		Ref< IProcess > process = OS::getInstance().execute(c_postUpdateFileName, L"", L"", 0, false, true, true);
		if (!process)
		{
			log::error << L"Unable to spawn post update process" << Endl;
			return CrFailed;
		}
	}
	
#else

	// We've successfully downloaded entire bundle; replace existing local copies with updated items.
	// Move updated files into place.
	RefArray< File > files;
	FileSystem::getInstance().find(L"*.updated", files);

	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		std::wstring filePath = (*i)->getPath().getPathName();
		if (filePath.length() > 8)
		{
			if (!FileSystem::getInstance().move(
				filePath.substr(0, filePath.length() - 8),
				filePath,
				true
			))
				log::error << L"Failed to rename updated item; installation might be broken" << Endl;
		}
	}	

#endif

	return CrTerminate;
}

	}
}
