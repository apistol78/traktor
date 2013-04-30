#include <algorithm>
#include <limits>
#include <cstdio>

extern "C"
{
	using std::FILE;
	#include "lwo2.h"
}

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatLwo.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

#define ID_WGHT	LWID_('W','G','H','T')
#define ID_SPOT	LWID_('S','P','O','T')	// Absolute
#define ID_MORF	LWID_('M','O','R','F')	// Relative
#define ID_RGBA LWID_('R','G','B','A')	// Vertex color map.
#define ID_RGB  LWID_('R','G','B',' ')	// Vertex color map (no alpha).

Semaphore s_lock;	//< LWO parser isn't thread-safe; it's having global variables for tracking IO bytes.

const lwClip* findLwClip(const lwObject* lwo, int clipIndex)
{
	const lwClip* clip = lwo->clip;
	while (clip && clip->index != clipIndex)
		clip = clip->next;
	return clip;
}

const lwTexture* getLwTexture(const lwTexture* tex)
{
	for (; tex; tex = tex->next)
	{
		if (tex->type == ID_IMAP && tex->param.imap.vmap_name)
			break;
	}
	return tex;
}

std::wstring fixTextureFileName(const std::wstring& fileName)
{
	std::wstring textureName = fileName;
	size_t pos;

	pos = textureName.find_last_of(L'/');
	if (pos != std::wstring::npos)
		textureName = textureName.substr(pos + 1);

	pos = textureName.find(L'.');
	if (pos != std::wstring::npos)
		textureName = textureName.substr(0, pos);

	return textureName;
}

const lwVMapPt* findLwVMapPt(const lwVMapPt* vmaps, int nvmaps, const std::string& vmapName)
{
	for (int i = 0; i < nvmaps; ++i)
	{
		if (std::string(vmaps[i].vmap->name) == vmapName)
			return &vmaps[i];
	}
	return 0;
}

uint32_t uvChannel(std::vector< std::string >& inoutChannels, const std::string vmap)
{
	std::vector< std::string >::iterator i = std::find(inoutChannels.begin(), inoutChannels.end(), vmap);
	if (i != inoutChannels.end())
		return std::distance(inoutChannels.begin(), i);

	uint32_t channel = uint32_t(inoutChannels.size());
	inoutChannels.push_back(vmap);

	return channel;
}

bool createMaterials(const lwObject* lwo, Model* outModel, std::vector< std::string >& outChannels)
{
	uint32_t channel = 0;

	for (const lwSurface* surface = lwo->surf; surface; surface = surface->next)
	{
		Material material;
		material.setName(mbstows(surface->name));

		const lwTexture* texDiffuse = getLwTexture(surface->color.tex);
		if (texDiffuse)
		{
			uint32_t channel = uvChannel(outChannels, texDiffuse->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texDiffuse->param.imap.cindex);
			if (clip)
			{
				Material::BlendOperator diffuseBlendOperator = Material::BoDecal;
				switch (surface->color.tex->opac_type)
				{
				case 0:
					diffuseBlendOperator = Material::BoDecal;
					break;
				case 3:
					diffuseBlendOperator = Material::BoMultiply;
					break;
				case 5:
					diffuseBlendOperator = Material::BoAlpha;
					break;
				case 7:
					diffuseBlendOperator = Material::BoAdd;
					break;
				default:
					log::warning << L"Unknown opacity type (" << surface->color.tex->opac_type << L") on surface \"" << material.getName() << L"\"" << Endl;
				}

				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setDiffuseMap(Material::Map(textureName, channel, true));
				material.setBlendOperator(diffuseBlendOperator);
			}
			else
				T_DEBUG(L"No diffuse texture clip for surface \"" << material.getName() << L"\"");
		}

		const lwTexture* texSpecular = getLwTexture(surface->specularity.tex);
		if (texSpecular)
		{
			uint32_t channel = uvChannel(outChannels, texSpecular->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texSpecular->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setSpecularMap(Material::Map(textureName, channel, false));
			}
			else
				T_DEBUG(L"No specular texture clip for surface \"" << mbstows(surface->name) << L"\"");
		}

		const lwTexture* texTransparency = getLwTexture(surface->transparency.val.tex);
		if (texTransparency)
		{
			uint32_t channel = uvChannel(outChannels, texTransparency->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texTransparency->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setTransparencyMap(Material::Map(textureName, channel, false));
			}
			else
				T_DEBUG(L"No transparency texture clip for surface \"" << mbstows(surface->name) << L"\"");

			material.setBlendOperator(Material::BoAlpha);
		}

		const lwTexture* texEmissive = getLwTexture(surface->luminosity.tex);
		if (texEmissive)
		{
			uint32_t channel = uvChannel(outChannels, texEmissive->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texEmissive->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setEmissiveMap(Material::Map(textureName, channel, false));
			}
			else
				T_DEBUG(L"No emissive texture clip for surface \"" << mbstows(surface->name) << L"\"");
		}

		const lwTexture* texReflective = getLwTexture(surface->reflection.val.tex);
		if (texReflective)
		{
			uint32_t channel = uvChannel(outChannels, texReflective->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texReflective->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setReflectiveMap(Material::Map(textureName, channel, false));
			}
			else
				T_DEBUG(L"No reflective texture clip for surface \"" << mbstows(surface->name) << L"\"");
		}

		const lwTexture* texBump = getLwTexture(surface->bump.tex);
		if (texBump)
		{
			uint32_t channel = uvChannel(outChannels, texBump->param.imap.vmap_name);

			const lwClip* clip = findLwClip(lwo, texBump->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
				material.setNormalMap(Material::Map(textureName, channel, false));
			}
			else
				T_DEBUG(L"No bump texture clip for surface \"" << mbstows(surface->name) << L"\"");
		}

		material.setColor(Color4ub(
			uint8_t(surface->color.rgb[0] * 255.0f),
			uint8_t(surface->color.rgb[1] * 255.0f),
			uint8_t(surface->color.rgb[2] * 255.0f),
			uint8_t((1.0f - surface->transparency.val.val) * 255.0f)
		));

		if (surface->transparency.val.val >= FUZZY_EPSILON)
		{
			material.setTransparency(surface->transparency.val.val);
			material.setBlendOperator(Material::BoAlpha);
		}

		material.setDiffuseTerm(surface->diffuse.val);
		material.setSpecularTerm(surface->specularity.val);
		material.setSpecularRoughness(surface->glossiness.val);
		material.setEmissive(surface->luminosity.val);
		material.setReflective(surface->reflection.val.val);
		material.setRimLightIntensity(surface->glow.val);

		if ((surface->sideflags & 3) == 3)
			material.setDoubleSided(true);

		if (surface->comment)
			material.setProperty< PropertyBoolean >(mbstows(surface->comment), true);

		outModel->addMaterial(material);
	}

	return true;
}

