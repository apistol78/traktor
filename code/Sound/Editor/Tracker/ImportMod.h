#pragma once

#include "Core/Object.h"

namespace traktor
{

class Path;

	namespace db
	{

class Group;

	}

	namespace sound
	{

class ImportMod : public Object
{
	T_RTTI_CLASS;

public:
	bool import(const Path& fileName, const Path& assetPath, const Path& samplePath, db::Group* group) const;
};

	}
}
