#ifndef ddc_CppCodeGenerator_H
#define ddc_CppCodeGenerator_H

#include "Ddc/ICodeGenerator.h"

namespace ddc
{

class CppCodeGenerator : public ICodeGenerator
{
	T_RTTI_CLASS(CppCodeGenerator)

public:
	virtual bool generate(const traktor::Path& sourceFilePath, const DfnNode* node) const;
};

}

#endif	// ddc_CppCodeGenerator_H
