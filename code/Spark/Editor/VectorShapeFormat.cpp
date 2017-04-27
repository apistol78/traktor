/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Spark/Editor/VectorShapeFormat.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.VectorShapeFormat", VectorShapeFormat, Object)

Ref< VectorShape > VectorShapeFormat::readAny(const Path& filePath)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (file)
		return readAny(file, filePath.getExtension());
	else
		return 0;
}

Ref< VectorShape > VectorShapeFormat::readAny(IStream* stream, const std::wstring& extension)
{
	Ref< VectorShape > vs;

	TypeInfoSet formatTypes;
	type_of< VectorShapeFormat >().findAllOf(formatTypes, false);

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< VectorShapeFormat > shapeFormat = dynamic_type_cast< VectorShapeFormat* >((*i)->createInstance());
		if (!shapeFormat)
			continue;

		if (!shapeFormat->supportFormat(extension))
			continue;

		vs = shapeFormat->read(stream);
		if (vs)
			break;
	}

	return vs;
}

	}
}
