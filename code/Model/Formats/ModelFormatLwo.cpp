#include <algorithm>
#include <limits>
#include <cstdio>

extern "C"
{
	using std::FILE;
	#include "lwo2.h"
}

#include "Model/Formats/ModelFormatLwo.h"
#include "Model/Model.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Io/Path.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

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

std::wstring fixTextureFileName(const Path& texturePath, const std::wstring& fileName)
{
	size_t pos = fileName.find(L':');
	if (pos != std::wstring::npos)
		return fileName.substr(0, pos + 1) + L'/' + fileName.substr(pos + 1);
	else
		return texturePath.getPathName() + fileName;
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

bool createMaterials(const lwObject* lwo, Model* outModel)
{
	for (const lwSurface* surface = lwo->surf; surface; surface = surface->next)
	{
		Material material;
		material.setName(mbstows(surface->name));

		const lwTexture* texDiffuse = getLwTexture(surface->color.tex);
		if (texDiffuse)
		{
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

				std::wstring textureName = mbstows(clip->source.still.name);
				material.setDiffuseMap(textureName, diffuseBlendOperator);
			}
			else
				log::debug << L"No diffuse texture clip for surface \"" << material.getName() << L"\"" << Endl;
		}

		const lwTexture* texSpecular = getLwTexture(surface->specularity.tex);
		if (texSpecular)
		{
			const lwClip* clip = findLwClip(lwo, texSpecular->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = mbstows(clip->source.still.name);
				material.setSpecularMap(textureName);
			}
			else
				log::debug << L"No specular texture clip for surface \"" << mbstows(surface->name) << L"\"" << Endl;
		}

		const lwTexture* texBump = getLwTexture(surface->bump.tex);
		if (texBump)
		{
			const lwClip* clip = findLwClip(lwo, texBump->param.imap.cindex);
			if (clip)
			{
				std::wstring textureName = mbstows(clip->source.still.name);
				material.setNormalMap(textureName);
			}
			else
				log::debug << L"No bump texture clip for surface \"" << mbstows(surface->name) << L"\"" << Endl;
		}

		material.setColor(Color(
			uint8_t(surface->color.rgb[0] * 255.0f),
			uint8_t(surface->color.rgb[1] * 255.0f),
			uint8_t(surface->color.rgb[2] * 255.0f),
			uint8_t(surface->transparency.val.val * 255.0f)
		));

		material.setDiffuseTerm(surface->diffuse.val);
		material.setSpecularTerm(surface->specularity.val);
		material.setSpecularTerm(surface->glossiness.val);

		if ((surface->sideflags & 3) == 3)
			material.setDoubleSided(true);

		outModel->addMaterial(material);
	}

	return true;
}

bool createMesh(const lwObject* lwo, Model* outModel)
{
	uint32_t positionBase;

	// Convert positions.
	positionBase = 0;
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
		positionBase += layer->point.count;
	}

	// Convert blend targets.
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

	// Convert polygons.
	positionBase = 0;
	for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
	{
		for (int i = 0; i < layer->polygon.count; ++i)
		{
			const lwPolygon* pol = layer->polygon.pol + i;

			// Ignore all polygons which aren't plain faces.
			if (pol->type != ID_FACE)
				continue;

			const lwSurface* surf = pol->surf;
			const lwTexture* tex = getLwTexture(surf->color.tex);

			int materialIndex = 0;
			for (lwSurface* s = lwo->surf; s && s != surf; s = s->next)
				materialIndex++;

			int normal = outModel->addUniqueNormal(
				Vector4(
					pol->norm[0],
					pol->norm[1],
					pol->norm[2]
				)
			);

			Polygon polygon;
			polygon.setMaterial(materialIndex);

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
				}

				// UV maps.
				if (tex)
				{
					const lwVMapPt* vpt = findLwVMapPt(pol->v[j].vm, pol->v[j].nvmaps, tex->param.imap.vmap_name);
					if (!vpt)
						vpt = findLwVMapPt(pnt->vm, pnt->nvmaps, tex->param.imap.vmap_name);

					if (vpt)
					{
						float u = vpt->vmap->val[vpt->index][0];
						float v = vpt->vmap->val[vpt->index][1];

						vertex.setTexCoord(outModel->addUniqueTexCoord(Vector2(
							u,
							1.0f - v
						)));
					}
					else
						log::warning << L"Vertex " << j << L" doesn't exist in UV map \"" << mbstows(tex->param.imap.vmap_name) << L"\"" << Endl;
				}

				// Convert weight maps into bones and influences.
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
						int boneIndex = outModel->addBone(mbstows(i->vmap->name));
						float boneInfluence = i->vmap->val[i->index][0];
						vertex.setBoneInfluence(boneIndex, boneInfluence);
					}
				}

				polygon.addVertex(outModel->addUniqueVertex(vertex));
			}

			outModel->addPolygon(polygon);
		}

		positionBase += layer->point.count;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.model.ModelFormatLwo", ModelFormatLwo, ModelFormat)

void ModelFormatLwo::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Lightwave Object";
	outExtensions.push_back(L"lwo");
	outExtensions.push_back(L"lw");
}

bool ModelFormatLwo::supportFormat(const Path& filePath) const
{
	return 
		compareIgnoreCase(filePath.getExtension(), L"lwo") == 0 ||
		compareIgnoreCase(filePath.getExtension(), L"lw") == 0;
}

Model* ModelFormatLwo::read(const Path& filePath, uint32_t importFlags) const
{
#if defined(_WIN32)
	std::string fileNameTmp = wstombs(filePath.getPathName());
#else
	std::string fileNameTmp = wstombs(filePath.getPathNameNoVolume());
#endif

	lwObject* lwo = lwGetObject(const_cast< char* >(fileNameTmp.c_str()), 0, 0);
	if (!lwo)
	{
		log::error << L"Unable to open Lightwave object \"" << filePath.getPathName() << L"\"" << Endl;
		return 0;
	}

	Ref< Model > md = gc_new< Model >();

	if (importFlags & IfMaterials)
	{
		if (!createMaterials(lwo, md))
			return 0;
	}

	if (importFlags & IfMesh)
	{
		if (!createMesh(lwo, md))
			return 0;
	}

	lwFreeObject(lwo);

	return md;
}

bool ModelFormatLwo::write(const Path& filePath, const Model* model) const
{
	return false;
}

	}
}
