#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/Text.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Math/Transform.h>
#include <Core/Math/Format.h>
#include <Core/Log/Log.h>

using namespace traktor;

void recursiveFindFiles(const Path& currentPath, std::set< Path >& outFiles)
{
	RefArray< File > localFiles;
	FileSystem::getInstance().find(currentPath.getPathName() + L"/*.*", localFiles);

	for (RefArray< File >::iterator i = localFiles.begin(); i != localFiles.end(); ++i)
	{
		if ((*i)->isReadOnly())
			continue;

		Path filePath = (*i)->getPath();
		if (filePath.getFileName() == L"." || filePath.getFileName() == L"..")
			continue;

		if (!(*i)->isDirectory() && compareIgnoreCase(filePath.getExtension(), L"xdi") == 0)
			outFiles.insert(filePath);
		else
			recursiveFindFiles(filePath, outFiles);
	}
}

uint32_t transform(xml::Element* element)
{
	uint32_t changes = 0;

	if (element->getName() == L"transform" || element->getName() == L"localTransform")
	{
		std::vector< float > values;
		Split< std::wstring, float >::any(element->getValue(), L",", values);

		if (values.size() == 16)
		{
			Matrix44 M = Matrix44::identity();
			M.load(&values[0]);

			Transform T(M);

			element->removeAllChildren();

			StringOutputStream sstf; sstf << T.translation();
			StringOutputStream ssrt; ssrt << T.rotation();

			Ref< xml::Element > xtranslation = gc_new< xml::Element >(L"translation");
			xtranslation->addChild(gc_new< xml::Text >( sstf.str() ));

			Ref< xml::Element > xrotation = gc_new< xml::Element >(L"rotation");
			xrotation->addChild(gc_new< xml::Text >( ssrt.str() ));

			element->addChild(xtranslation);
			element->addChild(xrotation);

			changes++;
		}
	}

	for (Ref< xml::Node > child = element->getFirstChild(); child; child = child->getNextSibling())
	{
		xml::Element* childElement = dynamic_type_cast< xml::Element* >(child);
		if (childElement)
			changes += transform(childElement);
	}

	return changes;
}

void transform(const Path& filePath)
{
	xml::Document document;
	if (!document.loadFromFile(filePath))
	{
		log::error << L"Unable to load " << filePath.getPathName() << Endl;
		return;
	}

	log::info << L"Transforming \"" << filePath.getPathName() << L"\"..." << Endl;

	uint32_t changes = transform(document.getDocumentElement());
	if (changes)
	{
		log::info << L"\t" << changes << L" change(s) found; updating file..." << Endl;

		if (!document.saveAsFile(filePath))
			log::error << L"Unable to save " << filePath.getPathName() << Endl;
	}
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	std::set< Path > files;
	recursiveFindFiles(Path(L"."), files);

	log::info << L"Found " << files.size() << L" file(s)" << Endl;

	for (std::set< Path >::iterator i = files.begin(); i != files.end(); ++i)
		transform(*i);
}
