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
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/IPeer2PeerProvider.h"
#include "Net/Replication/IReplicatorEventListener.h"
#include "Net/Replication/IReplicatorStateListener.h"
#include "Net/Replication/MeasureP2PProvider.h"
#include "Net/Replication/Peer2PeerTopology.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
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

class ReplicatorConfiguration : public Object
{
	T_RTTI_CLASS;

public:
	ReplicatorConfiguration()
	{
	}

	ReplicatorConfiguration(const Replicator::Configuration& configuration)
	:	m_configuration(configuration)
	{
	}

	void setTimeVarianceThreshold(float timeVarianceThreshold) { m_configuration.timeVarianceThreshold = timeVarianceThreshold; }

	float getTimeVarianceThreshold() const { return m_configuration.timeVarianceThreshold; }

	void setNearDistance(float nearDistance) { m_configuration.nearDistance = nearDistance; }

	float getNearDistance() const { return m_configuration.nearDistance; }

	void setFarDistance(float farDistance) { m_configuration.farDistance = farDistance; }

	float getFarDistance() const { return m_configuration.farDistance; }

	void setFurthestDistance(float furthestDistance) { m_configuration.furthestDistance = furthestDistance; }

	float getFurthestDistance() const { return m_configuration.furthestDistance; }

	void setTimeUntilTxStateNear(float timeUntilTxStateNear) { m_configuration.timeUntilTxStateNear = timeUntilTxStateNear; }

	float getTimeUntilTxStateNear() const { return m_configuration.timeUntilTxStateNear; }

	void setTimeUntilTxStateFar(float timeUntilTxStateFar) { m_configuration.timeUntilTxStateFar = timeUntilTxStateFar; }

	float getTimeUntilTxStateFar() const { return m_configuration.timeUntilTxStateFar; }

	void setTimeUntilTxPing(float timeUntilTxPing) { m_configuration.timeUntilTxPing = timeUntilTxPing; }

	float getTimeUntilTxPing() const { return m_configuration.timeUntilTxPing; }

