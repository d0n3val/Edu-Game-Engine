#include "ResourceMesh.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"

#include "OpenGL.h"
#include "DefaultShaderLocations.h"

#include "Assimp/include/mesh.h"
#include "utils/SimpleBinStream.h"
#include "thekla_atlas/src/thekla/thekla_atlas.h"

#pragma warning(push)
#pragma warning(disable : 4996)  
#pragma warning(disable : 4244)  
#pragma warning(disable : 4305)  
#pragma warning(disable : 4838)  

#define PAR_SHAPES_IMPLEMENTATION
#include "utils/par_shapes.h"
#pragma warning(pop)

#include "mmgr/mmgr.h"

using namespace Thekla;

namespace
{

    template<class T>
    void copy_new_vertices(T*& src, Atlas_Output_Vertex* vertices, uint count)
    {
        if(src != nullptr)
        {
            T* new_src = new T[count];
            for(uint i=0; i < count ; ++i)
            {
                new_src[i] = src[vertices[i].xref];
            }

            delete [] src;

            src = new_src;
        }
    }

}

// ---------------------------------------------------------
ResourceMesh::ResourceMesh(UID uid) : Resource(uid, Resource::Type::mesh)
{
	bbox.SetNegativeInfinity();
} 

// ---------------------------------------------------------
ResourceMesh::~ResourceMesh()
{
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
bool ResourceMesh::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;
        uint size = App->fs->Load(LIBRARY_MESH_FOLDER, GetExportedFile(), &buffer);

        if(size > 0)
        {
            simple::mem_istream<std::true_type> read_stream(buffer, size);
            std::string tmp;

            read_stream >> tmp;
            name = HashString(tmp.c_str());

            read_stream >> vertex_size;
            read_stream >> attribs;
            read_stream >> texcoord0_offset;
            read_stream >> normal_offset;
            read_stream >> tangent_offset;
            read_stream >> bone_idx_offset;
            read_stream >> bone_weight_offset;

            read_stream >> num_vertices;

            src_vertices = new float3[num_vertices];

            for(uint i=0; i< num_vertices; ++i)
            {
                read_stream >> src_vertices[i].x >> src_vertices[i].y >> src_vertices[i].z;
            }

            if((attribs & ATTRIB_TEX_COORDS_0) != 0)
            {
                src_texcoord0 = new float2[num_vertices];

                for(uint i=0; i< num_vertices; ++i)
                {
                    read_stream >> src_texcoord0[i].x >> src_texcoord0[i].y;
                }
            }

            if((attribs & ATTRIB_NORMALS) != 0)
            {
                src_normals = new float3[num_vertices];

                for(uint i=0; i< num_vertices; ++i)
                {
                    read_stream >> src_normals[i].x >> src_normals[i].y >> src_normals[i].z;
                }
            }

            if((attribs & ATTRIB_TANGENTS) != 0)
            {
                src_tangents = new float3[num_vertices];

                for(uint i=0; i< num_vertices; ++i)
                {
                    read_stream >> src_tangents[i].x >> src_tangents[i].y >> src_tangents[i].z;
                }
            }

            read_stream >> num_indices;
            src_indices = new unsigned[num_indices];

            for(uint i=0; i< num_indices; ++i)
            {
                read_stream >> src_indices[i];
            }

            if((attribs & ATTRIB_BONES) != 0)
            {
                read_stream >> num_bones;

                bones = new Bone[num_bones];

                for(uint i=0; i< num_bones; ++i)
                {
                    read_stream >> tmp;
                    bones[i].name = HashString(tmp.c_str());
                    for (uint j = 0; j < 16; ++j)
                    {
                        read_stream >> bones[i].bind.ptr()[j];
                    }
                }

                src_bone_indices = new unsigned[num_vertices*4];
                src_bone_weights = new float4[num_vertices];

                for(uint i=0; i< num_vertices; ++i)
                {
                    read_stream >> src_bone_indices[i*4+0] >> src_bone_indices[i*4+1] >> src_bone_indices[i*4+2] >> src_bone_indices[i*4+3];
                    read_stream >> src_bone_weights[i].x >> src_bone_weights[i].y >>src_bone_weights[i].z >> src_bone_weights[i].w;
                }
            }

            read_stream >> bbox.minPoint.x >> bbox.minPoint.y >> bbox.minPoint.z;
            read_stream >> bbox.maxPoint.x >> bbox.maxPoint.y >> bbox.maxPoint.z;

            GenerateVBO();
            GenerateVAO();

            delete [] buffer;

            return true;
        }
    }

	return false;
}

