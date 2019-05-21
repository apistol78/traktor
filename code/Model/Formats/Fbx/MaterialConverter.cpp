#include "Core/Io/Path.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/MaterialConverter.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const FbxTexture* getTexture(const FbxSurfaceMaterial* material, const char* fbxPropertyName)
{
	if (!material || !fbxPropertyName)
		return nullptr;

	const FbxProperty prop = material->FindProperty(fbxPropertyName);
	if (!prop.IsValid())
		return nullptr;

	int fileTextureCount = prop.GetSrcObjectCount< FbxFileTexture >();
	for (int i = 0; i < fileTextureCount; ++i)
	{
		FbxFileTexture* fileTexture = prop.GetSrcObject< FbxFileTexture >(i);
		if (fileTexture)
			return fileTexture;
	}

	int layeredTextureCount = prop.GetSrcObjectCount< FbxLayeredTexture >();
	if (layeredTextureCount)
	{
		for (int i = 0; i < layeredTextureCount; ++i)
		{
			FbxLayeredTexture* layeredTexture = prop.GetSrcObject< FbxLayeredTexture >(i);
			if (layeredTexture)
				return layeredTexture;
		}
	}

	return nullptr;
}

std::wstring getTextureName(const FbxTexture* texture)
{
	const FbxFileTexture* fileTexture = FbxCast< const FbxFileTexture >(texture);
	if (fileTexture)
	{
		const Path texturePath(mbstows(fileTexture->GetFileName()));
		return texturePath.getFileNameNoExtension();
	}
	else
		return std::wstring(mbstows(texture->GetName()));
}

uint32_t uvChannel(Model& outModel, const std::string& uvSet)
{
	// In case "default" uv set requested then assume first but do NOT add to inoutChannels
	// in order to when first named UVSet is extracted it will get 0 index.
	if (uvSet != "default")
		return outModel.addUniqueTexCoordChannel(mbstows(uvSet));
	else
		return 0;
}

		}

