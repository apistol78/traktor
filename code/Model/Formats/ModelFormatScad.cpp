/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Core/System/IProcess.h"
#include "Model/Formats/ModelFormatScad.h"
#include "Model/Formats/ModelFormatStl.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatScad", 0, ModelFormatScad, ModelFormat)

void ModelFormatScad::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"OpenSCAD object";
	outExtensions.push_back(L"scad");
}

bool ModelFormatScad::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"scad") == 0;
}

Ref< Model > ModelFormatScad::read(IStream* stream, uint32_t importFlags) const
{
	// Generate temporary filenames.
	char buf[L_tmpnam];
	char* st = tmpnam(buf);
	if (!st)
		return 0;

	Path tmpFile = mbstows(st);
	Path sourceTmpFile = tmpFile.getPathNameNoExtension() + L".scad";
	Path outputTmpFile = tmpFile.getPathNameNoExtension() + L".stl";

	// Write out entire stream into temporary file.
	Ref< IStream > sourceFile = FileSystem::getInstance().open(sourceTmpFile, File::FmWrite);
	if (!sourceFile)
		return 0;

	if (!StreamCopy(sourceFile, stream).execute())
	{
		FileSystem::getInstance().remove(sourceTmpFile);
		return 0;
	}

	sourceFile->close();

	// Figure out OpenSCAD executable.
	std::wstring executable = L"openscad";

#if defined(_WIN32)
	std::wstring openCommand;
	if (OS::getInstance().getRegistry(L"HKEY_LOCAL_MACHINE", L"SOFTWARE\\Classes\\OpenSCAD_File\\shell\\open\\command", L"", openCommand))
	{
		if (endsWith< std::wstring >(openCommand, L" \"%1\""))
			openCommand = openCommand.substr(0, openCommand.length() - 5);
		executable = openCommand;
	}
#endif

	// Convert OpenSCAD script into STL file.
	std::wstring cmdLine = executable + L" -o " + outputTmpFile.getFileName() + L" " + sourceTmpFile.getFileName();
	Ref< IProcess > openscad = OS::getInstance().execute(cmdLine, tmpFile.getPathOnly(), 0, false, false, false);
	if (!openscad)
	{
		FileSystem::getInstance().remove(sourceTmpFile);
		return 0;
	}

	if (!openscad->wait())
	{
		FileSystem::getInstance().remove(sourceTmpFile);
		return 0;
	}

	// Open output temporary file.
	Ref< IStream > outputFile = FileSystem::getInstance().open(outputTmpFile, File::FmRead);
	if (!outputFile)
	{
		FileSystem::getInstance().remove(sourceTmpFile);
		return 0;
	}

	Ref< Model > md = ModelFormatStl().read(outputFile, importFlags);

	outputFile->close();

	// Cleanup temporary files.
	FileSystem::getInstance().remove(sourceTmpFile);
	FileSystem::getInstance().remove(outputTmpFile);
	return md;
}

bool ModelFormatScad::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
