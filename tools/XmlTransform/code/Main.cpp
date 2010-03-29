#include <Xml/Attribute.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/Text.h>
#include <Core/Guid.h>
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

		if (
			!(*i)->isDirectory() &&
			(
				//compareIgnoreCase(filePath.getExtension(), L"xdi") == 0 ||
				compareIgnoreCase(filePath.getExtension(), L"config") == 0
			)
		)
			outFiles.insert(filePath);
		else
			recursiveFindFiles(filePath, outFiles);
	}
}

void createElementValue(xml::Element* element, const std::wstring& key, const std::wstring& value)
{
	Ref< xml::Element > kv = new xml::Element(key);
	kv->addChild(new xml::Text(value));
	element->addChild(kv);
}

uint32_t transform(xml::Element* element)
{
	uint32_t changes = 0;

	{
		std::wstring typeName = element->getAttribute(L"type", L"")->getValue();
		std::wstring replaceTypeName;

		if (typeName == L"traktor.editor.PropertyGroup")
			replaceTypeName = L"traktor.PropertyGroup";
		if (typeName == L"traktor.editor.PropertyString")
			replaceTypeName = L"traktor.PropertyString";
		if (typeName == L"traktor.editor.PropertyBoolean")
			replaceTypeName = L"traktor.PropertyBoolean";
		if (typeName == L"traktor.editor.PropertyFloat")
			replaceTypeName = L"traktor.PropertyFloat";
		if (typeName == L"traktor.editor.PropertyInteger")
			replaceTypeName = L"traktor.PropertyInteger";
		if (typeName == L"traktor.editor.PropertyColor")
			replaceTypeName = L"traktor.PropertyColor";
		if (typeName == L"traktor.editor.PropertyStringArray")
			replaceTypeName = L"traktor.PropertyStringArray";

		if (!replaceTypeName.empty())
		{
			element->setAttribute(L"type", replaceTypeName);
			changes++;
		}
	}

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

			Ref< xml::Element > xtranslation = new xml::Element(L"translation");
			xtranslation->addChild(new xml::Text( sstf.str() ));

			Ref< xml::Element > xrotation = new xml::Element(L"rotation");
			xrotation->addChild(new xml::Text( ssrt.str() ));

			element->addChild(xtranslation);
			element->addChild(xrotation);

			changes++;
		}
	}

	if (element->getName() == L"worldRenderSettings")
	{
		int32_t version = parseString< int32_t >(element->getAttribute(L"version", L"100")->getValue());
		if (version < 3)
		{
			element->removeAllChildren();

			createElementValue(element, L"viewNearZ", L"1");
			createElementValue(element, L"viewFarZ", L"500");
			createElementValue(element, L"depthPassEnabled", L"true");
			createElementValue(element, L"velocityPassEnable", L"false");
			createElementValue(element, L"shadowsEnabled", L"true");
			createElementValue(element, L"ssaoEnabled", L"false");
			createElementValue(element, L"shadowFarZ", L"100");
			createElementValue(element, L"shadowMapResolution", L"1024");
			createElementValue(element, L"shadowMapBias", L"0.01");

			element->setAttribute(L"version", L"3");

			changes++;
		}
	}

	//if (
	//	element->getName() == L"object" ||
	//	element->getName() == L"entityData" ||
	//	element->getName() == L"item"
	//)
	{
		std::wstring typeName = element->getAttribute(L"type", L"")->getValue();

		if (typeName == L"traktor.world.GroupEntityData")
		{
			Ref< xml::Element > xentityData = element->getChildElementByName(L"entityData");
			if (xentityData)
			{
				Ref< xml::Node > previousSibling = xentityData->getPreviousSibling();
				element->removeChild(xentityData);

				Ref< xml::Element > xinstances = new xml::Element(L"instances");

				RefArray< xml::Element > xentityDataItems;
				xentityData->get(L"item", xentityDataItems);

				for (RefArray< xml::Element >::iterator i = xentityDataItems.begin(); i != xentityDataItems.end(); ++i)
				{
					xentityData->removeChild(*i);

					xml::Element* xentityDataItemName = (*i)->getChildElementByName(L"name");
					std::wstring name = xentityDataItemName ? xentityDataItemName->getValue() : L"";

					Ref< xml::Element > xinstancesItem = new xml::Element(L"item");
					xinstancesItem->setAttribute(L"type", L"traktor.world.EntityInstance");

					Ref< xml::Element > xinstancesItemName = new xml::Element(L"name");
					xinstancesItemName->addChild(new xml::Text(name));

					xinstancesItem->addChild(xinstancesItemName);
					
					(*i)->setName(L"entityData");
					xinstancesItem->addChild(*i);

					xinstancesItem->addChild(new xml::Element(L"references"));

					xinstances->addChild(xinstancesItem);
				}

				element->insertAfter(xinstances, previousSibling);
				changes++;
			}
		}

		if (
			typeName == L"traktor.scene.SceneAsset" ||
			typeName == L"traktor.physics.RigidEntityData"
		)
		{
			Ref< xml::Element > xentityData = element->getChildElementByName(L"entityData");
			if (xentityData)
			{
				if (xentityData->getFirstChild())
				{
					Ref< xml::Element > xentityDataName = xentityData->getChildElementByName(L"name");
					std::wstring entityDataName = xentityDataName ? xentityDataName->getValue() : L"";

					Ref< xml::Node > previousSibling = xentityData->getPreviousSibling();
					element->removeChild(xentityData);

					Ref< xml::Element > xinstance = new xml::Element(L"instance");
					xinstance->setAttribute(L"type", L"traktor.world.EntityInstance");

					Ref< xml::Element > xinstanceName = new xml::Element(L"name");
					xinstanceName->addChild(new xml::Text(entityDataName));

					xinstance->addChild(xinstanceName);
					xinstance->addChild(xentityData);
					xinstance->addChild(new xml::Element(L"references"));

					element->insertAfter(xinstance, previousSibling);
				}
				else
				{
					Ref< xml::Node > previousSibling = xentityData->getPreviousSibling();
					element->removeChild(xentityData);

					Ref< xml::Element > xinstance = new xml::Element(L"instance");
					element->insertAfter(xinstance, previousSibling);
				}

				changes++;
			}

			Ref< xml::Element > xjoints = element->getChildElementByName(L"joints");
			if (xjoints)
			{
				element->removeChild(xjoints);
				changes++;
			}
		}

		if (
			typeName == L"traktor.world.GroupEntityData" ||
			typeName == L"traktor.world.ExternalEntityData" ||
			typeName == L"traktor.world.ExternalSpatialEntityData" ||
			typeName == L"traktor.physics.RigidEntityData" ||
			typeName == L"traktor.world.DirectionalLightEntityData" ||
			typeName == L"traktor.terrain.TerrainEntityData"
		)
		{
			Ref< xml::Element > xentityDataName = element->getChildElementByName(L"name");
			if (xentityDataName)
			{
				element->removeChild(xentityDataName);
				changes++;
			}
		}

		if (
			typeName == L"traktor.mesh.BlendMeshEntityData" ||
			typeName == L"traktor.mesh.IndoorMeshEntityData" ||
			typeName == L"traktor.mesh.InstanceMeshEntityData" ||
			typeName == L"traktor.mesh.SkinnedMeshEntityData" ||
			typeName == L"traktor.mesh.StaticMeshEntityData" ||
			typeName == L"traktor.mesh.StreamMeshEntityData"
		)
		{
			element->setAttribute(L"type", L"traktor.mesh.MeshEntityData");
			changes++;
		}

		if (typeName == L"traktor.world.EntityInstance")
		{
			int32_t version = parseString< int32_t >(element->getAttribute(L"version", L"0")->getValue());
			if (version <= 1)
			{
				Ref< xml::Element > xinstanceGuid = new xml::Element(L"guid");
				xinstanceGuid->addChild(new xml::Text(Guid::create().format()));
				element->insertBefore(xinstanceGuid, 0);

				if (version <= 0)
				{
					Ref< xml::Element > xinstanceData = new xml::Element(L"instanceData");
					element->insertAfter(xinstanceData, element->getChildElementByName(L"entityData"));
				}

				element->setAttribute(L"version", L"2");
				changes++;
			}
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