// ---------------------------------------------------------
void ResourceMesh::ReleaseFromMemory() 
{
    // \todo: check if merge with clear and check import and check module resources final release

    delete [] src_vertices;
    src_vertices = nullptr;

    delete [] src_texcoord0;
    src_texcoord0 = nullptr;

    delete [] src_normals;
    src_normals = nullptr;

    delete [] src_tangents;
    src_tangents = nullptr;

    delete [] src_indices;
    src_indices = nullptr;

    delete [] bones;
    bones = nullptr;

    delete [] src_bone_indices;
    src_bone_indices = nullptr;

    delete [] src_bone_weights;
    src_bone_weights = nullptr;

    if(vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if(ibo != 0)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }

    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

// ---------------------------------------------------------
bool ResourceMesh::Save() 
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    if(exported_file.length() > 0)
    {
		char full_path[250];

		sprintf_s(full_path, 250, "%s%s", LIBRARY_MESH_FOLDER, exported_file.c_str());

        return App->fs->Save(full_path, &data[0], data.size()) > 0;
    }

	std::string output;

	if(App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MESH_FOLDER, "mesh", "edumesh"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }

	return false;
}

// ---------------------------------------------------------
void ResourceMesh::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << name.C_str();
    write_stream << vertex_size;
    write_stream << attribs;
    write_stream << texcoord0_offset;
    write_stream << normal_offset;
    write_stream << tangent_offset;
	write_stream << bone_idx_offset;
    write_stream << bone_weight_offset;

    write_stream << num_vertices;

    for(uint i=0; i< num_vertices; ++i)
    {
        write_stream << src_vertices[i].x << src_vertices[i].y << src_vertices[i].z;
    }

    if((attribs & ATTRIB_TEX_COORDS_0) != 0)
    {
        for(uint i=0; i< num_vertices; ++i)
        {
            write_stream << src_texcoord0[i].x << src_texcoord0[i].y;
        }
    }

    if((attribs & ATTRIB_TEX_COORDS_1) != 0)
    {
        for(uint i=0; i< num_vertices; ++i)
        {
            write_stream << src_texcoord1[i].x << src_texcoord1[i].y;
        }
    }

    if((attribs & ATTRIB_NORMALS) != 0)
    {
        for(uint i=0; i< num_vertices; ++i)
        {
            write_stream << src_normals[i].x << src_normals[i].y << src_normals[i].z;
        }
    }

    if((attribs & ATTRIB_TANGENTS) != 0)
    {
        for(uint i=0; i< num_vertices; ++i)
        {
            write_stream << src_tangents[i].x << src_tangents[i].y << src_tangents[i].z;
        }
    }

    write_stream << num_indices;

    for(uint i=0; i< num_indices; ++i)
    {
        write_stream << src_indices[i];
    }

    if((attribs & ATTRIB_BONES) != 0)
    {
        write_stream << num_bones;

        for(uint i=0; i< num_bones; ++i)
        {
            write_stream << bones[i].name.C_str();
            write_stream << bones[i].bind;
        }

        for(uint i=0; i< num_vertices; ++i)
        {
            write_stream << src_bone_indices[i*4+0] << src_bone_indices[i*4+1] << src_bone_indices[i*4+2] << src_bone_indices[i*4+3];
            write_stream << src_bone_weights[i].x << src_bone_weights[i].y << src_bone_weights[i].z << src_bone_weights[i].w;
        }
    }

    write_stream << static_mesh;
    write_stream << bbox.minPoint.x << bbox.minPoint.y << bbox.minPoint.z;
    write_stream << bbox.maxPoint.x << bbox.maxPoint.y << bbox.maxPoint.z;
}


