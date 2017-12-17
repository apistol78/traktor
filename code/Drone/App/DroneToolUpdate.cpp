/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Ui/Custom/BackgroundWorkerDialog.h>
#include <Ui/Custom/BackgroundWorkerStatus.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/BufferedStream.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/ThreadManager.h>
#include <Core/Misc/Split.h>
#include <Net/Network.h>
#include <Net/UrlConnection.h>
#include <Xml/XmlDeserializer.h>
#include "Drone/App/DroneToolUpdate.h"
#include "Drone/App/UpdateBundle.h"

#undef MessageBox

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolUpdate", 0, DroneToolUpdate, DroneTool)

DroneToolUpdate::DroneToolUpdate()
{
	net::Network::initialize();
}

DroneToolUpdate::~DroneToolUpdate()
{
	net::Network::finalize();
}

void DroneToolUpdate::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > checkItem = new ui::MenuItem(ui::Command(L"Drone.Update.Check"), L"Check updates...");
	checkItem->setData(L"TOOL", this);
	outItems.push_back(checkItem);
}

bool DroneToolUpdate::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	T_ASSERT (menuItem);

	if (menuItem->getCommand() == L"Drone.Update.Check")
	{
		net::Url url(m_url);

		Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
		if (!connection)
		{
			ui::MessageBox::show(parent, L"Unable to connect to " + url.getHost(), L"Unable to connect", ui::MbIconError | ui::MbOk);
			return true;
		}

		Ref< IStream > stream = new BufferedStream(connection->getStream());
		T_ASSERT (stream);

		Ref< UpdateBundle > bundle = xml::XmlDeserializer(stream).readObject< UpdateBundle >();

		stream->close();

		if (!bundle)
		{
			ui::MessageBox::show(parent, L"Unable to download update bundle", L"Download error", ui::MbIconError | ui::MbOk);
			return true;
		}

		if (ui::MessageBox::show(parent, bundle->getDescription(), L"Update available, install?", ui::MbIconQuestion | ui::MbYesNo) != ui::DrYes)
			return true;

		ui::custom::BackgroundWorkerStatus status(bundle->getItems().size());

		Thread* updateThread = ThreadManager::getInstance().create(makeFunctor(
			this,
			&DroneToolUpdate::updateThread,
			parent,
			bundle.ptr(),
			&status
		), L"Update thread");

		ui::custom::BackgroundWorkerDialog updateDialog;
		updateDialog.create(parent, L"Updating...", L"Downloading update...", ui::Dialog::WsDefaultFixed | ui::Dialog::WsCenterDesktop);
		updateDialog.execute(updateThread, &status);
		updateDialog.destroy();

		ThreadManager::getInstance().destroy(updateThread);

		ui::MessageBox::show(parent, L"Successfully downloaded update", L"Update finished", ui::MbIconInformation | ui::MbOk);
	}

	return true;
}

void DroneToolUpdate::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"url", m_url);
	s >> Member< std::wstring >(L"rootPath", m_rootPath);
}

void DroneToolUpdate::updateThread(ui::Widget* parent, UpdateBundle* bundle, ui::custom::BackgroundWorkerStatus* status)
{
	const std::vector< UpdateBundle::BundledItem >& bundledItems = bundle->getItems();
	for (std::vector< UpdateBundle::BundledItem >::const_iterator i = bundledItems.begin(); i != bundledItems.end(); ++i)
	{
		status->notify(uint32_t(std::distance(bundledItems.begin(), i)), L"Updating " + i->path);

		net::Url url(i->url);
		Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
		if (!connection)
		{
			if (ui::MessageBox::show(parent, L"Unable to connect to " + url.getHost() + L",\ncontinue with update?", L"Unable to connect", ui::MbIconError | ui::MbYesNo) == ui::DrNo)
				return;
		}

		// Get absolute path to destination file.
		Path destinationPath(i->path);
		if (destinationPath.isRelative())
		{
			destinationPath = m_rootPath + L"/" + destinationPath.getPathName();
			if (destinationPath.isRelative())
			{
				destinationPath = FileSystem::getInstance().getAbsolutePath(destinationPath);
				T_ASSERT (!destinationPath.isRelative());
			}
		}

		// Ensure path exists.
		std::vector< std::wstring > dirs;
		Split< std::wstring >::any((std::wstring)destinationPath.getPathOnlyNoVolume(), L"/", dirs);
		if (dirs.size() > 0)
		{
			std::wstring buildPath = destinationPath.getVolume() + L":";
			for (size_t i = 0; i < dirs.size(); ++i)
			{
				buildPath += L"/" + dirs[i];
				if (FileSystem::getInstance().exist(buildPath))
					continue;
				if (!FileSystem::getInstance().makeDirectory(buildPath))
				{
					if (ui::MessageBox::show(parent, L"Unable to create directory " + buildPath + L",\ncontinue with update?", L"Unable to connect", ui::MbIconError | ui::MbYesNo) == ui::DrYes)
						continue;
					else
						return;
				}
			}
		}

		Ref< IStream > destinationStream = FileSystem::getInstance().open(destinationPath, File::FmWrite);
		if (!destinationStream)
		{
			if (ui::MessageBox::show(parent, L"Unable to create file " + i->path + L",\ncontinue with update?", L"Unable to connect", ui::MbIconError | ui::MbYesNo) == ui::DrYes)
				continue;
			else
				return;
		}

		Ref< IStream > sourceStream = new BufferedStream(connection->getStream());
		T_ASSERT (sourceStream);

		uint8_t buf[4096];
		for (;;)
		{
			int avail = sourceStream->available();
			if (avail <= 0)
				break;

			int read = std::min< int >(avail, sizeof(buf));

			int nread = sourceStream->read(buf, read);
			if (nread <= 0)
				break;

			int nwritten = destinationStream->write(buf, nread);
			if (nwritten != nread)
			{
				if (ui::MessageBox::show(parent, L"Unable to write to file " + i->path + L",\ncontinue with update?", L"Unable to connect", ui::MbIconError | ui::MbYesNo) == ui::DrNo)
					return;
			}
		}

		sourceStream->close();
		destinationStream->close();
	}
}

	}
}
