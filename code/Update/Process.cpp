#include "Core/Io/IStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/UrlConnection.h"
#include "Update/AvailableDialog.h"
#include "Update/Bundle.h"
#include "Update/IPostAction.h"
#include "Update/Resource.h"
#include "Update/Process.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

#if defined(T_STATIC)
#	include "Update/PostLaunch.h"
#endif

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
	uint8_t m_buffer[4096];
	bool m_result;
	std::set< Path > m_receivedFiles;

	BundleDownloader()
	:	m_result(true)
	{
	}

	void execute(const Bundle* bundle, const net::Url& bundleUrl)
	{
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		const RefArray< Item >& bundleItems = bundle->getItems();

		setSteps(1000);

		// Calculate total size of update bundle.
		uint64_t totalSize = 0;
		for (RefArray< Item >::const_iterator i = bundleItems.begin(); i != bundleItems.end(); ++i)
		{
			const Resource* resource = dynamic_type_cast< const Resource* >(*i);
			if (resource)
				totalSize += resource->getSize();
		}

		// Download each resource in update bundle.
		uint64_t progressSize = 0;
		for (RefArray< Item >::const_iterator i = bundleItems.begin(); i != bundleItems.end(); ++i)
		{
			if (currentThread->stopped())
			{
				m_result = false;
				return;
			}

			const Resource* resource = dynamic_type_cast< const Resource* >(*i);
			if (resource)
			{
				net::Url bundleItemUrl = resource->getUrl();
				if (!bundleItemUrl.valid())
				{
					log::error << L"Invalid URL in bundle resource" << Endl;
					m_result = false;
					break;
				}

				Path filePath = resource->getTargetPath();

				uint32_t progress = uint32_t((progressSize * 1000) / totalSize);
				notify(progress, filePath.getFileName());

				// Ensure all intermediate directories exist.
				FileSystem::getInstance().makeAllDirectories(filePath.getPathOnly());

				// Skip downloading if resource already exists and have same checksum
				// as in bundle.
				if (FileSystem::getInstance().exist(filePath))
				{
					Ref< IStream > targetStream = FileSystem::getInstance().open(filePath, File::FmRead);
					if (targetStream)
					{
						MD5 targetMD5;
						targetMD5.begin();

						int32_t nread;
						while ((nread = targetStream->read(m_buffer, sizeof(m_buffer))) > 0)
							targetMD5.feed(m_buffer, nread);

						targetMD5.end();

						targetStream->close();

						if (targetMD5 == resource->getMD5())
						{
							log::info << L"Item \"" << filePath.getPathName() << L"\" already up-to-date; skipping" << Endl;
							progressSize += resource->getSize();
							continue;
						}
					}
				}

				Ref< net::UrlConnection > connection = net::UrlConnection::open(bundleItemUrl);
				if (!connection)
				{
					log::error << L"Unable to connect to bundle resource \"" << bundleItemUrl.getString() << L"\"" << Endl;
					m_result = false;
					break;
				}

				Ref< IStream > sourceStream = connection->getStream();
				if (!sourceStream)
				{
					log::error << L"Unable to open stream to resource" << Endl;
					m_result = false;
					break;
				}

				Ref< IStream > targetStream = FileSystem::getInstance().open(filePath.getPathName() + L".updated", File::FmWrite);
				if (!targetStream)
				{
					log::error << L"Unable to create file for updated resource" << Endl;
					m_result = false;
					break;
				}

				MD5 checksum;
				checksum.begin();

				for (;;)
				{
					if (currentThread->stopped())
					{
						m_result = false;
						return;
					}

					uint32_t progress = uint32_t((progressSize * 1000) / totalSize);
					notify(progress, filePath.getFileName());

					int32_t nread = sourceStream->read(m_buffer, sizeof(m_buffer));
					if (nread < 0)
					{
						log::error << L"Unable to download resource" << Endl;
						m_result = false;
						break;
					}

					if (nread == 0)
						break;

					int32_t nwritten = targetStream->write(m_buffer, nread);
					if (nwritten != nread)
					{
						log::error << L"Unable to write updated resource" << Endl;
						m_result = false;
						break;
					}

					checksum.feed(m_buffer, nread);

					progressSize += nread;
				}

				checksum.end();

				targetStream->close();
				sourceStream->close();

				if (checksum != resource->getMD5())
				{
					log::error << L"Resource \"" << filePath.getPathName() << L"\" corrupt; incorrect checksum" << Endl;
					m_result = false;
					break;
				}
				
				m_receivedFiles.insert(filePath);
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.update.Process", Process, Object)

Process::CheckResult Process::check(const net::Url& bundleUrl)
{
#if defined(T_STATIC)
	T_FORCE_LINK_REF(PostLaunch);
#endif

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
	
#if !defined(__APPLE__)
	Path installedBundle = L"Installed.bundle";
#else
	Path installedBundle = L"$(BUNDLE_PATH)/Contents/Resources/Installed.bundle";
#endif

	// Check if downloaded bundle is same as we already have installed.
	Ref< IStream > installedBundleStream = FileSystem::getInstance().open(installedBundle, File::FmRead);
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
		log::error << L"Unable to create download thread" << Endl;
		return CrFailed;
	}

	ui::custom::BackgroundWorkerDialog progressDialog;
	if (progressDialog.create(0, L"Downloading", L"Downloading update...", ui::Dialog::WsDefaultFixed | ui::custom::BackgroundWorkerDialog::WsAbortButton))
	{
		progressDialog.execute(threadDownload, bundleDownloader);
		progressDialog.destroy();
	}

	threadDownload->stop();
	ThreadManager::getInstance().destroy(threadDownload);

	if (!bundleDownloader->m_result)
	{
		log::warning << L"Download failed; assuming aborted" << Endl;
		return CrAborted;
	}

#if defined(_WIN32)

	// Update installed bundle descriptor.
	installedBundleStream = FileSystem::getInstance().open(installedBundle.getPathName() + L".updated", File::FmWrite);
	if (installedBundleStream)
	{
		if (!xml::XmlSerializer(installedBundleStream).writeObject(bundle))
			log::error << L"Unable to update \"Installed.bundle.updated\"; xml serialization failed" << Endl;
		installedBundleStream->close();
		installedBundleStream = 0;
	}
	else
		log::error << L"Unable to update \"Installed.bundle.updated\"; failed to create file" << Endl;

	// Extract post binary and launch it.
	Ref< IStream > streamPost = FileSystem::getInstance().open(c_postUpdateFileName, File::FmWrite);
	if (streamPost)
	{
		streamPost->write(c_ResourceTraktor_Update_Post, sizeof(c_ResourceTraktor_Update_Post));
		streamPost->close();

		// Create and acquire named mutex; this is used by post binary
		// to wait until this process has been terminated before
		// replacing files.

		Mutex terminationMutex(c_guidParentTerminate);
		if (!terminationMutex.wait())
		{
			log::error << L"Unable to create post synchronization named mutex" << Endl;
			return CrFailed;
		}

		Ref< IProcess > process = OS::getInstance().execute(
			c_postUpdateFileName,
			L"Installed.bundle",
			L"",
			0,
			false,
			false,
			true
		);
		if (!process)
		{
			log::error << L"Unable to spawn post update process" << Endl;
			return CrFailed;
		}
	}
	
#else

	// Rename all downloaded files.
	for (std::set< Path >::const_iterator i = bundleDownloader->m_receivedFiles.begin(); i != bundleDownloader->m_receivedFiles.end(); ++i)
	{
		const Path& targetPath = *i;
		if (!FileSystem::getInstance().move(
			targetPath,
			targetPath.getPathName() + L".updated",
			true
		))
		{
			log::error << L"Failed to rename updated \"" << targetPath.getOriginal() << L"\"; installation might be broken" << Endl;
			return CrFailed;
		}
	}

	// Execute post actions.
	const RefArray< IPostAction >& postActions = bundle->getPostActions();
	for (RefArray< IPostAction >::const_iterator i = postActions.begin(); i != postActions.end(); ++i)
	{
		if (!(*i)->execute())
			log::error << L"Failed to perform post update action; installation might be broken" << Endl;
	}

	// Update installed bundle descriptor.
	installedBundleStream = FileSystem::getInstance().open(installedBundle, File::FmWrite);
	if (installedBundleStream)
	{
		if (!xml::XmlSerializer(installedBundleStream).writeObject(bundle))
			log::error << L"Unable to update \"Installed.bundle\"; xml serialization failed" << Endl;
		installedBundleStream->close();
		installedBundleStream = 0;
	}
	else
		log::error << L"Unable to update \"Installed.bundle\"; failed to create file" << Endl;

#endif

	return CrTerminate;
}

	}
}