// ---------------------------------------------------------
bool ResourceMesh::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MESH_FOLDER, "mesh", "edumesh");
}

// ---------------------------------------------------------
UID ResourceMesh::Import(const aiMesh* mesh, const char* source_file)
{
    ResourceMesh* m = static_cast<ResourceMesh*>(App->resources->CreateNewResource(Resource::mesh));

    m->name   = HashString(mesh->mName.C_Str());

    m->GenerateCPUBuffers(mesh);
    if(mesh->HasBones())
    {
        m->GenerateBoneData(mesh);
    }

    m->GenerateAttribInfo();

    std::string output;

    if(m->Save(source_file, output))
    {
        LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());
    }
    else
    {
        LOG("Importing aiMesh %s FAILED", source_file);
    }

    m->ReleaseFromMemory();

    return m->uid;
}

bool ResourceMesh::Save(const char* source, std::string& output)
{
    bool save_ok = Save(output);

    if(save_ok)
    {
		if (source != nullptr) 
        {
			file = source;
			App->fs->NormalizePath(file);
		}

		std::string file_name;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file_name);
		exported_file = file_name;

        App->fs->SplitFilePath(file.c_str(), nullptr, &user_name, nullptr);

        if (user_name.empty())
        {
            user_name = exported_file;
        }

        size_t pos_dot = user_name.find_last_of(".");
        if(pos_dot != std::string::npos)
        {
            user_name.erase(user_name.begin()+pos_dot, user_name.end());
        }

        if(name && name.Length() > 0)
        {
            user_name += "_";
            user_name += name.C_str();
        }
    }

    return save_ok;
}

void ResourceMesh::GenerateAttribInfo()
{
    vertex_size         = sizeof(float3);
    attribs             = 0;
    texcoord0_offset    = 0;
    normal_offset       = 0;
    tangent_offset      = 0;
    bone_weight_offset  = 0;

    if(src_normals != nullptr)
    {
        attribs |= ATTRIB_NORMALS;
        normal_offset = vertex_size*num_vertices;
        vertex_size += sizeof(float3);
    }

    if(src_texcoord0 != nullptr)
    {
        attribs |= ATTRIB_TEX_COORDS_0;
        texcoord0_offset = vertex_size*num_vertices;
        vertex_size += sizeof(float2);
    }

    if(src_texcoord1 != nullptr)
    {
        attribs |= ATTRIB_TEX_COORDS_1;
        texcoord1_offset = vertex_size*num_vertices;
        vertex_size += sizeof(float2);
    }

    if(src_tangents != nullptr)
    {
        attribs |= ATTRIB_TANGENTS;
        tangent_offset = vertex_size*num_vertices;
        vertex_size += sizeof(float3);
    }

    if(src_bone_indices != nullptr && src_bone_weights != nullptr)
    {
        attribs |= ATTRIB_BONES;
        bone_idx_offset = vertex_size*num_vertices;
        vertex_size += sizeof(unsigned)*4;
        bone_weight_offset = vertex_size*num_vertices;
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
        // uncomment iif copy from assimp
        src_tangents = new float3[mesh->mNumVertices];
        memcpy(src_tangents, mesh->mTangents, sizeof(float3)*mesh->mNumVertices);
        //GenerateTangentSpace();
    }

    bbox.SetNegativeInfinity();
    bbox.Enclose(src_vertices, num_vertices);
}

