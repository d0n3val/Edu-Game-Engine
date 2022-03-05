#ifndef _DEFAULT_SHADER_BINDINGS_H_
#define _DEFAULT_SHADER_BINDINGS_H_

#include "ResourceMaterial.h"

const uint transformBlockIndex = 10;
const uint materialsBlockIndex = 5;
const uint instancesBlockIndex = 15;
const uint skinningBlockIndex = 16;
const uint morphDataIndex = 13;
const uint morphWeightsIndex = 17;
const uint directionalBlockIndex = 12;
const uint pointBlockIndex = 13;
const uint spotBlockIndex = 14;
const uint cameraBlockIndex = 0;
const uint levelsLoc = 64;
const uint diffuseIBLUnit = TextureCount;
const uint prefilteredIBLUnit = TextureCount + 1;
const uint environmentIBLUnit = TextureCount + 2;

#endif /* _DEFAULT_SHADER_BINDINGS_H_ */