bool convertMaterials(Model& outModel, std::map< int32_t, int32_t >& outMaterialMap, FbxNode* meshNode)
{
	int32_t materialCount = meshNode->GetMaterialCount();
	for (int32_t i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* material = meshNode->GetMaterial(i);
		if (!material)
			continue;

		Material mm;
		mm.setName(mbstows(material->GetName()));

		FbxProperty prop = meshNode->GetFirstProperty();
		while (prop.IsValid())
		{
			int userTag = prop.GetUserTag();
			std::wstring propName = mbstows(prop.GetNameAsCStr());
			if (startsWith< std::wstring >(propName, L"DEA_"))
			{
				propName = replaceAll< std::wstring >(propName, L"DEA_", L"");
				FbxPropertyT< FbxBool > propState = prop;
				if (propState.IsValid())
				{
					bool propValue = propState.Get();
					mm.setProperty< PropertyBoolean >(propName, propValue);
				}
			}
			prop = meshNode->GetNextProperty(prop);
		}

		// \note In case weird FBX show up with un-standard texture names.
#if 0
		{
			FbxProperty prop = material->GetFirstProperty();
			while (prop.IsValid())
			{
				int userTag = prop.GetUserTag();
				std::wstring propName = mbstows(prop.GetNameAsCStr());

				log::info << L"\"" << propName << L"\"" << Endl;

				const FbxTexture* texture = getTexture(material, prop.GetNameAsCStr());
				if (texture)
					log::info << L"\t as texture \"" << getTextureName(texture) << L"\"" << Endl;

				prop = material->GetNextProperty(prop);
			}
		}
#endif

		const FbxTexture* diffuseTexture = getTexture(material, FbxSurfaceMaterial::sDiffuse);
		if (diffuseTexture)
		{
			uint32_t channel = uvChannel(outModel, diffuseTexture->UVSet.Get().Buffer());
			mm.setDiffuseMap(Material::Map(getTextureName(diffuseTexture), channel, true));
		}

		const FbxTexture* specularTexture = getTexture(material, FbxSurfaceMaterial::sSpecular);
		if (specularTexture)
		{
			uint32_t channel = uvChannel(outModel, specularTexture->UVSet.Get().Buffer());
			mm.setSpecularMap(Material::Map(getTextureName(specularTexture), channel, false));
		}

		const FbxTexture* shininessTexture = getTexture(material, FbxSurfaceMaterial::sShininess);
		if (shininessTexture)
		{
			uint32_t channel = uvChannel(outModel, shininessTexture->UVSet.Get().Buffer());
			mm.setRoughnessMap(Material::Map(getTextureName(shininessTexture), channel, false));
		}

		const FbxTexture* reflectionFactorTexture = getTexture(material, FbxSurfaceMaterial::sReflectionFactor);
		if (reflectionFactorTexture)
		{
			uint32_t channel = uvChannel(outModel, reflectionFactorTexture->UVSet.Get().Buffer());
			mm.setMetalnessMap(Material::Map(getTextureName(reflectionFactorTexture), channel, false));
		}

		const FbxTexture* normalTexture = getTexture(material, FbxSurfaceMaterial::sNormalMap);
		if (normalTexture)
		{
			uint32_t channel = uvChannel(outModel, normalTexture->UVSet.Get().Buffer());
			mm.setNormalMap(Material::Map(getTextureName(normalTexture), channel, false));
		}

		const FbxTexture* transparencyTexture = getTexture(material, FbxSurfaceMaterial::sTransparentColor);
		if (transparencyTexture)
		{
			uint32_t channel = uvChannel(outModel, transparencyTexture->UVSet.Get().Buffer());
			mm.setTransparencyMap(Material::Map(getTextureName(transparencyTexture), channel, false));
			mm.setBlendOperator(Material::BoAlpha);
		}

		const FbxTexture* emissiveTexture = getTexture(material, /*mayaExported ? FbxSurfaceMaterial::sAmbient :*/ FbxSurfaceMaterial::sEmissive);
		if (emissiveTexture)
		{
			uint32_t channel = uvChannel(outModel, emissiveTexture->UVSet.Get().Buffer());
			mm.setEmissiveMap(Material::Map(getTextureName(emissiveTexture), channel, false));
		}

		if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phongMaterial = (FbxSurfacePhong*)material;

			if (diffuseTexture == nullptr)
			{
				FbxPropertyT< FbxDouble3 > phongDiffuse = phongMaterial->Diffuse;
				if (phongDiffuse.IsValid())
				{
					FbxDouble3 diffuse = phongDiffuse.Get();
					mm.setColor(Color4ub(
						uint8_t(diffuse[0] * 255),
						uint8_t(diffuse[1] * 255),
						uint8_t(diffuse[2] * 255),
						255
					));
				}
			}

			FbxPropertyT< FbxDouble > phongDiffuseFactor = phongMaterial->DiffuseFactor;
			if (phongDiffuseFactor.IsValid())
			{
				FbxDouble diffuseFactor = phongDiffuseFactor.Get();
				mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
			}

			FbxPropertyT< FbxDouble > phongSpecularFactor = phongMaterial->SpecularFactor;
			if (phongSpecularFactor.IsValid())
			{
				FbxDouble specularFactor = phongSpecularFactor.Get() * 2.0f;
				mm.setSpecularTerm(clamp(float(specularFactor), 0.0f, 1.0f));
			}

			FbxPropertyT< FbxDouble > phongShininess = phongMaterial->Shininess;
			if (phongShininess.IsValid())
			{
				FbxDouble shininess = phongShininess.Get();
				float roughness = std::pow(1.0f - shininess / 100.0f, 2.0f);
				mm.setRoughness(clamp(roughness, 0.0f, 1.0f));
			}

			FbxPropertyT< FbxDouble > reflectionFactor = phongMaterial->ReflectionFactor;
			if (reflectionFactor.IsValid())
			{
				FbxDouble reflection = reflectionFactor.Get();
				mm.setMetalness(clamp(float(reflection), 0.0f, 1.0f));
			}

			FbxPropertyT< FbxDouble3 > phongEmissive = /*mayaExported ? phongMaterial->Ambient :*/ phongMaterial->Emissive;
			if (phongEmissive.IsValid())
			{
				FbxDouble3 emissive = phongEmissive.Get();
				FbxDouble emissiveFactor = /*mayaExported ? phongMaterial->AmbientFactor.Get() :*/ phongMaterial->EmissiveFactor.Get();

				emissive[0] *= emissiveFactor;
				emissive[1] *= emissiveFactor;
				emissive[2] *= emissiveFactor;

				mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
			}
		}
		else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			FbxSurfaceLambert* lambertMaterial = (FbxSurfaceLambert*)material;

			if (diffuseTexture == nullptr)
			{
				FbxPropertyT< FbxDouble3 > lambertDiffuse = lambertMaterial->Diffuse;
				if (lambertDiffuse.IsValid())
				{
					FbxDouble3 diffuse = lambertDiffuse.Get();
					mm.setColor(Color4ub(
						uint8_t(diffuse[0] * 255),
						uint8_t(diffuse[1] * 255),
						uint8_t(diffuse[2] * 255),
						255
					));
				}
			}
			
			FbxPropertyT< FbxDouble > lambertDiffuseFactor = lambertMaterial->DiffuseFactor;
			if (lambertDiffuseFactor.IsValid())
			{
				FbxDouble diffuseFactor = lambertDiffuseFactor.Get();
				mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
			}

			FbxPropertyT< FbxDouble3 > lambertEmissive = /*mayaExported ? lambertMaterial->Ambient :*/ lambertMaterial->Emissive;
			if (lambertEmissive.IsValid())
			{
				FbxDouble3 emissive = lambertEmissive.Get();
				FbxDouble emissiveFactor = /*mayaExported ? lambertMaterial->AmbientFactor.Get() :*/ lambertMaterial->EmissiveFactor.Get();

				emissive[0] *= emissiveFactor;
				emissive[1] *= emissiveFactor;
				emissive[2] *= emissiveFactor;

				mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
			}

			mm.setSpecularTerm(0.0f);
		}

		outMaterialMap[i] = outModel.addUniqueMaterial(mm);
	}

	return true;
}

	}
}