	const Replicator::Configuration& getConfiguration() const { return m_configuration; }

private:
	Replicator::Configuration m_configuration;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorConfiguration", ReplicatorConfiguration, Object)

class ReplicatorListener : public IReplicatorStateListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(Replicator* replicator, float eventTime, uint32_t eventId, ReplicatorProxy* proxy, const Object* eventObject) final
	{
		Any argv[] =
		{
			CastAny< Object* >::set(replicator),
			CastAny< float >::set(eventTime),
			CastAny< int32_t >::set(int32_t(eventId)),
			CastAny< Object* >::set(proxy),
			CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

class ReplicatorEventListener : public IReplicatorEventListener
{
	T_RTTI_CLASS;

public:
	ReplicatorEventListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual bool notify(Replicator* replicator, float eventTime, ReplicatorProxy* fromProxy, const Object* eventObject) final
	{
		Any argv[] =
		{
			CastAny< Object* >::set(replicator),
			CastAny< float >::set(eventTime),
			CastAny< Object* >::set(fromProxy),
			CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			return m_delegate->call(sizeof_array(argv), argv).getBoolean();
		else
			return false;
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorListener", ReplicatorListener, IReplicatorStateListener)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorEventListener", ReplicatorEventListener, IReplicatorEventListener)

bool net_Replicator_create(Replicator* self, INetworkTopology* topology, const ReplicatorConfiguration* configuration)
{
	return self->create(topology, configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

void net_Replicator_setConfiguration(Replicator* self, const ReplicatorConfiguration* configuration)
{
	self->setConfiguration(configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

Ref< ReplicatorConfiguration > net_Replicator_getConfiguration(Replicator* self)
{
	return new ReplicatorConfiguration(self->getConfiguration());
}

Ref< Object > Replicator_addListener(Replicator* self, IRuntimeDelegate* delegate)
{
	Ref< IReplicatorStateListener > listener = new ReplicatorListener(delegate);
	self->addListener(listener);
	return listener;
}

void Replicator_removeListener(Replicator* self, Object* listener)
{
	self->removeListener(mandatory_non_null_type_cast< IReplicatorStateListener* >(listener));
}

Ref< Object > Replicator_addEventListener(Replicator* self, const TypeInfo& eventType, IRuntimeDelegate* delegate)
{
	Ref< IReplicatorEventListener > listener = new ReplicatorEventListener(delegate);
	self->addEventListener(eventType, listener);
	return listener;
}

void Replicator_removeEventListener(Replicator* self, Object* listener)
{
	self->removeEventListener(mandatory_non_null_type_cast< IReplicatorEventListener* >(listener));
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

	auto classState = new AutoRuntimeClass< State >();
	registrar->registerClass(classState);

	auto classStateTemplate = new AutoRuntimeClass< StateTemplate >();
	classStateTemplate->addMethod("extrapolate", &StateTemplate::extrapolate);
	registrar->registerClass(classStateTemplate);

	auto classINetworkTopology = new AutoRuntimeClass< INetworkTopology >();
	registrar->registerClass(classINetworkTopology);

	auto classIPeer2PeerProvider = new AutoRuntimeClass< IPeer2PeerProvider >();
	registrar->registerClass(classIPeer2PeerProvider);

	auto classMeasureP2PProvider = new AutoRuntimeClass< MeasureP2PProvider >();
	classMeasureP2PProvider->addConstructor< IPeer2PeerProvider* >();
	classMeasureP2PProvider->addProperty("sendBitsPerSecond", &MeasureP2PProvider::getSendBitsPerSecond);
	classMeasureP2PProvider->addProperty("recvBitsPerSecond", &MeasureP2PProvider::getRecvBitsPerSecond);
	registrar->registerClass(classMeasureP2PProvider);

	auto classPeer2PeerTopology = new AutoRuntimeClass< Peer2PeerTopology >();
	classPeer2PeerTopology->addConstructor< IPeer2PeerProvider* >();
	classPeer2PeerTopology->addMethod("setIAmInterval", &Peer2PeerTopology::setIAmInterval);
	classPeer2PeerTopology->addMethod("setPropagateCMaskInterval", &Peer2PeerTopology::setPropagateCMaskInterval);
	registrar->registerClass(classPeer2PeerTopology);

	auto classReplicatorProxy = new AutoRuntimeClass< ReplicatorProxy >();
	classReplicatorProxy->addProperty("handle", &ReplicatorProxy::getHandle);
	classReplicatorProxy->addProperty("name", &ReplicatorProxy::getName);
	classReplicatorProxy->addProperty("user", &ReplicatorProxy::getUser);
	classReplicatorProxy->addProperty("status", &ReplicatorProxy::getStatus);
	classReplicatorProxy->addProperty("latencyReliable", &ReplicatorProxy::isLatencyReliable);
	classReplicatorProxy->addProperty("latency", &ReplicatorProxy::getLatency);
	classReplicatorProxy->addProperty("latencySpread", &ReplicatorProxy::getLatencySpread);
	classReplicatorProxy->addProperty("reverseLatency", &ReplicatorProxy::getReverseLatency);
	classReplicatorProxy->addProperty("reverseLatencySpread", &ReplicatorProxy::getReverseLatencySpread);
	classReplicatorProxy->addProperty("timeRate", &ReplicatorProxy::getTimeRate);
	classReplicatorProxy->addProperty("connected", &ReplicatorProxy::isConnected);
	classReplicatorProxy->addProperty("primary", &ReplicatorProxy::isPrimary);
	classReplicatorProxy->addProperty("relayed", &ReplicatorProxy::isRelayed);
	classReplicatorProxy->addProperty("object", &ReplicatorProxy::setObject, &ReplicatorProxy::getObject);
	classReplicatorProxy->addProperty("origin", &ReplicatorProxy::setOrigin, &ReplicatorProxy::getOrigin);
	classReplicatorProxy->addProperty("stateTemplate", &ReplicatorProxy::setStateTemplate, &ReplicatorProxy::getStateTemplate);
	classReplicatorProxy->addMethod("getState", &ReplicatorProxy::getState);
	classReplicatorProxy->addMethod("getFilteredState", &ReplicatorProxy::getFilteredState);
	classReplicatorProxy->addMethod("setPrimary", &ReplicatorProxy::setPrimary);
	classReplicatorProxy->addMethod("resetLatencies", &ReplicatorProxy::resetLatencies);
	classReplicatorProxy->addMethod("resetStates", &ReplicatorProxy::resetStates);
	classReplicatorProxy->addMethod("setSendState", &ReplicatorProxy::setSendState);
	classReplicatorProxy->addMethod("sendEvent", &ReplicatorProxy::sendEvent);
	registrar->registerClass(classReplicatorProxy);

	auto classIReplicatorStateListener = new AutoRuntimeClass< IReplicatorStateListener >();
	registrar->registerClass(classIReplicatorStateListener);

	auto classIReplicatorEventListener = new AutoRuntimeClass< IReplicatorEventListener >();
	registrar->registerClass(classIReplicatorEventListener);

	auto classReplicatorConfiguration = new AutoRuntimeClass< ReplicatorConfiguration >();
	classReplicatorConfiguration->addConstructor();
	classReplicatorConfiguration->addProperty("timeVarianceThreshold", &ReplicatorConfiguration::setTimeVarianceThreshold, &ReplicatorConfiguration::getTimeVarianceThreshold);
	classReplicatorConfiguration->addProperty("nearDistance", &ReplicatorConfiguration::setNearDistance, &ReplicatorConfiguration::getNearDistance);
	classReplicatorConfiguration->addProperty("farDistance", &ReplicatorConfiguration::setFarDistance, &ReplicatorConfiguration::getFarDistance);
	classReplicatorConfiguration->addProperty("furthestDistance", &ReplicatorConfiguration::setFurthestDistance, &ReplicatorConfiguration::getFurthestDistance);
	classReplicatorConfiguration->addProperty("timeUntilTxStateNear", &ReplicatorConfiguration::setTimeUntilTxStateNear, &ReplicatorConfiguration::getTimeUntilTxStateNear);
	classReplicatorConfiguration->addProperty("timeUntilTxStateFar", &ReplicatorConfiguration::setTimeUntilTxStateFar, &ReplicatorConfiguration::getTimeUntilTxStateFar);
	classReplicatorConfiguration->addProperty("timeUntilTxPing", &ReplicatorConfiguration::setTimeUntilTxPing, &ReplicatorConfiguration::getTimeUntilTxPing);
	registrar->registerClass(classReplicatorConfiguration);

	auto classReplicator = new AutoRuntimeClass< Replicator >();
	classReplicator->addConstructor();
	classReplicator->addProperty("name", &Replicator::getName);
	classReplicator->addProperty("status", &Replicator::setStatus, &Replicator::getStatus);
	classReplicator->addProperty("primary", &Replicator::isPrimary);
	classReplicator->addProperty("state", &Replicator::setState, &Replicator::getState);
	classReplicator->addProperty("averageLatency", &Replicator::getAverageLatency);
	classReplicator->addProperty("averageReverseLatency", &Replicator::getAverageReverseLatency);
	classReplicator->addProperty("bestLatency", &Replicator::getBestLatency);
	classReplicator->addProperty("bestReverseLatency", &Replicator::getBestReverseLatency);
	classReplicator->addProperty("worstLatency", &Replicator::getWorstLatency);
	classReplicator->addProperty("worstReverseLatency", &Replicator::getWorstReverseLatency);
	classReplicator->addProperty("time", &Replicator::getTime);
	classReplicator->addProperty("timeVariance", &Replicator::getTimeVariance);
	classReplicator->addProperty("timeSynchronized", &Replicator::isTimeSynchronized);
	classReplicator->addProperty("proxyCount", &Replicator::getProxyCount);
	classReplicator->addProperty("primaryProxy", &Replicator::getPrimaryProxy);
	classReplicator->addMethod("create", &net_Replicator_create);
	classReplicator->addMethod("destroy", &Replicator::destroy);
	classReplicator->addMethod("setConfiguration", &net_Replicator_setConfiguration);
	classReplicator->addMethod("getConfiguration", &net_Replicator_getConfiguration);
	classReplicator->addMethod("addEventType", &Replicator::addEventType);
	classReplicator->addMethod("removeAllEventTypes", &Replicator::removeAllEventTypes);
	classReplicator->addMethod("addListener", &Replicator_addListener);
	classReplicator->addMethod("removeListener", &Replicator_removeListener);
	classReplicator->addMethod("removeAllListeners", &Replicator::removeAllListeners);
	classReplicator->addMethod("addEventListener", &Replicator_addEventListener);
	classReplicator->addMethod("removeEventListener", &Replicator_removeEventListener);
	classReplicator->addMethod("removeAllEventListeners", &Replicator::removeAllEventListeners);
	classReplicator->addMethod("update", &Replicator::update);
	classReplicator->addMethod("flush", &Replicator::flush);
	classReplicator->addMethod("setOrigin", &Replicator::setOrigin);
	classReplicator->addMethod("setStateTemplate", &Replicator::setStateTemplate);
	classReplicator->addMethod("setSendState", &Replicator::setSendState);
	classReplicator->addMethod("getProxy", &Replicator::getProxy);
	classReplicator->addMethod("resetAllLatencies", &Replicator::resetAllLatencies);
	classReplicator->addMethod("broadcastEvent", &Replicator::broadcastEvent);
	classReplicator->addMethod("sendEventToPrimary", &Replicator::sendEventToPrimary);
	classReplicator->addMethod("setTimeSynchronization", &Replicator::setTimeSynchronization);
	registrar->registerClass(classReplicator);
}

	}
}