void ResourceMesh::GenerateTangentSpace()
{
	// tangent space

    // edif0 = udif0*t+vdif0*b;
    // edif1 = udif1*t+vdif1*b;
    // b     = edif1/vidf1-udif1/vdif1*t;
    // edif0 = udif0*t+vidf0*edif1/vdif1-vdif0*udif1/vdif1*t;
    // edif0 - vdif0*edif1/vdif1 = t*(udif0-vdif0*udif1/vdif1);

    src_tangents = new math::float3[num_vertices];

    for(unsigned i=0; i <  num_vertices; ++i)
    {
        src_tangents[i] = float3(0.0f);
    }

    for(unsigned i=0; i< num_indices/3; ++i)
    {
        unsigned face[3] = { src_indices[i*3], src_indices[i*3+1], src_indices[i*3+2] };

		float3 edif0 = src_vertices[face[1]]-src_vertices[face[0]];
        float3 edif1 = src_vertices[face[2]]-src_vertices[face[1]];
        float udif0  = src_texcoord0[face[1]].x-src_texcoord0[face[0]].x;
        float vdif0  = src_texcoord0[face[1]].y-src_texcoord0[face[0]].y;
        float udif1  = src_texcoord0[face[2]].x-src_texcoord0[face[1]].x;
        float vdif1  = src_texcoord0[face[2]].y-src_texcoord0[face[1]].y;

        math::float3 t;

        if(fabs(vdif1) < 0.00001f)
        {
            assert(fabs(udif1) > 0.00001f);

            t = edif1*(1.0f/udif1);
        }
        else
        {
            float f = 1.0f/(udif0-vdif0*udif1/vdif1);
            t       = (edif0-edif1*vdif0/vdif1)*f; 
        }

        src_tangents[face[0]] += t;
        src_tangents[face[1]] += t; 
        src_tangents[face[2]] += t;
    }

    for(unsigned i=0; i <  num_vertices; ++i)
    {
		src_tangents[i].Normalize();
        // \todo: orthogonalize ?
    }
}

void ResourceMesh::GenerateVBO()
{
    if(vbo == 0)
    {
        glGenBuffers(1, &vbo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, nullptr, static_mesh ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3)*num_vertices, src_vertices);

    if((attribs & ATTRIB_TEX_COORDS_0) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, texcoord0_offset, sizeof(float2)*num_vertices, src_texcoord0);
    }

    if((attribs & ATTRIB_TEX_COORDS_1) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, texcoord1_offset, sizeof(float2)*num_vertices, src_texcoord1);
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
        glBufferSubData(GL_ARRAY_BUFFER, bone_idx_offset, sizeof(unsigned)*num_vertices*4, src_bone_indices);
        glBufferSubData(GL_ARRAY_BUFFER, bone_weight_offset, sizeof(float4)*num_vertices, src_bone_weights);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(ibo == 0)
    {
        glGenBuffers(1, &ibo);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(unsigned), src_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ResourceMesh::GenerateBoneData(const aiMesh* mesh)
{
    assert(mesh->HasBones());

    bones      = new Bone[mesh->mNumBones];
    num_bones  = mesh->mNumBones;

    for(unsigned i=0; i< mesh->mNumBones; ++i)
    {
        const aiBone* bone   = mesh->mBones[i];
        Bone& dst_bone       = bones[i];

        dst_bone.name 	     = HashString(bone->mName.C_Str());
        dst_bone.bind 	     = float4x4(float4(bone->mOffsetMatrix.a1, bone->mOffsetMatrix.b1, bone->mOffsetMatrix.c1, bone->mOffsetMatrix.d1),
								        float4(bone->mOffsetMatrix.a2, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.d2),
                                        float4(bone->mOffsetMatrix.a3, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.d3),
                                        float4(bone->mOffsetMatrix.a4, bone->mOffsetMatrix.b4, bone->mOffsetMatrix.c4, bone->mOffsetMatrix.d4));
    }

    unsigned* bone_indices = new unsigned[4*mesh->mNumVertices];
    float* bone_weights    = new float[4*mesh->mNumVertices];

    for(unsigned i=0; i < num_vertices*4; ++i) 
    {
        bone_indices[i] = 0;
        bone_weights[i] = 0.0f;
    }

    for(unsigned i=0; i< num_bones; ++i)
    {
        const aiBone* bone = mesh->mBones[i];

        for(unsigned j=0; j < bone->mNumWeights; ++j)
        {
            unsigned index = bone->mWeights[j].mVertexId;
            float weight   = bone->mWeights[j].mWeight;

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

    src_bone_indices = bone_indices;
    src_bone_weights = (float4*)bone_weights;

    math::float4* src_bone_weights = (math::float4*)(bone_weights);
    for(unsigned i=0; i < num_vertices; ++i) 
    {
        float length = 0.0f;
        for(unsigned j=0; j < 4; ++j) 
        {
            length += src_bone_weights[i][j];
        }

        if(length > 0.0f)
        {
            src_bone_weights[i] = src_bone_weights[i] / length;
        }
    }

}

void ResourceMesh::GenerateVAO()
{
    if(vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }

    glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)0);

	if ((attribs & ATTRIB_NORMALS) != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)(normal_offset));
	}

	if ((attribs & ATTRIB_TEX_COORDS_0) != 0)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float2), (void*)(texcoord0_offset));
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
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)(tangent_offset));
    }

	if ((attribs & ATTRIB_TEX_COORDS_1) != 0)
	{
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(float2), (void*)(texcoord1_offset));
	}


    glBindVertexArray(0);
}

