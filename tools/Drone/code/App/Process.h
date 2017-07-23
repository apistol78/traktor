/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_Process_H
#define traktor_drone_Process_H

#include <Core/Platform.h>
#include <Core/Object.h>
#include <Core/Io/Path.h>

#undef MessageBox

namespace traktor
{
	namespace drone
	{

class Process : public Object
{
	T_RTTI_CLASS

public:
	Process();

	virtual ~Process();

	bool create(
		const Path& command,
		const std::wstring& commandArguments,
		const Path& workingDirectory
	);

	bool create(
		const Path& command,
		const std::wstring& commandArguments
	);

	bool wait(int timeout = -1);

	bool finished();

	uint32_t exitCode();

private:
	PROCESS_INFORMATION m_pi;
};

	}
}

#endif	// traktor_drone_Process_H
