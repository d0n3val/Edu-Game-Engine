#ifndef _AMBIENT_LIGHT_H_
#define _AMBIENT_LIGHT_H_

#include "Math.h"

class Config;

class AmbientLight
{
public:

    AmbientLight();
    ~AmbientLight();

    void Save(Config& config) const;
    void Load(Config& config);

    const float3& GetColor    () const { return color; }
    void          SetColor    (const float3& c) { color = c; }

private:

    float3 color = float3::zero;
};

#endif /* _AMBIENT_LIGHT_H_ */