UID ResourceMesh::LoadSphere(const char* sphere_name, float size, unsigned slices, unsigned stacks)
{
    par_shapes_mesh* mesh = par_shapes_create_parametric_sphere(int(slices), int(stacks));

	if (mesh)
	{
        par_shapes_scale(mesh, size, size, size);

        UID uid = Generate(sphere_name, mesh);

        par_shapes_free_mesh(mesh);

        return uid;
    }

	return 0;
}

UID ResourceMesh::LoadCylinder(const char* cylinder_name, float height, float radius, unsigned slices, unsigned stacks)
{
    par_shapes_mesh* mesh = par_shapes_create_cylinder(int(slices), int(stacks));
    par_shapes_rotate(mesh, -float(PAR_PI*0.5), (float*)&float3::unitX);
	par_shapes_translate(mesh, 0.0f, -0.5f, 0.0f);

	if (mesh)
	{
        par_shapes_scale(mesh, radius, height, radius);

        for(uint i=0; i< uint(mesh->npoints); ++i)
        {
            std::swap(mesh->tcoords[i*2], mesh->tcoords[i*2+1]);
            mesh->tcoords[i*2]*=2.0f;
        }

        UID uid = Generate(cylinder_name, mesh);

		par_shapes_free_mesh(mesh);

		return uid;
	}

	return 0;
}

UID ResourceMesh::LoadTorus(const char* torus_name, float inner_r, float outer_r, unsigned slices, unsigned stacks)
{
    par_shapes_mesh* mesh = par_shapes_create_torus(int(slices), int(stacks), inner_r);

	if (mesh)
	{
        par_shapes_scale(mesh, outer_r, outer_r, outer_r);

        UID uid = Generate(torus_name, mesh);

		par_shapes_free_mesh(mesh);

		return uid;
	}

	return 0;
}

