#include <Core/Io/IStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/String.h>

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 3)
	{
		traktor::log::info << L"BinaryInclude : Usage BinaryInclude.exe (option) input output symbol" << Endl;
		traktor::log::info << L"                -v, -verbose" << Endl;
		return 1;
	}

	Path inputFile = cmdLine.getString(0);
	Path outputFile = cmdLine.getString(1);
	std::wstring symbol = cmdLine.getString(2);
	bool verbose = cmdLine.hasOption('v', L"verbose");

	// Ensure symbol doesn't contain invalid characters.
	symbol = replaceAll< std::wstring >(symbol, '.', '_');
	symbol = replaceAll< std::wstring >(symbol, '/', '_');
	symbol = replaceAll< std::wstring >(symbol, '\\', '_');

	// Check if we need to build; do not touch output uneccessarily.
	Ref< File > fileInputFile = FileSystem::getInstance().get(inputFile);
	if (!fileInputFile)
	{
		traktor::log::error << L"Unable to convert file; no such file" << Endl;
		return 1;
	}

	Ref< File > fileOutputFile = FileSystem::getInstance().get(outputFile);
	if (fileOutputFile)
	{
		// Output file exists; only create output if source file been modified since output file created.
		if (fileInputFile->getLastWriteTime()  <= fileOutputFile->getCreationTime())
		{
			if (verbose)
				traktor::log::info << L"File up-to-date; skipped" << Endl;
			return 0;
		}
	}

	Ref< IStream > input = FileSystem::getInstance().open(inputFile, File::FmRead);
	if (!input)
	{
		traktor::log::error << L"Unable to convert file; failed to open file \"" << inputFile.getPathName() << L"\"" << Endl;
		return 1;
	}

	FileSystem::getInstance().makeAllDirectories(outputFile.getPathOnly());

	Ref< IStream > output = FileSystem::getInstance().open(outputFile, File::FmWrite);
	if (!output)
	{
		traktor::log::error << L"Unable to convert file; failed to create file \"" << outputFile.getPathName() << L"\"" << Endl;
		return 1;
	}

	{
		FileOutputStream s(output, new Utf8Encoding());

		s << L"#ifndef _INCLUDE_" << toUpper(symbol) << L"_H" << Endl;
		s << L"#define _INCLUDE_" << toUpper(symbol) << L"_H" << Endl;
		s << Endl;

		s << L"namespace" << Endl;
		s << L"{" << Endl;
		s << Endl;

		s << L"const unsigned char " << symbol << L"[] =" << Endl;
		s << L"{" << Endl;

		while (input->available() > 0)
		{
			unsigned char buffer[16];

			int count = input->read(buffer, sizeof(buffer));
			T_ASSERT (count > 0);

			s << L"\t";

			for (int i = 0; i < count; ++i)
			{
				const wchar_t hex[] = { L"0123456789abcdef" };

				wchar_t fmt[] = { hex[buffer[i] >> 4], hex[buffer[i] & 15], 0 };
				s << L"0x" << fmt;

				if (count >= sizeof(buffer) || i < count - 1)
					s << L", ";
			}

			s << Endl;
		}

		s << L"};" << Endl;
		s << Endl;

		s << L"}" << Endl;
		s << Endl;

		s << L"#endif\t// _INCLUDE_" << toUpper(symbol) << L"_H" << Endl;
	}

	output->close();
	input->close();

	if (verbose)
		traktor::log::info << L"File \"" << outputFile.getPathName() << L"\" converted successfully" << Endl;
}
