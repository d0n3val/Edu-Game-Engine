#include "Globals.h"

#include "CascadeShadowPass.h"
#include "Application.h"
#include "ModuleHints.h"

CascadeShadowPass::CascadeShadowPass()
{
}

CascadeShadowPass::~CascadeShadowPass()
{
}

void CascadeShadowPass::execute(const Frustum& culling)
{
    static const uint resolutions[] = {ModuleHints::SHADOW_CASCADE_0_RES, ModuleHints::SHADOW_CASCADE_1_RES, ModuleHints::SHADOW_CASCADE_2_RES };

    for(int i=0; i< CASCADE_COUNT; ++i)
    {
        Frustum frustum = culling;
        float2 depth;

        switch (i)
        {
        case 2:
            depth.x = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH).y+
                      App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_1_DEPTH);
            depth.y = depth.x+App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_2_DEPTH);
            break;
        case 1:
            depth.x = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH).y;
            depth.y = depth.x+App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_1_DEPTH);
            break;
        case 0:
            depth = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH);
            break;
        default:
            break;
        }

        frustum.nearPlaneDistance = depth.x;
        frustum.farPlaneDistance = depth.y;

        float2 res = App->hints->GetFloat2Value(ModuleHints::Hint(resolutions[i]));
        cascades[i].execute(frustum, uint(res.x), uint(res.y));
    }
}

void CascadeShadowPass::debugDraw()
{
    // TODO:
}