UID ResourceMesh::LoadCube(const char* cube_name, float size)
{
    par_shapes_mesh* mesh   = par_shapes_create_plane(1, 1);
    par_shapes_mesh* top    = par_shapes_create_plane(1, 1);
	par_shapes_mesh* bottom = par_shapes_create_plane(1, 1);
	par_shapes_mesh* back   = par_shapes_create_plane(1, 1);
	par_shapes_mesh* left   = par_shapes_create_plane(1, 1);
	par_shapes_mesh* right  = par_shapes_create_plane(1, 1);

	par_shapes_translate(mesh, -0.5f, -0.5f, 0.5f);

    par_shapes_rotate(top, -float(PAR_PI*0.5), (float*)&float3::unitX);
	par_shapes_translate(top, -0.5f, 0.5f, 0.5f);

	par_shapes_rotate(bottom, float(PAR_PI*0.5), (float*)&float3::unitX);
	par_shapes_translate(bottom, -0.5f, -0.5f, -0.5f);

	par_shapes_rotate(back, float(PAR_PI), (float*)&float3::unitX);
	par_shapes_translate(back, -0.5f, 0.5f, -0.5f);

	par_shapes_rotate(left, float(-PAR_PI*0.5), (float*)&float3::unitY);
	par_shapes_translate(left, -0.5f, -0.5f, -0.5f);

	par_shapes_rotate(right, float(PAR_PI*0.5), (float*)&float3::unitY);
	par_shapes_translate(right, 0.5f, -0.5f, 0.5f);

    par_shapes_merge_and_free(mesh, top);
	par_shapes_merge_and_free(mesh, bottom);
	par_shapes_merge_and_free(mesh, back);
	par_shapes_merge_and_free(mesh, left);
	par_shapes_merge_and_free(mesh, right);
	 
	if (mesh)
	{
        par_shapes_scale(mesh, size, size, size);

        UID uid = Generate(cube_name, mesh);

		par_shapes_free_mesh(mesh);

		return uid;
	}

	return 0;
}

UID  ResourceMesh::LoadPlane(const char* plane_name, float width, float height, unsigned slices, unsigned stacks)
{
    par_shapes_mesh* mesh = par_shapes_create_plane(slices, stacks);
	par_shapes_translate(mesh, -0.5f, -0.5f, 0.0f);

	if (mesh)
	{
        par_shapes_scale(mesh, width, height, 1.0f);

        UID uid = Generate(plane_name, mesh);

		par_shapes_free_mesh(mesh);

		return uid;
	}

	return 0;
}

UID ResourceMesh::Generate(const char* shape_name, par_shapes_mesh* shape)
{
    ResourceMesh* m = static_cast<ResourceMesh*>(App->resources->CreateNewResource(Resource::mesh));

    m->name = HashString(shape_name);

    m->GenerateCPUBuffers(shape);
    m->GenerateAttribInfo();

	std::string output;
    bool ok = m->Save(shape_name, output);

    m->ReleaseFromMemory();

    return ok ? m->uid : 0;
}

void ResourceMesh::GenerateCPUBuffers(par_shapes_mesh* shape)
{
    src_vertices = new float3[shape->npoints];
    memcpy(src_vertices, shape->points, shape->npoints*sizeof(float3));

    if(shape->normals)
    {
        src_normals = new float3[shape->npoints];
        memcpy(src_normals, shape->normals, shape->npoints*sizeof(float3));
    }
    
    if(shape->tcoords)
    {
        src_texcoord0 = new float2[shape->npoints];
        memcpy(src_texcoord0, shape->tcoords, shape->npoints*sizeof(float2));
    }

    src_indices = new unsigned[shape->ntriangles*3];
	for (uint i = 0; i < uint(shape->ntriangles) * 3; ++i)
	{
		src_indices[i] = shape->triangles[i];
	}

	num_vertices = shape->npoints;
    num_indices  = shape->ntriangles*3;

    if(shape->normals)
    {
        GenerateTangentSpace();
    }
}

void ResourceMesh::UpdateUniforms(const float4x4* skin_palette) const
{
    unsigned vertex_indices[NUM_VERTEX_SUBROUTINE_UNIFORMS];

    if((attribs & ResourceMesh::ATTRIB_BONES) != 0)
    {
        glUniformMatrix4fv(App->programs->GetUniformLocation("palette"), num_bones, GL_TRUE, reinterpret_cast<const float*>(skin_palette));
        vertex_indices[TRANSFORM_OUTPUT] = TRANSFORM_OUTPUT_SKINNING;
    }
    else
    {
        vertex_indices[TRANSFORM_OUTPUT] = TRANSFORM_OUTPUT_RIGID;
    }

    glUniformSubroutinesuiv(GL_VERTEX_SHADER, sizeof(vertex_indices)/sizeof(unsigned), vertex_indices);

}

