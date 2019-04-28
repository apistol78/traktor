#include "Core/Guid.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Url.h"
#include "Net/Http/HttpRequest.h"
#include "Net/Http/HttpServer.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

using namespace traktor;

namespace
{

std::wstring getChildElementValue(xml::Element* elm, const std::wstring& name, const std::wstring& defaultValue)
{
    auto child = elm->getChildElementByName(name);
    return (child != nullptr) ? child->getValue() : defaultValue;
}

class StoreHttpRequestListener : public net::HttpServer::IRequestListener
{
public:
	StoreHttpRequestListener(const Path& dataPath)
	:	m_dataPath(dataPath)
	{
	}

	virtual int32_t httpClientRequest(
		net::HttpServer* server,
		const net::HttpRequest* request,
		IStream* clientStream,
		OutputStream& os,
		Ref< traktor::IStream >& outStream,
		bool& outCache,
		std::wstring& inoutSession
	) override final
	{
		std::wstring resource = net::Url::decodeString(request->getResource());
        std::map< std::wstring, std::wstring > params;

        // Extract url parameters.
        auto p = resource.find(L'?');
        if (p != resource.npos)
        {
            for (auto kv : StringSplit< std::wstring >(resource.substr(p + 1), L"&"))
            {
                auto p2 = kv.find(L'=');
                if (p2 != kv.npos)
                {
                    auto k = kv.substr(0, p2);
                    auto v = kv.substr(p2 + 1);
                    params.insert(std::make_pair(k, v));
                }
            }
            resource = resource.substr(0, p);
        }

        if (request->getMethod() == net::HttpRequest::MtGet)
        {
            log::info << L"GET " << resource << Endl;
            if (resource == L"/catalogue")
            {
                auto category = params[L"category"];
                if (!Guid(category).isValid())
                    return 404;

                os << L"<?xml version=\"1.0\"?>" << Endl;
                os << L"<catalogue>" << Endl;

                // Find all package directories in category.
                RefArray< File > files;
                FileSystem::getInstance().find(m_dataPath.getPathName() + L"/" + category + L"/*.*", files);
                for (auto file : files)
                {
                    const auto p = file->getPath();
                    if (!file->isDirectory() || p.getFileName() == L"." || p.getFileName() == L"..")
                        continue;

                    if (!Guid(p.getFileName()).isValid())
                        continue;

                    Path manifestPath = FileSystem::getInstance().getAbsolutePath(p.getPathName() + L"/Manifest.xml");

                    Path manifestPathRel;
                    FileSystem::getInstance().getRelativePath(
                        manifestPath,
                        m_dataPath,
                        manifestPathRel
                    );

                    os << L"\t<package id=\"" << p.getFileName() << L"\"/>" << Endl;
                }
                
                os << L"</catalogue>" << Endl;
                return 200;
            }
            else
            {
                if ((outStream = FileSystem::getInstance().open(m_dataPath.getPathName() + resource, File::FmRead)) != nullptr)
                    return 200;
            }
        }
        else if (request->getMethod() == net::HttpRequest::MtPut)
        {
            log::info << L"PUT " << resource << Endl;

            Path filePath(m_dataPath.getPathName() + resource);

            if (!FileSystem::getInstance().makeAllDirectories(filePath.getPathOnly()))
            {
                log::error << L"Failed to create path \"" << filePath.getPathOnly() << L"\"." << Endl;
                return 404;
            }

            Ref< IStream > fileStream = FileSystem::getInstance().open(filePath, File::FmWrite);
            if (!fileStream)
            {
                log::error << L"Failed to create file \"" << filePath.getPathName() << L"\"." << Endl;
                return 404;
            }

            if (!StreamCopy(fileStream, clientStream).execute(
                clientStream->available()
            ))
            {
                log::error << L"Failed to receive content into file \"" << filePath.getPathName() << L"\"." << Endl;
                return 404;
            }
            
            fileStream->close();
            fileStream = nullptr;
            return 200;
        }

        log::error << L"Unhandled method." << Endl;        
        return 404;
	}

private:
	Path m_dataPath;
};

}

int main(int argc, const char** argv)
{
   	log::info << L"Traktor.Store.Server.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
 
    const Path dataPath = FileSystem::getInstance().getAbsolutePath(L"packages");

    if (!FileSystem::getInstance().makeAllDirectories(dataPath))
    {
        log::error << L"Data path do not exist nor cannot it be created." << Endl;
        return 1;
    }

	net::Network::initialize();

	Ref< net::HttpServer > httpServer = new net::HttpServer();
	if (!httpServer->create(net::SocketAddressIPv4(8118)))
    {
        log::error << L"Unable to create HTTP server." << Endl;
        return 1;
    }

	httpServer->setRequestListener(new StoreHttpRequestListener(
        dataPath
    ));

    log::info << L"Waiting for client(s)..." << Endl;
    for (;;)
    {
        httpServer->update(200);
    }

    safeDestroy(httpServer);

    net::Network::finalize();
    return 0;
}