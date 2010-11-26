#include "Amalgam/Editor/LaunchTargetAction.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Net/SocketAddressIPv4.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_targetConnectionPort = 34000;
const uint16_t c_remoteDatabasePort = 35000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.LaunchTargetAction", LaunchTargetAction, ITargetAction)

LaunchTargetAction::LaunchTargetAction(TargetInstance* targetInstance, TargetManager* targetManager, const Guid& activeGuid)
:	m_targetInstance(targetInstance)
,	m_targetManager(targetManager)
,	m_activeGuid(activeGuid)
{
}

bool LaunchTargetAction::execute()
{
	Ref< IStream > file;

	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsLaunching);

	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();

	std::wstring targetPath = target->getTargetPath();
	std::wstring deployTool = target->getDeployTool();
	std::wstring executable = target->getExecutable();
	std::wstring configuration = target->getConfiguration();

	// Determine our interface address; we let applications know where to find data.
	RefArray< net::SocketAddressIPv4 > interfaces = net::SocketAddressIPv4::getInterfaces();
	std::wstring host = !interfaces.empty() ? interfaces[0]->getHostName() : L"localhost";

	// Load application configuration.
	if (!(file = FileSystem::getInstance().open(configuration, File::FmRead)))
	{
		log::error << L"Unable to open template configuration \"" << configuration << L"\"" << Endl;
		return false;
	}

	Ref< Settings > settings = Settings::read< xml::XmlDeserializer >(file);
	file->close();

	if (!settings)
	{
		log::error << L"Unable to read template configuration \"" << configuration << L"\"" << Endl;
		return false;
	}

	// Build application database connection string; applications should
	// connect to our database server.
	StringOutputStream ss;
	ss << L"provider=traktor.db.RemoteDatabase;host=" << host << L":" << c_remoteDatabasePort << L";database=" << m_targetInstance->getName();
	settings->setProperty< PropertyString >(L"Amalgam.Database", ss.str());

	// Expose target connection host.
	settings->setProperty< PropertyString >(L"Amalgam.TargetManager/Host", host);
	settings->setProperty< PropertyInteger >(L"Amalgam.TargetManager/Port", c_targetConnectionPort);

	// Append optional "active guid" to application configuration; it's the applications responsibility
	// to determine what to do with it.
	if (m_activeGuid.isValid() && !m_activeGuid.isNull())
		settings->setProperty< PropertyString >(L"Amalgam.ActiveGuid", m_activeGuid.format());

	// Write modified configuration file to target.
	if (!(file = FileSystem::getInstance().open(targetPath + L"/Application.config", File::FmWrite)))
	{
		log::error << L"Unable to create configuration \"" << targetPath << L"/Application.config\"" << Endl;
		return false;
	}

	settings->write< xml::XmlSerializer >(file);
	file->close();

	OS::envmap_t envmap = OS::getInstance().getEnvironment();

	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
	envmap[L"DEPLOY_TARGETPATH"] = targetPath;
	envmap[L"DEPLOY_DEPLOYTOOL"] = deployTool;
	envmap[L"DEPLOY_EXECUTABLE"] = executable;

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool,
		L"launch",
		targetPath,
		&envmap,
		true,
		true,
		false
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << deployTool << L"\"" << Endl;
		m_targetInstance->setState(TsIdle);
		return false;
	}

	// Accept connection from launched target.
	if (!m_targetManager->accept(m_targetInstance))
	{
		m_targetInstance->setState(TsIdle);
		return false;
	}

	return true;
}

	}
}