void ResourceMesh::Draw() const
{
    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ResourceMesh::GenerateTexCoord1()
{
    Atlas_Input_Mesh mesh_input;

    mesh_input.vertex_count = num_vertices;
    mesh_input.face_count   = num_indices/3;
    mesh_input.vertex_array = new Atlas_Input_Vertex[num_vertices]; 
    mesh_input.face_array   = new Atlas_Input_Face[num_indices/3]; 

    for(uint i=0; i< num_vertices; ++i)
    {
        mesh_input.vertex_array[i].position[0] = src_vertices[i][0];
        mesh_input.vertex_array[i].position[1] = src_vertices[i][1];
        mesh_input.vertex_array[i].position[2] = src_vertices[i][2];

        // \note: what if no normals or uvs?
        mesh_input.vertex_array[i].normal[0] = src_normals[i][0];
        mesh_input.vertex_array[i].normal[1] = src_normals[i][1];
        mesh_input.vertex_array[i].normal[2] = src_normals[i][2];

		mesh_input.vertex_array[i].uv[0] = src_texcoord0[i][0];
		mesh_input.vertex_array[i].uv[1] = src_texcoord0[i][1];

        mesh_input.vertex_array[i].first_colocal = i;

		/*
        for(uint j=0; j < i; ++j)
        {
            if(src_vertices[j].x == src_vertices[i].x &&
			   src_vertices[j].y == src_vertices[i].y &&
			   src_vertices[j].z == src_vertices[i].z)
            {
                mesh_input.vertex_array[i].first_colocal = j;
            }
        }
		*/
    }

    for(uint i=0; i< num_indices/3; ++i)
    {
        mesh_input.face_array[i].material_index = 0;
        mesh_input.face_array[i].vertex_index[0] = src_indices[i*3+0];
        mesh_input.face_array[i].vertex_index[1] = src_indices[i*3+1];
        mesh_input.face_array[i].vertex_index[2] = src_indices[i*3+2];

		assert(src_indices[i * 3 + 0] < num_vertices);
		assert(src_indices[i * 3 + 1] < num_vertices);
		assert(src_indices[i * 3 + 2] < num_vertices);
    }

    Atlas_Options atlas_options;
    atlas_set_default_options(&atlas_options);

    atlas_options.packer_options.witness.packing_quality = 1;
	atlas_options.packer_options.witness.texels_per_unit = 2.0f;

    Atlas_Error error = Atlas_Error_Success;
    Atlas_Output_Mesh * mesh_output = atlas_generate(&mesh_input, &atlas_options, &error);

    GenerateCPUBuffers(mesh_output);
    GenerateAttribInfo();
    GenerateVBO();
    GenerateVAO();

    delete [] mesh_input.vertex_array;
    delete [] mesh_input.face_array;

    atlas_free(mesh_output);
}

void ResourceMesh::GenerateCPUBuffers(const Atlas_Output_Mesh* atlas)
{
    num_indices = atlas->index_count;
    memcpy(src_indices, atlas->index_array, sizeof(int)*num_indices);

    num_vertices  = atlas->vertex_count;
    if(src_texcoord1)
    {
        delete [] src_texcoord1;
    }

	src_texcoord1 = new float2[num_vertices];

	for(uint i=0; i < num_vertices; ++i)
    {
        src_texcoord1[i] = float2(atlas->vertex_array[i].uv[0]/atlas->atlas_width, atlas->vertex_array[i].uv[1]/atlas->atlas_height);
    }

    copy_new_vertices(src_vertices, atlas->vertex_array, num_vertices);
    copy_new_vertices(src_texcoord0, atlas->vertex_array, num_vertices);
    copy_new_vertices(src_normals, atlas->vertex_array, num_vertices);
    copy_new_vertices(src_tangents, atlas->vertex_array, num_vertices);
}
