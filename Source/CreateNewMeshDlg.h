#pragma once

#include <string>

class CreateNewMeshDlg
{
    struct SpotConeParams
    {
        char name[1024];
        int slices = 10;
        int stacks = 10;
        float height = 1.0f;
        float radius = 0.1f;

        SpotConeParams()
        {
            name[0] = 0;
        }
    };

    enum MeshType
    {
        MeshType_SpotCone = 0
    };

    UID             meshID = 0;
    bool            openFlag = false;
    std::string     openName;
    int             typeMesh = 0;
    SpotConeParams  params;
public:

    CreateNewMeshDlg();

    void Open           ();
    void Display        ();
    void Clear          ();

    UID getMesh() const { return meshID; }
};