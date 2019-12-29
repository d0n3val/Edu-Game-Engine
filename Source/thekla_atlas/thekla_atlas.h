
// Thekla Atlas Generator

#if THEKLA_ATLAS_SHARED
#if _WIN32
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif
#ifdef THEKLA_ATLAS_EXPORTS
#define THEKLA_ATLAS_API DLL_EXPORT
#else
#define THEKLA_ATLAS_API DLL_IMPORT
#endif
#else
#define THEKLA_ATLAS_API
#endif


namespace Thekla {

enum Atlas_Charter {
    Atlas_Charter_Witness,
    Atlas_Charter_Extract,
    Atlas_Charter_Default = Atlas_Charter_Witness
};

enum Atlas_Mapper {
    Atlas_Mapper_LSCM,
    Atlas_Mapper_Default = Atlas_Mapper_LSCM
};

enum Atlas_Packer {
    Atlas_Packer_Witness,
    Atlas_Packer_Default = Atlas_Packer_Witness
};

struct Atlas_Options {
    Atlas_Charter charter;
    union {
        struct {
            float proxy_fit_metric_weight;
            float roundness_metric_weight;
            float straightness_metric_weight;
            float normal_seam_metric_weight;
            float texture_seam_metric_weight;
            float max_chart_area;
            float max_boundary_length;
        } witness;
        struct {
        } extract;
    } charter_options;

    Atlas_Mapper mapper;
    struct {
        bool preserve_uvs;
        bool preserve_boundary;
    } mapper_options;

    Atlas_Packer packer;
    union {
        struct {
            int packing_quality;
            float texels_per_unit;      // Unit to texel scale. e.g. a 1x1 quad with texelsPerUnit of 32 will take up approximately 32x32 texels in the atlas.
            bool block_align;           // Align charts to 4x4 blocks. 
            bool conservative;          // Pack charts with extra padding.
        } witness;
    } packer_options;
};

struct Atlas_Input_Vertex {
    float position[3];
    float normal[3];
    float uv[2];
    int first_colocal;
};

struct Atlas_Input_Face {
    int vertex_index[3];
    int material_index;
};

struct Atlas_Input_Mesh {
    int vertex_count;
    int face_count;
    Atlas_Input_Vertex * vertex_array;
    Atlas_Input_Face * face_array;
};

struct Atlas_Output_Vertex {
    float uv[2];
    int xref;   // Index of input vertex from which this output vertex originated.
};

struct Atlas_Output_Mesh {
    int atlas_width;
    int atlas_height;
    int vertex_count;
    int index_count;
    Atlas_Output_Vertex * vertex_array;
    int * index_array;
};

enum Atlas_Error {
    Atlas_Error_Success,
    Atlas_Error_Invalid_Args,
    Atlas_Error_Invalid_Options,
    Atlas_Error_Invalid_Mesh,
    Atlas_Error_Invalid_Mesh_Non_Manifold,
    Atlas_Error_Not_Implemented,
};

typedef void Atlas_Debug_Output(const char * output);
THEKLA_ATLAS_API void atlas_set_debug_output(Atlas_Debug_Output * output);

THEKLA_ATLAS_API void atlas_set_default_options(Atlas_Options * options);

THEKLA_ATLAS_API Atlas_Output_Mesh * atlas_generate(const Atlas_Input_Mesh * input, const Atlas_Options * options, Atlas_Error * error);

THEKLA_ATLAS_API void atlas_free(Atlas_Output_Mesh * output);


/*

Should we represent the input mesh with an opaque structure that simply holds pointers to the user data? That would allow us to avoid having to copy attributes to an intermediate representation.

struct Atlas_Input_Mesh;

void mesh_set_vertex_position(Atlas_Input_Mesh * mesh, float * ptr, int stride);
void mesh_set_vertex_normal(Atlas_Input_Mesh * mesh, float * ptr, int stride);
void mesh_set_vertex_uv(Mesh * mesh, float * ptr, int stride);

void mesh_set_index(Mesh * mesh, int * ptr);
*/

} // Thekla namespace

