#include "ResourceMesh.h"
#include "Application.h"
#include "ModuleMeshes.h"

// ---------------------------------------------------------
ResourceMesh::ResourceMesh(UID uid) : Resource(uid, Resource::Type::mesh)
{} 

// ---------------------------------------------------------
ResourceMesh::~ResourceMesh()
{
	// TODO: deallocate opengl buffers
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(colors);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(texture_coords);
}

// ---------------------------------------------------------
bool ResourceMesh::LoadInMemory()
{
	return App->meshes->Load(this);
}

// ---------------------------------------------------------
void ResourceMesh::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceMesh::Load(const Config & config)
{
	Resource::Load(config);
}

// ---------------------------------------------------------
bool ResourceMesh::Save(string& output) const
{
    string data;

    // \todo: reserve string data

    stringstream write_string(data);

    write_stream << name.C_str();
    write_stream << mat_id;
    write_stream << vertex_size;
    write_stream << texcoord_offset;
    write_stream << normal_offset;
    write_stream << tangent_offset;
    write_stream << bone_weight_offset;
    write_stream << num_vertices;

    write_stream << num_indices;

	return App->fs->SaveUnique(output, data.c_str(), data.size(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
}

// ---------------------------------------------------------
UID ResourceMesh::Import(const aiMesh* mesh, UID mat_id, const char* source_file)
{
    ResourceMaterial* m = static_cast<ResourceMesh*>(App->resources->CreateNewResource(Resource::mesh));

    m->name     = mesh->mName.length > 0 ? std::string(mesh->mName.C_Str()) : std::string();
    m->material = mat_id;

    m->GenerateAttribInfo(orig);
    m->GenerateCPUBuffers(orig);

    if((m->attribs & ATTRIB_BONES) != 0)
    {
        m->GenerateBoneData(orig);
    }

    m->GenerateVBO(false);
    m->GenerateVAO();

    std::string output;

    if(m->Save(output))
    {
		if (source_file != nullptr) 
        {
			m->file = source_file;
			App->fs->NormalizePath(m->file);
		}

		string file;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file);
		m->exported_file = file;

		LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());

        return m->uid;
    }

    // \todo: remove resource

    LOG("Importing of BUFFER aiMaterial [%s] FAILED", source_file);

    return 0;
}

void ResourceMesh::GenerateAttribInfo(const aiMesh* mesh)
{
    vertex_size         = sizeof(math::float3);
    attribs             = 0;
    texcoord_offset     = 0;
    normal_offset       = 0;
    tangent_offset      = 0;
    bone_weight_offset  = 0;

    if(mesh->HasNormals())
    {
        attribs |= ATTRIB_NORMALS;
        normal_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(math::float3);
    }

    if(mesh->HasTextureCoords(0))
    {
        attribs |= ATTRIB_TEX_COORDS_0;
        texcoord_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(aiVector2D);
    }

    if(mesh->HasTangentsAndBitangents())
    {
        attribs |= ATTRIB_TANGENTS;
        tangent_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(math::float3);
    }

    if(mesh->HasBones())
    {
        attribs |= ATTRIB_BONES;
        bone_idx_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(unsigned)*4;
        bone_weight_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(float)*4;
    }
}

