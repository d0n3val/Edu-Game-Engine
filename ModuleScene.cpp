#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include <gl/GL.h>
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc130-mt.lib")

using namespace std;

ModuleScene::ModuleScene( bool start_enabled) : Module( start_enabled)
{}

// Destructor
ModuleScene::~ModuleScene()
{}

// Called before render is available
bool ModuleScene::Init()
{
	LOG("Loading Scene Manager");
	bool ret = true;
	struct aiLogStream stream;

	// Stream log messages to Debug window
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return ret;
}

bool ModuleScene::LoadScene(const char* file)
{
	if (scene != nullptr) // Unload all textures ?
		aiReleaseImport(scene);

	scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	// Load textures
	if (scene != nullptr)
	{
		for (uint i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* material = scene->mMaterials[i];
			uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
			aiString path;

			for (uint k = 0; k < numTextures; ++k)
			{
				material->GetTexture(aiTextureType_DIFFUSE, k, &path);

				std::string texPath(file);
				size_t pos = texPath.find_last_of("\\/");
				if (pos != string::npos)
					texPath.erase(pos + 1, string::npos);

				App->tex->Load(path.C_Str(), texPath.c_str());
			}
		}
	}

	return scene != nullptr;
}

void ModuleScene::Draw() const
{
	if(scene != nullptr)
		RecursiveDraw(scene->mRootNode);
}

void ModuleScene::RecursiveDraw(const struct aiNode* node) const
{
	aiMatrix4x4 transform = node->mTransformation;

	// push this matrix before drawing
	aiTransposeMatrix4(&transform);
	glPushMatrix();
	glMultMatrixf((float*)&transform);

	// iterate all meches in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		PrepareMaterial(scene->mMaterials[mesh->mMaterialIndex]);

		// iterate all faces
		for (uint k = 0; k < mesh->mNumFaces; ++k)
		{
			const aiFace* face = &mesh->mFaces[k];

			mesh->mNormals ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
			mesh->mColors[0] ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);

			glBegin(GL_POLYGON);

			// iterate all indices
			for (uint j = 0; j < face->mNumIndices; ++j)
			{
				int index = face->mIndices[j];

				if(mesh->HasTextureCoords(0))
					glTexCoord2f(mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y);

				if(mesh->mNormals != NULL) 
					glNormal3fv(&mesh->mNormals[index].x);

				glVertex3fv(&mesh->mVertices[index].x);
			}

			glEnd();
		}
	}

	// recursive call to draw the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursiveDraw(node->mChildren[i]);
	}

	// pop this matrix before leaving this node
	glPopMatrix();
}

void ModuleScene::PrepareMaterial(const aiMaterial* material) const
{
	aiString texPath;

	material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);

	GLuint id = App->tex->GetId(texPath.C_Str());
	glBindTexture(GL_TEXTURE_2D, id);
}

// Called before quitting
bool ModuleScene::CleanUp()
{
	LOG("Freeing Scene Manager");

	// Clean scene data
	if(scene != nullptr)	// Unload Textures ?
		aiReleaseImport(scene);

	// detach log stream
	aiDetachAllLogStreams();

	return true;
}
