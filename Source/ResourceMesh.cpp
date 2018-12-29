#include "ResourceMesh.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"

#include "OpenGL.h"

#include "Assimp/include/mesh.h"
#include "utils/SimpleBinStream.h"

#pragma warning(push)
#pragma warning(disable : 4996)  
#pragma warning(disable : 4244)  
#pragma warning(disable : 4305)  
#pragma warning(disable : 4838)  

#define PAR_SHAPES_IMPLEMENTATION
#include "utils/par_shapes.h"
#pragma warning(pop)

#include "mmgr/mmgr.h"

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

        simple::mem_istream<std::true_type> read_stream(buffer, size);
        std::string tmp;

        read_stream >> tmp;
        name = HashString(tmp.c_str());

        read_stream >> vertex_size;
        read_stream >> attribs;
        read_stream >> texcoord_offset;
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

				read_stream >> bones[i].num_weights;

				bones[i].weights = new Weight[bones[i].num_weights];

                for(uint j=0; j< bones[i].num_weights; ++j)
                {
                    read_stream >> bones[i].weights[j].vertex;
                    read_stream >> bones[i].weights[j].weight;
                }
            }
        }

        read_stream >> bbox.minPoint.x >> bbox.minPoint.y >> bbox.minPoint.z;
        read_stream >> bbox.maxPoint.x >> bbox.maxPoint.y >> bbox.maxPoint.z;

        GenerateVBO(false);
        GenerateVAO();

        delete [] buffer;

		return true;
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

    for(uint i=0; i< num_bones; ++i)
    {
        delete [] bones[i].weights;
    }

    delete [] bones;
    bones = nullptr;

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
bool ResourceMesh::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    write_stream << name.C_str();
    write_stream << vertex_size;
    write_stream << attribs;
    write_stream << texcoord_offset;
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
            write_stream << bones[i].num_weights;

            for(uint j=0; j< bones[i].num_weights; ++j)
            {
                write_stream << bones[i].weights[j].vertex;
                write_stream << bones[i].weights[j].weight;
            }
        }
    }

    write_stream << bbox.minPoint.x << bbox.minPoint.y << bbox.minPoint.z;
    write_stream << bbox.maxPoint.x << bbox.maxPoint.y << bbox.maxPoint.z;

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MESH_FOLDER, "mesh", "edumesh");
}

// ---------------------------------------------------------
UID ResourceMesh::Import(const aiMesh* mesh, const char* source_file)
{
    ResourceMesh* m = static_cast<ResourceMesh*>(App->resources->CreateNewResource(Resource::mesh));

    m->name   = HashString(mesh->mName.C_Str());

    m->GenerateAttribInfo(mesh);
    m->GenerateCPUBuffers(mesh);

    if((m->attribs & ATTRIB_BONES) != 0)
    {
        m->GenerateBoneData(mesh);
    }

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

		std::string file;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file);
		exported_file = file;
    }

    return save_ok;
}

void ResourceMesh::GenerateAttribInfo(const aiMesh* mesh)
{
    vertex_size         = sizeof(float3);
    attribs             = 0;
    texcoord_offset     = 0;
    normal_offset       = 0;
    tangent_offset      = 0;
    bone_weight_offset  = 0;

    if(mesh->HasNormals())
    {
        attribs |= ATTRIB_NORMALS;
        normal_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(float3);
    }

    if(mesh->HasTextureCoords(0))
    {
        attribs |= ATTRIB_TEX_COORDS_0;
        texcoord_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(float2);
    }

    if(mesh->HasTangentsAndBitangents())
    {
        attribs |= ATTRIB_TANGENTS;
        tangent_offset = vertex_size*mesh->mNumVertices;
        vertex_size += sizeof(float3);
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
        //src_tangents = new float3[mesh->mNumVertices];
        //memcpy(src_tangents, mesh->mTangents, sizeof(float3)*mesh->mNumVertices);
        GenerateTangentSpace();
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

void ResourceMesh::GenerateVBO(bool dynamic)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3)*num_vertices, src_vertices);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)0);

	if ((attribs & ATTRIB_NORMALS) != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)(normal_offset));
	}

	if ((attribs & ATTRIB_TEX_COORDS_0) != 0)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float2), (void*)(texcoord_offset));
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

    m->GenerateAttribInfo(shape);
    m->GenerateCPUBuffers(shape);

	std::string output;
    bool ok = m->Save(shape_name, output);

    m->ReleaseFromMemory();

    return ok ? m->uid : 0;
}

void ResourceMesh::GenerateAttribInfo (par_shapes_mesh* shape)
{
    vertex_size         = sizeof(float3);
    attribs             = 0;
    texcoord_offset     = 0;
    normal_offset       = 0;
    tangent_offset      = 0;
    bone_weight_offset  = 0;

    if(shape->normals)
    {
        attribs |= ATTRIB_NORMALS;
        normal_offset = vertex_size*shape->npoints;
        vertex_size += sizeof(float3);
    }

    if(shape->tcoords)
    {
        attribs |= ATTRIB_TEX_COORDS_0;
        texcoord_offset = vertex_size*shape->npoints;
        vertex_size += sizeof(float2);
    }

    if(shape->normals)
    {
        attribs |= ATTRIB_TANGENTS;
        tangent_offset = vertex_size*shape->npoints;
        vertex_size += sizeof(float3);
    }
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

    if((attribs & ATTRIB_TANGENTS) != 0)
    {
        GenerateTangentSpace();
    }
}

