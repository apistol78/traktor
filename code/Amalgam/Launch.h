#ifndef traktor_amalgam_Launch_H
#define traktor_amalgam_Launch_H

#include "Core/Config.h"
#include "Core/Misc/CommandLine.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class ISessionManagerProvider;

	}

	namespace amalgam
	{

class IStateFactory;

/*! \brief Amalgam launcher main.
 *
 * Call this function as soon as possible as it
 * will maintain the entire application and
 * it dependent systems.
 *
 * \param cmdLine Command line.
 * \param settingsFileName File name of settings file.
 * \param sessionManagerProvider Online session manager provider.
 * \param stateFactory State factory implementation for initial state.
 * \return Error code.
 */
int32_t T_DLLCLASS amalgamMain(
	const CommandLine& cmdLine,
	const std::wstring& settingsFileName,
	online::ISessionManagerProvider* sessionManagerProvider,
	amalgam::IStateFactory* stateFactory
);

	}
}

#endif	// traktor_amalgam_Launch_H
