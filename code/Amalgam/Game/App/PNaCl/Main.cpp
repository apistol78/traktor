#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/message_loop.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/utility/completion_callback_factory.h>

#include <nacl_io/nacl_io.h>

// Traktor
#include "Amalgam/Game/Impl/Application.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/PNaCl/DelegateInstance.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Xml/XmlDeserializer.h"

#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyFloat.h"

using namespace traktor;

namespace
{

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}
	else
		log::error << L"Unable to open file " << settingsFile.getPathName() << Endl;

	return settings;
}

class BrowserLogTarget : public ILogTarget
{
public:
	BrowserLogTarget(pp::Instance* instance)
	:	m_instance(instance)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		m_instance->LogToConsole(PP_LOGLEVEL_LOG, pp::Var(wstombs(str)));
	}

private:
	pp::Instance* m_instance;
};

class FileIOVolume : public IVolume
{
public:
	pp::Instance* m_instance;
	pp::CompletionCallbackFactory< FileIOVolume > m_callbackFactory;
	pp::URLLoader m_urlLoader;
	pp::URLRequestInfo m_urlRequest;
	pp::CompletionCallback m_callback;
	uint8_t m_buffer[4096];
	std::wstring m_deferredUrl;
	pp::CompletionCallback m_deferredCallback;
	Ref< DynamicMemoryStream > m_stream;
	Signal m_streamFinished;
	Semaphore m_volumeLock;

	FileIOVolume(pp::Instance* instance)
	:	m_instance(instance)
	,	m_callbackFactory(this)
	{
	}

	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL
	{
		return L"Pepper FileIO";
	}

	virtual Ref< File > get(const Path& path) T_OVERRIDE T_FINAL
	{
		return 0;
	}

	virtual int find(const Path& mask, RefArray< File >& out) T_OVERRIDE T_FINAL
	{
		return 0;
	}

	virtual bool modify(const Path& fileName, uint32_t flags) T_OVERRIDE T_FINAL
	{
		return false;
	}

	void OnRead(int32_t result)
	{
		if (result == PP_OK)
		{
			m_streamFinished.set();
			m_volumeLock.release();
		}
		else if (result > 0)
		{
			// Push buffer onto output stream.
			m_stream->write(m_buffer, result);

			// Issue another read.
			m_callback = m_callbackFactory.NewCallback(&FileIOVolume::OnRead);
			m_urlLoader.ReadResponseBody(m_buffer, sizeof(m_buffer), m_callback);
		}
		else
		{
			log::error << L"Unable to read URL (" << result << L")" << Endl;
			m_stream = 0;
			m_streamFinished.set();
			m_volumeLock.release();
		}
	}

	void OnOpen(int32_t result)
	{
		if (result == PP_OK)
		{
			// Initiate first read.
			m_callback = m_callbackFactory.NewCallback(&FileIOVolume::OnRead);
			m_urlLoader.ReadResponseBody(m_buffer, sizeof(m_buffer), m_callback);
		}
		else
		{
			log::error << L"Unable to open URL (" << result << L")" << Endl;
			m_stream = 0;
			m_streamFinished.set();
			m_volumeLock.release();
		}
	}

	void OnDeferredOpen(int32_t result)
	{
		m_urlRequest = pp::URLRequestInfo(m_instance);
		m_urlRequest.SetURL(wstombs(m_deferredUrl));
		m_urlRequest.SetMethod("GET");

		m_callback = m_callbackFactory.NewCallback(&FileIOVolume::OnOpen);

		m_urlLoader = pp::URLLoader(m_instance);
		m_urlLoader.Open(m_urlRequest, m_callback);
	}

	virtual Ref< IStream > open(const Path& fileName, uint32_t mode) T_OVERRIDE T_FINAL
	{
		m_volumeLock.wait();

		// Create output stream.
		m_stream = new DynamicMemoryStream();

		// Queue "open then read" on main thread.
		m_deferredUrl = fileName.getPathNameNoVolume();
		m_deferredCallback = m_callbackFactory.NewCallback(&FileIOVolume::OnDeferredOpen);
		pp::MessageLoop::GetForMainThread().PostWork(m_deferredCallback);

		// Wait until file been downloaded into memory.
		m_streamFinished.wait();

		// Re-wind stream; need to look like it's a fresh stream.
		if (m_stream)
			m_stream->seek(IStream::SeekSet, 0);

		return m_stream;
	}

