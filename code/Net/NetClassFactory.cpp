/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Net/IcmpSocket.h"
#include "Net/MulticastUdpSocket.h"
#include "Net/NetClassFactory.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/SocketSet.h"
#include "Net/TcpSocket.h"
#include "Net/UdpSocket.h"
#include "Net/UrlConnection.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Net/Http/HttpResponse.h"
#include "Net/Http/HttpRequest.h"
#include "Net/Http/HttpRequestContent.h"
#include "Net/Http/HttpServer.h"

namespace traktor::net
{
	namespace
	{

Ref< HttpClientResult > net_HttpClient_get_2(HttpClient* self, const Url& url)
{
	return self->get(url);
}

Ref< HttpClientResult > net_HttpClient_get_3(HttpClient* self, const Url& url, const IHttpRequestContent* content)
{
	return self->get(url, content);
}

Ref< HttpClientResult > net_HttpClient_put_2(HttpClient* self, const Url& url)
{
	return self->put(url);
}

Ref< HttpClientResult > net_HttpClient_put_3(HttpClient* self, const Url& url, const IHttpRequestContent* content)
{
	return self->put(url, content);
}

Ref< HttpClientResult > net_HttpClient_post_2(HttpClient* self, const Url& url)
{
	return self->post(url);
}

Ref< HttpClientResult > net_HttpClient_post_3(HttpClient* self, const Url& url, const IHttpRequestContent* content)
{
	return self->post(url, content);
}

int32_t net_HttpRequest_getMethod(HttpRequest* self)
{
	return (int32_t)self->getMethod();
}

bool net_HttpServer_create(HttpServer* self, int32_t port)
{
	return self->create(SocketAddressIPv4(port));
}

class HttpServerListenerDelegate : public HttpServer::IRequestListener
{
public:
	HttpServerListenerDelegate(IRuntimeDelegate* delegateListener)
	:	m_delegateListener(delegateListener)
	{
	}