bool createMesh(const lwObject* lwo, Model* outModel, std::vector< std::string >& inoutChannels, uint32_t importFlags)
{
	uint32_t pointCount;
	uint32_t polygonCount;
	uint32_t positionBase;

	// Count number of primitives.
	pointCount = polygonCount = 0;
	for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
	{
		pointCount += layer->point.count;
		polygonCount += layer->polygon.count;
	}

	// Convert positions.
	if (importFlags & ModelFormat::IfMeshPositions)
	{
		outModel->reservePositions(pointCount);
		for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
		{
			for (int i = 0; i < layer->point.count; ++i)
			{
				lwPoint* pnt = layer->point.pt + i;
				Vector4 position(
					pnt->pos[0],
					pnt->pos[1],
					pnt->pos[2],
					1.0f
				);
				outModel->addPosition(position);
			}
		}
	}

	// Convert blend targets.
	if (importFlags & ModelFormat::IfMeshBlendTargets)
	{
		positionBase = 0;
		for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
		{
			for (int i = 0; i < layer->point.count; ++i)
			{
				lwPoint* pnt = layer->point.pt + i;
				for (int j = 0; j < pnt->nvmaps; ++j)
				{
					if (pnt->vm[j].vmap->type == ID_SPOT || pnt->vm[j].vmap->type == ID_MORF)
					{
						uint32_t blendTargetIndex = outModel->addBlendTarget(mbstows(pnt->vm[j].vmap->name));

						Vector4 basePosition = outModel->getPosition(positionBase + i);
						Vector4 targetPosition(
							pnt->vm[j].vmap->val[pnt->vm[j].index][0],
							pnt->vm[j].vmap->val[pnt->vm[j].index][1],
							pnt->vm[j].vmap->val[pnt->vm[j].index][2],
							1.0f
						);
						if (pnt->vm[j].vmap->type == ID_MORF)
							targetPosition += basePosition.xyz0();

						outModel->setBlendTargetPosition(
							blendTargetIndex,
							positionBase + i,
							targetPosition
						);
					}
				}
			}
			positionBase += layer->point.count;
		}
	}

	// Convert polygons.
	if (importFlags & (ModelFormat::IfMeshPolygons | ModelFormat::IfMeshVertices))
	{
		positionBase = 0;

		if (importFlags & ModelFormat::IfMeshPolygons)
			outModel->reservePolygons(polygonCount);

		for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
		{
			for (int i = 0; i < layer->polygon.count; ++i)
			{
				const lwPolygon* pol = layer->polygon.pol + i;

				// Ignore all polygons which aren't plain faces.
				if (pol->type != ID_FACE)
					continue;

				const lwSurface* surf = pol->surf;
				T_ASSERT (surf);

				int materialIndex = 0;
				for (lwSurface* s = lwo->surf; s && s != surf; s = s->next)
					materialIndex++;

				Polygon polygon;
				polygon.setMaterial(materialIndex);
				polygon.setNormal(outModel->addUniqueNormal(Vector4(
					pol->norm[0],
					pol->norm[1],
					pol->norm[2]
				)));

				if (importFlags & ModelFormat::IfMeshVertices)
				{
					for (int32_t j = pol->nverts - 1; j >= 0; --j)
					{
						lwPoint* pnt = layer->point.pt + pol->v[j].index;

						Vertex vertex;
						vertex.setPosition(positionBase + pol->v[j].index);
						vertex.setNormal(outModel->addUniqueNormal(Vector4(
							pol->v[j].norm[0],
							pol->v[j].norm[1],
							pol->v[j].norm[2]
						)));

						// Vertex colors.
						for (int32_t k = 0; k < pnt->nvmaps; ++k)
						{
							const lwVMapPt* vc = &pnt->vm[k];
							if (vc->vmap->type == ID_RGBA)
							{
								const float* color = vc->vmap->val[vc->index];
								vertex.setColor(outModel->addUniqueColor(Vector4(
									color[0],
									color[1],
									color[2],
									color[3]
								)));
								break;
							}
							else if (vc->vmap->type == ID_RGB)
							{
								const float* color = vc->vmap->val[vc->index];
								vertex.setColor(outModel->addUniqueColor(Vector4(
									color[0],
									color[1],
									color[2],
									1.0f
								)));
								break;
							}
						}

						// UV maps.
						for (int32_t k = 0; k < pnt->nvmaps; ++k)
						{
							const lwVMapPt* vpt = &pnt->vm[k];
							uint32_t channel = uvChannel(inoutChannels, vpt->vmap->name);

							float u = vpt->vmap->val[vpt->index][0];
							float v = vpt->vmap->val[vpt->index][1];

							vertex.setTexCoord(channel, outModel->addUniqueTexCoord(Vector2(
								u,
								1.0f - v
							)));
						}

						// Convert weight maps into bones and influences.
						if (importFlags & ModelFormat::IfMeshBlendWeights)
						{
							// Collect weight map references for this point.
							std::vector< lwVMapPt > weightRefs;
							for (int i = 0; i < pnt->nvmaps; ++i)
							{
								if (pnt->vm[i].vmap->type == ID_WGHT)
									weightRefs.push_back(pnt->vm[i]);
							}

							// Add weights to vertex, also allocate bone index.
							for (std::vector< lwVMapPt >::iterator i = weightRefs.begin(); i != weightRefs.end(); ++i)
							{
								int jointIndex = outModel->addJoint(mbstows(i->vmap->name));
								float jointInfluence = i->vmap->val[i->index][0];
								vertex.setJointInfluence(jointIndex, jointInfluence);
							}
						}

						int32_t vertexId = outModel->addUniqueVertex(vertex);
						if (importFlags & ModelFormat::IfMeshPolygons)
							polygon.addVertex(vertexId);
					}
				}

				if (importFlags & ModelFormat::IfMeshPolygons)
					outModel->addPolygon(polygon);
			}

			positionBase += layer->point.count;
		}
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatLwo", 0, ModelFormatLwo, ModelFormat)

void ModelFormatLwo::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Lightwave Object";
	outExtensions.push_back(L"lwo");
	outExtensions.push_back(L"lw");
}

bool ModelFormatLwo::supportFormat(const std::wstring& extension) const
{
	return 
		compareIgnoreCase< std::wstring >(extension, L"lwo") == 0 ||
		compareIgnoreCase< std::wstring >(extension, L"lw") == 0;
}

Ref< Model > ModelFormatLwo::read(IStream* stream, uint32_t importFlags) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_lock);

#if defined(_WIN32)
	char* tmp = _tempnam("c:\\temp", "lwo");
#else
	char* tmp = tempnam("/tmp", "lwo");
#endif
	if (!tmp)
		return 0;

	Ref< IStream > tmpFile = FileSystem::getInstance().open(mbstows(tmp), File::FmWrite);
	if (!tmpFile)
		return 0;

	if (!StreamCopy(tmpFile, stream).execute())
		return 0;

	tmpFile->close();
	tmpFile = 0;

	lwObject* lwo = lwGetObject(tmp, 0, 0);
	FileSystem::getInstance().remove(mbstows(tmp));

	if (!lwo)
		return 0;

	Ref< Model > md = new Model();
	std::vector< std::string > channels;

	if (importFlags & IfMaterials)
	{
		if (!createMaterials(lwo, md, channels))
		{
			lwFreeObject(lwo);
			return 0;
		}
	}

	if (importFlags & IfMesh)
	{
		if (!createMesh(lwo, md, channels, importFlags))
		{
			lwFreeObject(lwo);
			return 0;
		}
	}

	lwFreeObject(lwo);

	return md;
}

bool ModelFormatLwo::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
