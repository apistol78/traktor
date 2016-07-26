#ifndef traktor_run_IOutput_H
#define traktor_run_IOutput_H

#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

/*! \brief Scriptable output writer interface.
 * \ingroup Run
 */
class IOutput : public Object
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) = 0;

	virtual void printLn(const std::wstring& s) = 0;
};

	}
}

#endif	// traktor_run_IOutput_H
