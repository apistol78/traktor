/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Blog/BlogData.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Demo/DemoApplication.h"
#include "Demo/UserData.h"
#include "Forum/ForumData.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Http/HttpServer.h"
#include "Webber/HttpRequestListener.h"

using namespace traktor;

int main(int argc, const char** argv)
{
	const CommandLine cmdLine(argc, argv);
	const uint16_t port = cmdLine.hasOption(L'p', L"port") ? (uint16_t)cmdLine.getOption(L'p', L"port").getInteger() : 1337;
	const bool verbose = cmdLine.hasOption(L'v', L"verbose");

	if (!net::Network::initialize())
	{
		log::error << L"Unable to initialize networking." << Endl;
		return 1;
	}

	Ref< net::HttpServer > server = new net::HttpServer();
	if (!server->create(net::SocketAddressIPv4(port)))
	{
		log::error << L"Unable to create HTTP server on port " << (int32_t)port << L"." << Endl;
		return 1;
	}

	const AlignedVector< demo::User > users = demo::createSampleUsers();
	blog::Blog blogData = blog::createSampleBlog();
	forum::Forum forumData = forum::createSampleForum();

	server->setRequestListener(new wbr::HttpRequestListener(
		[&users, &blogData, &forumData]() -> Ref< wbr::IApplication > { return new demo::DemoApplication(users, blogData, forumData); },
		verbose
	));

	log::info << L"Webber demo running at http://localhost:" << server->getListenPort() << L"/" << Endl;

	for (;;)
		server->update(100);
}
