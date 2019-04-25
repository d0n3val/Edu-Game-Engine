#ifndef _POSTPROCESSSHADERLOCATIONS_H_
#define _POSTPROCESSSHADERLOCATIONS_H_

enum PostprocessLocations
{
    SCREEN_TEXTURE_LOCATION = 0,
    BLOOM_TEXTURE_LOCATION
};

enum PostprocessSubroutineLocations
{
    TONEMAP_LOCATION = 0,
    NUM_POSPROCESS_SUBROUTINES
};

enum PostprocessSubroutineIndex
{
    TONEMAP_UNCHARTED2 = 0,
    TONEMAP_REINHARD,
    TONEMAP_NONE
};

#endif /* _POSTPROCESSSHADERLOCATIONS_H_ */
