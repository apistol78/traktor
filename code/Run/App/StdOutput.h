/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_run_StdOutput_H
#define traktor_run_StdOutput_H

#include <cstdio>
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

/*! \brief Standard IO output writer.
 * \ingroup Run
 */
class StdOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StdOutput(FILE* handle);

	virtual void print(const std::wstring& s) override final;

	virtual void printLn(const std::wstring& s) override final;

private:
	FILE* m_handle;
};

	}
}

#endif	// traktor_run_StdOutput_H
