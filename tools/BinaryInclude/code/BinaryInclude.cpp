#include <Core/Io/FileSystem.h>
#include <Core/Io/Stream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 3)
	{
		log::info << L"BinaryInclude : Usage BinaryInclude.exe input output symbol" << Endl;
		return 1;
	}

	Path inputFile = cmdLine.getString(0);
	Path outputFile = cmdLine.getString(1);
	std::wstring symbol = cmdLine.getString(2);

	Ref< Stream > input = FileSystem::getInstance().open(inputFile, File::FmRead);
	if (!input)
	{
		log::error << L"Generation failed, unable to open file \"" << inputFile.getPathName() << L"\"" << Endl;
		return 1;
	}

	FileSystem::getInstance().makeAllDirectories(outputFile.getPathOnly());

	Ref< Stream > output = FileSystem::getInstance().open(outputFile, File::FmWrite);
	if (!output)
	{
		log::error << L"Generation failed, unable to create file \"" << outputFile.getPathName() << L"\"" << Endl;
		return 1;
	}
	
	{
		FileOutputStream s(output, gc_new< Utf8Encoding >());

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
	
	log::info << L"File \"" << outputFile.getPathName() << L"\" generated successfully" << Endl;
}
