#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class OutputStream;

	namespace sb
	{

class SolutionBuilderMsvcVCXImportCommon : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool generate(OutputStream& os) const = 0;
};

	}
}

