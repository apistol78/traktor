#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Drawing/Image.h>
#include <Drawing/PixelFormat.h>
#include <Script/AutoScriptClass.h>
#include <Script/IScriptContext.h>
#include <Script/IScriptResource.h>
#include <Script/Js/ScriptManagerJs.h>

using namespace traktor;


class Color : public Object
{
	T_RTTI_CLASS;

public:
	float r;
	float g;
	float b;
	float a;

	Color()
	:	r(0.0f)
	,	g(0.0f)
	,	b(0.0f)
	,	a(1.0f)
	{
	}

	Color(float r_, float g_, float b_)
	:	r(r_)
	,	g(g_)
	,	b(b_)
	,	a(1.0f)
	{
	}

	Color(float r_, float g_, float b_, float a_)
	:	r(r_)
	,	g(g_)
	,	b(b_)
	,	a(a_)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"Color", Color, Object)

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 2)
	{
		traktor::log::info << L"ImageGenerator v1.0" << Endl;
		traktor::log::info << Endl;
		traktor::log::info << L"Scriptable Image Generator tool" << Endl;
		traktor::log::info << Endl;
		traktor::log::info << L"XmlTransform <-w=width> <-h=height> <script file> <output>" << Endl;
		return -1;
	}


	int32_t width = cmdLine.hasOption('w') ? cmdLine.getOption('w').getInteger() : 256;
	int32_t height = cmdLine.hasOption('h') ? cmdLine.getOption('h').getInteger() : 256;


	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getA8R8G8B8(),
		width,
		height
	);


	Ref< IStream > file = FileSystem::getInstance().open(cmdLine.getString(0), File::FmRead);
	if (!file)
	{
		traktor::log::error << L"Unable to read script; no such file" << Endl;
		return 1;
	}

	Utf8Encoding encoding;
	StringReader reader(file, &encoding);
	StringOutputStream ss;
	std::wstring tmp;

	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();


	Ref< script::IScriptManager > scriptManager = new script::ScriptManagerJs();

	Ref< script::AutoScriptClass< Color > > colorClass = new script::AutoScriptClass< Color >();
	colorClass->addConstructor();
	colorClass->addConstructor< float, float, float >();
	colorClass->addConstructor< float, float, float, float >();
	colorClass->addProperty(L"r", &Color::r);
	colorClass->addProperty(L"g", &Color::g);
	colorClass->addProperty(L"b", &Color::b);
	colorClass->addProperty(L"a", &Color::a);
	scriptManager->registerClass(colorClass);

	Ref< script::IScriptResource > scriptResource = scriptManager->compile(ss.str(), false, 0);
	if (!scriptResource)
	{
		log::error << L"Error when compiling script; unable to transform document" << Endl;
		return false;
	}

	Ref< script::IScriptContext > context = scriptManager->createContext();
	if (!context->executeScript(scriptResource, Guid()))
	{
		log::error << L"Error when executing script; unable to transform document" << Endl;
		return false;
	}


	script::Any args[2];
	args[0] = script::Any(width);
	args[1] = script::Any(height);
	context->executeFunction(L"initialize", 2, args);


	traktor::log::info << L"Generating";

	for (int32_t y = 0; y < height; ++y)
	{
		traktor::log::info << L".";
		for (int32_t x = 0; x < width; ++x)
		{
			script::Any args[2];
			args[0] = script::Any(x);
			args[1] = script::Any(y);
			
			script::Any ret = context->executeFunction(L"evaluate", 2, args);

			const Color* color = dynamic_type_cast< const Color* >(ret.getObject());
			if (color)
			{
				image->setPixel(x, y, Color4f(
					color->r,
					color->g,
					color->b,
					color->a
				));
			}
		}
	}


	traktor::log::info << Endl << L"Saving..." << Endl;

	if (!image->save(cmdLine.getString(1)))
		traktor::log::error << L"Failed to save image" << Endl;
	else
		traktor::log::info << L"Finished" << Endl;

	return 0;
}
