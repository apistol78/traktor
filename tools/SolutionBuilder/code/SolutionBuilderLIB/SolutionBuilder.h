/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilder_H
#define SolutionBuilder_H

#include <Core/Object.h>
#include <Core/Misc/CommandLine.h>

class Solution;

/*! \brief Solution builder generator base class.
 *
 * Each solution generate is derived from this
 * base class. Each derived generator is instanciated
 * through RTTI from command line parameters.
 */
class SolutionBuilder : public traktor::Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const traktor::CommandLine& cmdLine) = 0;

	virtual bool generate(Solution* solution) = 0;

	virtual void showOptions() const = 0;
};

#endif	// SolutionBuilder_H
