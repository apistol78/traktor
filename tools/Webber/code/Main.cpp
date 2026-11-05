/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Blog/BlogData.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Demo/DemoApplication.h"
#include "Demo/ImageStore.h"
#include "Demo/UserData.h"
#include "Forum/ForumData.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Http/HttpServer.h"
#include "Project/ProjectData.h"
#include "Webber/HttpRequestListener.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

namespace
{

bool saveData(const ISerializable* data, const Path& fileName)
{
	// Write a temporary file and replace the data file with it; a failed save never damages existing data.
	const Path tempFileName = fileName.getPathName() + L".tmp";

	Ref< traktor::IStream > file = FileSystem::getInstance().open(tempFileName, File::FmWrite);
	if (file)
	{
		const bool result = xml::XmlSerializer(file).writeObject(data);
		file->close();

		if (result && FileSystem::getInstance().move(fileName, tempFileName, true))
			return true;
	}

	log::error << L"Unable to save data to \"" << fileName.getPathName() << L"\"." << Endl;
	return false;
}

/*! Load data file, or create it from sample data when missing; null when existing data cannot be loaded. */
template < typename DataType >
Ref< DataType > loadData(const Path& fileName, Ref< DataType > (*createSample)())
{
	if (!FileSystem::getInstance().exist(fileName))
	{
		Ref< DataType > data = createSample();
		if (!saveData(data, fileName))
			return nullptr;

		log::info << L"Created \"" << fileName.getPathName() << L"\" with sample data." << Endl;
		return data;
	}

	// Existing data must load; never replace it with sample data.
	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	Ref< DataType > data = file ? xml::XmlDeserializer(file, fileName.getPathName()).readObject< DataType >() : nullptr;
	if (file)
		file->close();

	if (!data)
	{
		log::error << L"Unable to load data from \"" << fileName.getPathName() << L"\"." << Endl;
		return nullptr;
	}

	log::info << L"Loaded data from \"" << fileName.getPathName() << L"\"." << Endl;
	return data;
}

}

int main(int argc, const char** argv)
{
	const CommandLine cmdLine(argc, argv);
	const uint16_t port = cmdLine.hasOption(L'p', L"port") ? (uint16_t)cmdLine.getOption(L'p', L"port").getInteger() : 1337;
	const Path dataPath = cmdLine.hasOption(L'd', L"data") ? Path(cmdLine.getOption(L'd', L"data").getString()) : Path(L"WebberData");
	const bool verbose = cmdLine.hasOption(L'v', L"verbose");

	// Each feature has its own data file.
	if (!FileSystem::getInstance().makeAllDirectories(dataPath))
	{
		log::error << L"Unable to create data directory \"" << dataPath.getPathName() << L"\"." << Endl;
		return 1;
	}

	const Path usersFileName = dataPath + Path(L"Users.xml");
	const Path blogFileName = dataPath + Path(L"Blog.xml");
	const Path projectsFileName = dataPath + Path(L"Projects.xml");
	const Path forumFileName = dataPath + Path(L"Forum.xml");

	Ref< demo::Users > userData = loadData(usersFileName, &demo::createSampleUsers);
	Ref< blog::Blog > blogData = loadData(blogFileName, &blog::createSampleBlog);
	Ref< project::Projects > projectData = loadData(projectsFileName, &project::createSampleProjects);
	Ref< forum::Forum > forumData = loadData(forumFileName, &forum::createSampleForum);
	if (!userData || !blogData || !projectData || !forumData)
		return 1;

	Ref< demo::ImageStore > images = new demo::ImageStore(dataPath + Path(L"Images"));

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

	server->setRequestListener(new wbr::HttpRequestListener(
		[&]() -> Ref< wbr::IApplication > {
			return new demo::DemoApplication(
				userData->users,
				images,
				*blogData, [&]() { saveData(blogData, blogFileName); },
				*projectData, [&]() { saveData(projectData, projectsFileName); },
				*forumData, [&]() { saveData(forumData, forumFileName); }
			);
		},
		verbose
	));

	log::info << L"Webber demo running at http://localhost:" << server->getListenPort() << L"/" << Endl;

	for (;;)
		server->update(100);
}
