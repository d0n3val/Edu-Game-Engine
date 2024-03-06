#include "Globals.h"

#include "AnimController.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleResources.h"

#include "Leaks.h"

#include <algorithm>

namespace
{

    inline float Interpolate(float first, float second, float lambda) 
    {
        return first * (1.0f - lambda) + second * lambda;
    }

    inline float3 Interpolate(const float3& first, const float3& second, float lambda) 
    {
        return float3::Lerp(first, second, lambda);
    }

    inline Quat Interpolate(const Quat& first, const Quat& second, float lambda) 
    {
        // note: ensure minimal angle interpolation
        if(first.Dot(second) >= 0.0f)
        {
            return Quat::Lerp(first, second, lambda).Normalized();
        }
        return Quat::Lerp(first, second.Neg(), lambda).Normalized();
    }

}

AnimController::AnimController()
{
}

AnimController::~AnimController()
{
    if(current != nullptr)
    {
        ReleaseInstance(current);
    }
}

void AnimController::Update(unsigned elapsed)
{
    if(current != nullptr)
    {
        UpdateInstance(current, elapsed);
    }
}

void AnimController::UpdateInstance(Instance* instance, unsigned elapsed)
{
	ResourceAnimation* anim = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

	if (anim != nullptr && anim->GetDuration() > 0)
	{
        float me_elapsed = float(elapsed)*0.001f*instance->speed;
        me_elapsed = fmodf(me_elapsed, anim->GetDuration());
		float to_end = anim->GetDuration() - instance->time;

		if (me_elapsed <= to_end)
		{
			instance->time += me_elapsed;
		}
		else if (instance->loop)
		{
			instance->time = (me_elapsed - to_end);
		}
		else
		{
			instance->time = anim->GetDuration();
		}

		assert(instance->time <= anim->GetDuration());
	}

	if(instance->next != nullptr)
	{
		unsigned to_end = instance->fade_duration-instance->fade_time;
		if(elapsed <= to_end)
		{
			instance->fade_time += elapsed;
			UpdateInstance(instance->next, elapsed);
		}
		else
		{
			ReleaseInstance(instance->next);
			instance->next = nullptr;
			instance->fade_time = instance->fade_duration = 0;
		}
	}
}

void AnimController::Play(UID clip, bool loop, unsigned fade_time)
{
    Instance* new_instance = new Instance;
    new_instance->clip = clip;
    new_instance->loop = loop;
    new_instance->fade_duration = fade_time;
    new_instance->next = current;

    current = new_instance;
}

void AnimController::Stop()
{
    if(current != nullptr)
    {
        ReleaseInstance(current);
		current = nullptr;
    }
}

void AnimController::ReleaseInstance(Instance* instance)
{
	do
	{
		Instance* next = instance->next;
		delete instance;
		instance = next;
	} while (instance != nullptr);
}

std::span<const float> AnimController::GetWeights(const std::string& morph_name) const
{
    tmpWeights0.clear();

    if (current != nullptr)
    {
        const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(current->clip));
        
        const ResourceAnimation::MorphChannel* morph_channel = animation->GetMorphChannel(morph_name);
        if (morph_channel)
        {
            tmpWeights0.resize(animation->GetMorphChannel(morph_name)->numTargets, 0.0f);

            if (GetWeightsInstance(current, morph_name, tmpWeights0.data(), uint(tmpWeights0.size())))
            {
                return std::span<const float>(tmpWeights0);
            }
        }
    }

    return std::span<const float>(tmpWeights0); 
}

