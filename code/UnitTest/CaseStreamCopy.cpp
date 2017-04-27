/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Timer/Timer.h"
#include "UnitTest/CaseStreamCopy.h"

namespace traktor
{

void CaseStreamCopy::run()
{
	Timer t;
	t.start();

	Ref< IStream > source = FileSystem::getInstance().open(L"$(TRAKTOR_HOME)/data/temp/Blob.dat", File::FmRead);
	Ref< IStream > destination = FileSystem::getInstance().open(L"$(TRAKTOR_HOME)/data/temp/BlobCopy.dat", File::FmWrite);

	if (!source || !destination)
		return;

	BufferedStream ds(destination);
	BufferedStream ss(source);
	bool res = StreamCopy(&ds, &ss).execute();
	CASE_ASSERT(res);

	destination->close();
	source->close();

	double time = t.getElapsedTime();
	log::info << L"Stream copy in " << (time * 1000.0) << L" ms" << Endl;
}

}
