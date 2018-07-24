/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/SkeletonFormat.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormat", SkeletonFormat, Object)

Ref< Skeleton > SkeletonFormat::readAny(const Path& filePath, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (file)
		return readAny(file, filePath.getExtension(), offset, scale, radius, invertX, invertZ);
	else
		return 0;
}

Ref< Skeleton > SkeletonFormat::readAny(IStream* stream, const std::wstring& extension, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ)
{
	Ref< Skeleton > sk;

	TypeInfoSet formatTypes;
	type_of< SkeletonFormat >().findAllOf(formatTypes, false);

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< SkeletonFormat > format = dynamic_type_cast< SkeletonFormat* >((*i)->createInstance());
		if (!format)
			continue;

		if (!format->supportFormat(extension))
			continue;

		sk = format->read(stream, offset, scale, radius, invertX, invertZ);
		if (sk)
			break;
	}

	return sk;
}

	}
}