bool AnimController::GetWeightsInstance(Instance* instance, const std::string& morph_name, float* weights, uint num_weights) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        const ResourceAnimation::MorphChannel* morph_channel = animation->GetMorphChannel(morph_name);

        if(morph_channel != nullptr)
        {
            SDL_assert(instance->time <= animation->GetDuration());
            SDL_assert(morph_channel->numTargets == num_weights);

            if (morph_channel->numTime > 1)
            {
                uint index = uint(std::upper_bound(morph_channel->weightTime.get(), morph_channel->weightTime.get() + morph_channel->numTime, instance->time) - morph_channel->weightTime.get());
                if (index < morph_channel->numTime)
                {

                    float lambda;
                    if (index > 0)
                    {
                        lambda = (instance->time - morph_channel->weightTime[index - 1]) / (morph_channel->weightTime[index] - morph_channel->weightTime[index - 1]);

                        for (uint i = 0; i < num_weights; ++i)
                        {
                            weights[i] = Interpolate(morph_channel->weights[(index - 1) * num_weights + i], morph_channel->weights[index * num_weights + i], lambda);
                        }
                    }
                    else
                    {
                        lambda = instance->time / morph_channel->weightTime[index];

                        for (uint i = 0; i < num_weights; ++i)
                        {
                            weights[i] = Interpolate(0.0f, morph_channel->weights[index * num_weights + i], lambda);
                        }
                    }
                }
                else
                {
                    for (uint i = 0; i < num_weights; ++i)
                    {
                        weights[i] = morph_channel->weights[(index - 1) * num_weights +i];
                    }
                }
            }
            else if (morph_channel->numTime == 1)
            {
                for(uint i=0; i < num_weights; ++i) weights[i] = morph_channel->weights[0];
            }

            if (instance->next != nullptr)
            {
                assert(instance->fade_duration > 0.0f);

                SDL_assert(static_cast<ResourceAnimation*>(App->resources->Get(instance->next->clip))->GetMorphChannel(morph_name)->numTargets == num_weights);
                tmpWeights1.resize(num_weights);

                if (GetWeightsInstance(instance->next, morph_name, tmpWeights1.data(), num_weights))
                {
                    float blend_lambda = float(instance->fade_time) / float(instance->fade_duration);

                    for (uint i = 0; i < uint(tmpWeights1.size()); ++i)
                    {
                        weights[i] = Interpolate(tmpWeights1[i], weights[i], blend_lambda);
                    }
                }
            }

            return true;
        }
    }

	return false;
}

bool AnimController::GetTransform(const std::string& channel_name, float3& position, Quat& rotation) const
{
    if(current != nullptr)
    {
        return GetTransformInstance(current, channel_name, position, rotation);
    }

	return false;
}

bool AnimController::GetTransformInstance(Instance* instance, const std::string& channel_name, float3& position, Quat& rotation) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        const ResourceAnimation::Channel* channel = animation->GetChannel(channel_name);

        if(channel != nullptr)
        {
            SDL_assert(instance->time <= animation->GetDuration());

            auto sampleAnim = []<typename T>(T & result, const std::unique_ptr<T[]>&samples, const std::unique_ptr<float[]>&times, uint count, float time, const T& defValue)
            {
                if (count > 1)
                {
                    uint index = uint(std::upper_bound(times.get(), times.get() + count, time) - times.get());
                    if (index < count)
                    {
                        float lambda;
                        if (index > 0)
                        {
                            lambda = (time - times[index - 1]) / (times[index] - times[index - 1]);
                            result = Interpolate(samples[index - 1], samples[index], lambda);
                        }
                        else
                        {
                            lambda = time / times[index];
                            result = Interpolate(defValue, samples[index], lambda);
                        }
                    }
                    else
                    {
                        result = samples[count-1];
                    }
                }
                else if(count == 1)
                {
                    result = samples[0];
                }
            };

            sampleAnim(position, channel->positions, channel->posTime, channel->num_positions, instance->time, float3::zero);
            sampleAnim(rotation, channel->rotations, channel->rotTime, channel->num_rotations, instance->time, Quat::identity);

            if(instance->next != nullptr)
            {
                assert(instance->fade_duration > 0.0f);

                float3 next_position;
                Quat next_rotation;

                if(GetTransformInstance(instance->next, channel_name, next_position, next_rotation))
                {
                    float blend_lambda = float(instance->fade_time) / float(instance->fade_duration);

                    position = Interpolate(next_position, position, blend_lambda);
                    rotation = Interpolate(next_rotation, rotation, blend_lambda);
                }
            }

            return true;
        }
    }

	return false;
}