	virtual bool exist(const Path& fileName) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool remove(const Path& fileName) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool makeDirectory(const Path& directory) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool removeDirectory(const Path& directory) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual bool setCurrentDirectory(const Path& directory) T_OVERRIDE T_FINAL
	{
		return false;
	}

	virtual Path getCurrentDirectory() const T_OVERRIDE T_FINAL
	{
		return Path();
	}
};

}

class AmalgamInstance : public DelegateInstance
{
public:
	explicit AmalgamInstance(PP_Instance instance)
	:	DelegateInstance(instance)
	,	m_applicationThread(0)
	{
		nacl_io_init_ppapi(instance, pp::Module::Get()->get_browser_interface());

		log::info   .setGlobalTarget(new BrowserLogTarget(this));
		log::warning.setGlobalTarget(new BrowserLogTarget(this));
		log::error  .setGlobalTarget(new BrowserLogTarget(this));
		log::debug  .setGlobalTarget(new BrowserLogTarget(this));

		FileSystem::getInstance().mount(L"fileio", new FileIOVolume(this));
	}

	virtual ~AmalgamInstance()
	{
		destroy();

		FileSystem::getInstance().umount(L"fileio");

		log::info   .setGlobalTarget(0);
		log::warning.setGlobalTarget(0);
		log::error  .setGlobalTarget(0);
		log::debug  .setGlobalTarget(0);
	}

	virtual void DidChangeView(const pp::View& view) T_OVERRIDE T_FINAL
	{
		DelegateInstance::DidChangeView(view);
	}

	virtual void HandleMessage(const pp::Var& var_message) T_OVERRIDE T_FINAL
	{
		if (!var_message.is_string())
			return;

		std::string message = var_message.AsString();
		if (message == "create")
			create();
		else if (message == "destroy")
			destroy();

		DelegateInstance::HandleMessage(var_message);
	}

private:
	Ref< amalgam::Application > m_application;
	Thread* m_applicationThread;

	bool create()
	{
		destroy();
		T_FATAL_ASSERT (!m_applicationThread);

		m_applicationThread = ThreadManager::getInstance().create(makeFunctor(this, &AmalgamInstance::applicationThread), L"Application");
		if (!m_applicationThread)
		{
			log::error << L"Unable to create application; failed to create thread" << Endl;
			return false;
		}

		m_applicationThread->start();
		return true;
	}

	void destroy()
	{
		if (m_applicationThread)
		{
			m_applicationThread->stop();
			ThreadManager::getInstance().destroy(m_applicationThread);
			m_applicationThread = 0;
		}
	}

	void applicationThread()
	{
		Ref< PropertyGroup > defaultSettings = loadSettings(L"fileio:Application.config");
		if (!defaultSettings)
		{
			log::error << L"Unable to create application; failed to load default settings" << Endl;
			return;
		}

		Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
		T_FATAL_ASSERT (settings);

		log::info << L"Application settings loaded successfully" << Endl;

		Ref< amalgam::Application > application = new amalgam::Application();
		if (!application->create(
			defaultSettings,
			settings,
			(void*)this,
			0
		))
		{
			log::error << L"Unable to create application; failed to initialize systems" << Endl;
			return;
		}

		while (!m_applicationThread->stopped() && application->update())
			;

		application->destroy();
	}
};

class AmalgamModule : public pp::Module
{
public:
	AmalgamModule()
	:	pp::Module()
	{
	}
	
	virtual ~AmalgamModule()
	{
	}

	virtual pp::Instance* CreateInstance(PP_Instance instance) T_OVERRIDE T_FINAL
	{
		return new AmalgamInstance(instance);
	}
};

namespace pp
{

Module* CreateModule()
{
	return new AmalgamModule();
}

}
