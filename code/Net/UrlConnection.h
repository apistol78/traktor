/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_UrlConnection_H
#define traktor_net_UrlConnection_H

#include "Core/Object.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace net
	{

/*! \brief URL connection base.
 * \ingroup Net
 */
class T_DLLCLASS UrlConnection : public Object
{
	T_RTTI_CLASS;

public:
	enum EstablishResult
	{
		ErSucceeded = 0,
		ErRedirect = 1,
		ErInvalidUrl = -1,
		ErConnectFailed = -2,
		ErLoginFailed = -3,
		ErNoSuchResource = -4,
		ErFailed = -100
	};

	/*! \brief Open connection from URL.
	 *
	 * \param url URL to open.
	 * \return Established connection; null if unable to connect.
	 */
	static Ref< UrlConnection > open(const Url& url);

	/*! \brief Establish connection from URL.
	 *
	 * \note
	 * You should use open method rather than calling this
	 * method directly as this might return a redirection URL
	 * which need to be followed in order to establish a
	 * connection.
	 *
	 * \param url URL to open.
	 * \param outRedirectionUrl Redirection URL.
	 * \return Establish result.
	 */
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl) = 0;

	/*! \brief Get URL of connection.
	 *
	 * \return URL of connection.
	 */
	virtual Url getUrl() const = 0;

	/*! \brief Get stream to resource.
	 *
	 * \return Stream to resource.
	 */
	virtual Ref< IStream > getStream() = 0;
};

	}
}

#endif	// traktor_net_UrlConnection_H
