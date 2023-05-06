#include "Globals.h"

#include "CameraUBO.h"
#include "ComponentCamera.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

CameraUBO::CameraUBO()
{

}

CameraUBO::~CameraUBO()
{

}

void CameraUBO::Update(ComponentCamera* camera)
{
    struct CameraData
    {
        float4x4 proj     = float4x4::identity;
        float4x4 view     = float4x4::identity;
        float4   view_pos = float4::zero;
    } cameraData;

    if(!ubo)
    {
        ubo.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));
    }

    cameraData.proj     = camera->GetProjectionMatrix();  
    cameraData.view     = camera->GetViewMatrix();
    cameraData.view_pos = float4(cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart()), 1.0);

    ubo->InvalidateData();
    ubo->SetData(0, sizeof(CameraData), &cameraData);
}

void CameraUBO::Bind()
{
    ubo->BindToPoint(CAMERA_UBO_BINDING);
}