void ResourceMesh::GenerateCPUBuffers(const aiMesh* mesh)
{
    num_vertices = mesh->mNumVertices;
    src_vertices = new float3[mesh->mNumVertices];

    for(unsigned i=0; i< mesh->mNumVertices; ++i)
    {
        src_vertices[i] = *((float3*)&mesh->mVertices[i]);

        for(unsigned j=0; j<3; ++j)
        {
            min[j] = std::min(min[j], src_vertices[i][j]);
            max[j] = std::max(max[j], src_vertices[i][j]);
        }
    }

    if(mesh->HasTextureCoords(0))
    {
        src_texcoord0 = new float2[mesh->mNumVertices];

        for(unsigned i=0; i < mesh->mNumVertices; ++i) 
        {
            src_texcoord0[i] = float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
    }

    if(mesh->HasNormals())
    {
        src_normals = new float3[mesh->mNumVertices];
        memcpy(src_normals, mesh->mNormals, sizeof(float3)*mesh->mNumVertices);
    }

    src_indices = new unsigned[mesh->mNumFaces*3];
    num_indices = mesh->mNumFaces*3;

    for(unsigned j=0; j < mesh->mNumFaces; ++j)
    {
        const aiFace& face = mesh->mFaces[j];

        assert(face.mNumIndices == 3);

        src_indices[j * 3] = face.mIndices[0];
        src_indices[j * 3 + 1] = face.mIndices[1];
        src_indices[j * 3 + 2] = face.mIndices[2];
    }

    if(mesh->HasTangentsAndBitangents())
    {
        // uncomment iif copy form assimp
        src_tangents = new math::float3[mesh->mNumVertices];
        memcpy(src_tangents, mesh->mTangents, sizeof(float3)*mesh->mNumVertices);
        //GenerateTangentSpace(dst);
    }
}

void ResourceMesh::GenerateVBO(bool dynamic)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(math::float3)*num_vertices, src_vertices);

    if((attribs & ATTRIB_TEX_COORDS_0) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, texcoord_offset, sizeof(float2)*num_vertices, src_texcoord0);
    }

    if((attribs & ATTRIB_NORMALS) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, normal_offset, sizeof(float3)*num_vertices, src_normals);
    }

    if((attribs & ATTRIB_TANGENTS) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, tangent_offset, sizeof(float3)*num_vertices, src_tangents);
    }

    if((attribs & ATTRIB_BONES) != 0)
    {
        unsigned* bone_indices = (unsigned*)glMapBufferRange(GL_ARRAY_BUFFER, bone_idx_offset, 
                                                             (sizeof(unsigned)*4+sizeof(float)*4)*num_vertices, 
                                                             GL_MAP_WRITE_BIT);
        float* bone_weights    = (float*)(bone_indices+num_vertices*4);

        for(unsigned i=0; i < num_vertices*4; ++i) 
        {
            bone_indices[i] = 0;
            bone_weights[i] = 0.0f;
        }

        for(unsigned i=0; i< num_bones; ++i)
        {
            const Bone& bone = bones[i];

            for(unsigned j=0; j < bone.num_weights; ++j)
            {
                unsigned index = bone.weights[j].vertex;
                float weight   = bone.weights[j].weight;

                unsigned* bone_idx = &bone_indices[index*4];
                float* bone_weight = &bone_weights[index*4];

                assert(bone_weight[3] == 0.0f);
                for(unsigned l=0; l < 4; ++l)
                {
                    if(bone_weight[l] == 0.0f)
                    {
                        bone_idx[l] = i;
                        bone_weight[l] = weight;

                        break;
                    }
                }
            }
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(unsigned), src_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ResourceMesh::GenerateBoneData(const aiMesh* mesh)
{
    assert((attribs & ATTRIB_BONES) != 0);

    bones      = new Bone[mesh->mNumBones];
    num_bones  = mesh->mNumBones;

    palette    = new float4x4[num_bones];

    for(unsigned i=0; i< mesh->mNumBones; ++i)
    {
        const aiBone* bone   = mesh->mBones[i];
        Bone& dst_bone       = bones[i];

        dst_bone.name 	     = HashString(bone->mName.C_Str());
        dst_bone.weights     = new Weight[bone->mNumWeights];
        dst_bone.num_weights = bone->mNumWeights;
        dst_bone.bind 	     = float4x4(float4(bone->mOffsetMatrix.a1, bone->mOffsetMatrix.b1, bone->mOffsetMatrix.c1, bone->mOffsetMatrix.d1),
								        float4(bone->mOffsetMatrix.a2, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.d2),
                                        float4(bone->mOffsetMatrix.a3, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.d3),
                                        float4(bone->mOffsetMatrix.a4, bone->mOffsetMatrix.b4, bone->mOffsetMatrix.c4, bone->mOffsetMatrix.d4));

        for(unsigned j=0; j < bone->mNumWeights; ++j)
        {
            dst_bone.weights[j].vertex = bone->mWeights[j].mVertexId;
            dst_bone.weights[j].weight = bone->mWeights[j].mWeight;
        }
    }

}

void ResourceMesh::GenerateVAO()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)0);

	if ((attribs & ATTRIB_NORMALS) != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)(normal_offset));
	}

	if ((attribs & ATTRIB_TEX_COORDS_0) != 0)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(aiVector2D), (void*)(texcoord_offset));
	}

	if((attribs & ATTRIB_BONES) != 0)
	{
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(unsigned)*4, (void*)bone_idx_offset);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)bone_weight_offset);
	}

    if((attribs & ATTRIB_TANGENTS) != 0)
    {
		glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)(tangent_offset));
    }

    glBindVertexArray(0);
}