	virtual int32_t httpClientRequest(
		HttpServer* server,
		const HttpRequest* request,
		IStream* clientStream,
		OutputStream& os,
		Ref< traktor::IStream >& outStream,
		bool& outCache,
		std::wstring& inoutSession
	)
	{
		Any argv[] =
		{
			CastAny< HttpServer* >::set(server),
			CastAny< const HttpRequest* >::set(request),
			CastAny< IStream* >::set(clientStream)
		};
		Any ret = m_delegateListener->call(sizeof_array(argv), argv);
		if (ret.isString())
		{
			os << ret.getWideString();
			return 200;
		}
		else if (ret.isObject< traktor::IStream >())
		{
			outStream = ret.getObjectUnsafe< traktor::IStream >();
			return 200;
		}
		else
			return 404;
	}

private:
	Ref< IRuntimeDelegate > m_delegateListener;
};

void net_HttpServer_setRequestListener(HttpServer* self, Object* listener)
{
	if (HttpServer::IRequestListener* requestListener = dynamic_type_cast< HttpServer::IRequestListener* >(listener))
		self->setRequestListener(requestListener);
	else if (IRuntimeDelegate* delegateListener = dynamic_type_cast< IRuntimeDelegate* >(listener))
		self->setRequestListener(new HttpServerListenerDelegate(delegateListener));
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.NetClassFactory", 0, NetClassFactory, IRuntimeClassFactory)

void NetClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classSocket = new AutoRuntimeClass< Socket >();
	classSocket->addMethod("close", &Socket::close);
	classSocket->addMethod("select", &Socket::select);
	//classSocket->addMethod("send", &Socket::send);
	//classSocket->addMethod("recv", &Socket::recv);
	//classSocket->addMethod("send", &Socket::send);
	//classSocket->addMethod("recv", &Socket::recv);
	registrar->registerClass(classSocket);

	auto classIcmpSocket = new AutoRuntimeClass< IcmpSocket >();
	classIcmpSocket->addConstructor();
	//classIcmpSocket->addMethod("bind", &IcmpSocket::bind);
	//classIcmpSocket->addMethod("bind", &IcmpSocket::bind);
	//classIcmpSocket->addMethod("connect", &IcmpSocket::connect);
	//classIcmpSocket->addMethod("connect", &IcmpSocket::connect);
	//classIcmpSocket->addMethod("sendTo", &IcmpSocket::send);
	//classIcmpSocket->addMethod("recvFrom", &IcmpSocket::recv);
	registrar->registerClass(classIcmpSocket);

	auto classTcpSocket = new AutoRuntimeClass< TcpSocket >();
	classTcpSocket->addConstructor();
	classTcpSocket->addProperty("localAddress", &TcpSocket::getLocalAddress);
	classTcpSocket->addProperty("remoteAddress", &TcpSocket::getRemoteAddress);
	//classTcpSocket->addMethod("bind", &TcpSocket::bind);
	//classTcpSocket->addMethod("bind", &TcpSocket::bind);
	//classTcpSocket->addMethod("connect", &TcpSocket::connect);
	//classTcpSocket->addMethod("connect", &TcpSocket::connect);
	classTcpSocket->addMethod("listen", &TcpSocket::listen);
	classTcpSocket->addMethod("accept", &TcpSocket::accept);
	classTcpSocket->addMethod("setNoDelay", &TcpSocket::setNoDelay);
	registrar->registerClass(classTcpSocket);

	auto classUdpSocket = new AutoRuntimeClass< UdpSocket >();
	classUdpSocket->addConstructor();
	//classUdpSocket->addMethod("bind", &UdpSocket::bind);
	//classUdpSocket->addMethod("bind", &UdpSocket::bind);
	//classUdpSocket->addMethod("connect", &UdpSocket::connect);
	//classUdpSocket->addMethod("connect", &UdpSocket::connect);
	//classUdpSocket->addMethod("sendTo", &UdpSocket::send);
	//classUdpSocket->addMethod("recvFrom", &UdpSocket::recv);
	registrar->registerClass(classUdpSocket);

	auto classMulticastUdpSocket = new AutoRuntimeClass< MulticastUdpSocket >();
	registrar->registerClass(classMulticastUdpSocket);

	auto classSocketAddress = new AutoRuntimeClass< SocketAddress >();
	registrar->registerClass(classSocketAddress);

	auto classSocketAddressIPv4 = new AutoRuntimeClass< SocketAddressIPv4 >();
	classSocketAddressIPv4->addConstructor< uint16_t >();
	classSocketAddressIPv4->addConstructor< const std::wstring&, uint16_t >();
	registrar->registerClass(classSocketAddressIPv4);

	auto classSocketAddressIPv6 = new AutoRuntimeClass< SocketAddressIPv6 >();
	registrar->registerClass(classSocketAddressIPv6);

	auto classSocketSet = new AutoRuntimeClass< SocketSet >();
	classSocketSet->addMethod("add", &SocketSet::add);
	classSocketSet->addMethod("remove", &SocketSet::remove);
	classSocketSet->addMethod("count", &SocketSet::count);
	classSocketSet->addMethod("get", &SocketSet::get);
	classSocketSet->addMethod("contain", &SocketSet::contain);
	registrar->registerClass(classSocketSet);

	auto classUrl = new AutoRuntimeClass< Url >();
	classUrl->addConstructor();
	classUrl->addConstructor< const std::wstring& >();
	classUrl->addProperty("valid", &Url::valid);
	classUrl->addProperty("defaultPort", &Url::getDefaultPort);
	classUrl->addProperty("file", &Url::getFile);
	classUrl->addProperty("host", &Url::getHost);
	classUrl->addProperty("path", &Url::getPath);
	classUrl->addProperty("port", &Url::getPort);
	classUrl->addProperty("protocol", &Url::getProtocol);
	classUrl->addProperty("query", &Url::getQuery);
	classUrl->addProperty("ref", &Url::getRef);
	classUrl->addProperty("userInfo", &Url::getUserInfo);
	classUrl->addProperty("string", &Url::getString);
	classUrl->addMethod("set", &Url::set);
	//classUrl->addStaticMethod("encode", &Url::encode);
	//classUrl->addStaticMethod("encode", &Url::encode);
	//classUrl->addStaticMethod("decode", &Url::decode);
	registrar->registerClass(classUrl);

	auto classUrlConnection = new AutoRuntimeClass< UrlConnection >();
	classUrlConnection->addStaticMethod("open", &UrlConnection::open);
	classUrlConnection->addProperty("url", &UrlConnection::getUrl);
	classUrlConnection->addProperty("stream", &UrlConnection::getStream);
	registrar->registerClass(classUrlConnection);

	auto classIHttpRequestContent = new AutoRuntimeClass< IHttpRequestContent >();
	registrar->registerClass(classIHttpRequestContent);

	auto classHttpRequestContent = new AutoRuntimeClass< HttpRequestContent >();
	classHttpRequestContent->addConstructor();
	classHttpRequestContent->addConstructor< const std::wstring& >();
	registrar->registerClass(classHttpRequestContent);

	auto classHttpClient = new AutoRuntimeClass< HttpClient >();
	classHttpClient->addConstructor();
	classHttpClient->addMethod("get", &net_HttpClient_get_2);
	classHttpClient->addMethod("get", &net_HttpClient_get_3);
	classHttpClient->addMethod("put", &net_HttpClient_put_2);
	classHttpClient->addMethod("put", &net_HttpClient_put_3);
	classHttpClient->addMethod("post", &net_HttpClient_post_2);
	classHttpClient->addMethod("post", &net_HttpClient_post_3);
	registrar->registerClass(classHttpClient);

	auto classHttpClientResult = new AutoRuntimeClass< HttpClientResult >();
	classHttpClientResult->addProperty("response", &HttpClientResult::getResponse);
	classHttpClientResult->addProperty("stream", &HttpClientResult::getStream);
	registrar->registerClass(classHttpClientResult);

	auto classHttpResponse = new AutoRuntimeClass< HttpResponse >();
	classHttpResponse->addConstructor();
	classHttpResponse->addProperty("statusCode", &HttpResponse::getStatusCode);
	classHttpResponse->addProperty("statusMessage", &HttpResponse::getStatusMessage);
	classHttpResponse->addMethod("parse", &HttpResponse::parse);
	classHttpResponse->addMethod("set", &HttpResponse::set);
	classHttpResponse->addMethod("get", &HttpResponse::get);
	registrar->registerClass(classHttpResponse);

	auto classHttpRequest = new AutoRuntimeClass< HttpRequest >();
	classHttpRequest->addConstant("MtUnknown", CastAny< int32_t >::set(HttpRequest::MtUnknown));
	classHttpRequest->addConstant("MtGet", CastAny< int32_t >::set(HttpRequest::MtGet));
	classHttpRequest->addConstant("MtHead", CastAny< int32_t >::set(HttpRequest::MtHead));
	classHttpRequest->addConstant("MtPost", CastAny< int32_t >::set(HttpRequest::MtPost));
	classHttpRequest->addConstant("MtPut", CastAny< int32_t >::set(HttpRequest::MtPut));
	classHttpRequest->addConstant("MtDelete", CastAny< int32_t >::set(HttpRequest::MtDelete));
	classHttpRequest->addConstant("MtTrace", CastAny< int32_t >::set(HttpRequest::MtTrace));
	classHttpRequest->addConstant("MtOptions", CastAny< int32_t >::set(HttpRequest::MtOptions));
	classHttpRequest->addConstant("MtConnect", CastAny< int32_t >::set(HttpRequest::MtConnect));
	classHttpRequest->addConstant("MtPatch", CastAny< int32_t >::set(HttpRequest::MtPatch));
	classHttpRequest->addProperty("method", &net_HttpRequest_getMethod);
	classHttpRequest->addProperty("resource", &HttpRequest::getResource);
	classHttpRequest->addMethod("hasValue", &HttpRequest::hasValue);
	classHttpRequest->addMethod("setValue", &HttpRequest::setValue);
	classHttpRequest->addMethod("getValue", &HttpRequest::getValue);
	classHttpRequest->addStaticMethod("parse", &HttpRequest::parse);
	registrar->registerClass(classHttpRequest);

	Ref< AutoRuntimeClass< HttpServer::IRequestListener > > classHttpServer_IRequestListener = new AutoRuntimeClass< HttpServer::IRequestListener >();
	registrar->registerClass(classHttpServer_IRequestListener);

	auto classHttpServer = new AutoRuntimeClass< HttpServer >();
	classHttpServer->addConstructor();
	classHttpServer->addMethod("create", &net_HttpServer_create);
	classHttpServer->addMethod("destroy", &HttpServer::destroy);
	classHttpServer->addMethod("setRequestListener", &net_HttpServer_setRequestListener);
	classHttpServer->addMethod("update", &HttpServer::update);
	registrar->registerClass(classHttpServer);
}

}
