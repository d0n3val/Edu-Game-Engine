#include "Globals.h"

#include "AnimController.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleResources.h"

#include "Leaks.h"

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
        float me_elapsed = float(elapsed)*0.01f*instance->speed;
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

bool AnimController::GetWeights(const std::string& morph_name, float* weights, uint num_weights) const
{
    if (current != nullptr)
    {
        return GetWeightsInstance(current, morph_name, weights, num_weights, 1.0f);
    }

    return false;
}

bool AnimController::GetWeightsInstance(Instance* instance, const std::string& morph_name, float*& weights, uint num_weights, float lambda) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        const ResourceAnimation::MorphChannel* morph_channel = animation->GetMorphChannel(morph_name);

        if(morph_channel != nullptr)
        {
            assert(instance->time <= animation->GetDuration());

            float key          = float(instance->time*(morph_channel->numKeys-1))/float(animation->GetDuration());
            unsigned key_index = unsigned(key);
            float key_lambda   = key-float(key_index);

            if(key_lambda > 0.0f)
            {
                tmpWeights.clear();
                tmpWeights.resize(num_weights, 0.0f);

                ResourceAnimation::ValueWeights& first  = morph_channel->weights[key_index];
                ResourceAnimation::ValueWeights& second = morph_channel->weights[key_index+1];

                for (uint i = 0; i < first.count; ++i)
                {
                    std::pair<uint, float>& valueWeight = first.valueWeights[i];
                    assert(valueWeight.first < num_weights);
                    tmpWeights[valueWeight.first] = valueWeight.second;
                }

                for (uint i = 0; i < second.count; ++i)
                {
                    std::pair<uint, float>& valueWeight = second.valueWeights[i];
                    assert(valueWeight.first < num_weights);
                    tmpWeights[valueWeight.first] = Interpolate(tmpWeights[valueWeight.first], valueWeight.second, key_lambda);
                }

                for(uint i=0; i< num_weights; ++i)
                {
                    weights[i] = Interpolate(weights[i], tmpWeights[i], lambda);
                }

                /*
                ResourceAnimation::ValueWeights& valueWeights = morph_channel->weights[key_index];
                for (uint i = 0; i < valueWeights.count; ++i)
                {                    
                    float w0 = morph_channel->weights[i * morph_channel->num_keys + key_index];
                    float w1 = morph_channel->weights[i * morph_channel->num_keys + key_index+1]; 
                    weights[i] = Interpolate(weights[i], Interpolate(w0, w1, key_lambda), lambda);
                }
                */
            }
            else
            {
                tmpWeights.clear();
                tmpWeights.resize(num_weights, 0.0f);

                ResourceAnimation::ValueWeights& first  = morph_channel->weights[key_index];

                for (uint i = 0; i < first.count; ++i)
                {
                    std::pair<uint, float>& valueWeight = first.valueWeights[i];
                    assert(valueWeight.first < num_weights);
                    tmpWeights[valueWeight.first] = valueWeight.second;
                }

                for(uint i=0; i< num_weights; ++i)
                {
                    weights[i] = Interpolate(weights[i], tmpWeights[i], lambda);
                }

                /*
                for(uint i=0; i< num_weights; ++i)
                {
                    weights[i] = Interpolate(weights[i], morph_channel->weights[i* morph_channel->num_keys+key_index], lambda);
                }
                */
            }

            if(instance->next != nullptr)
            {
                assert(instance->fade_duration > 0.0f);

               float blend_lambda = float(instance->fade_time) / float(instance->fade_duration);

                GetWeightsInstance(instance->next, morph_name, weights, num_weights, blend_lambda*lambda);
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

            auto sampleAnim = []<typename T>(T & result, const std::unique_ptr<T[]>&samples, const std::unique_ptr<float[]>&times, uint count, float time)
            {
                if (count > 0)
                {
                    uint index = uint(std::lower_bound(times.get(), times.get() + count, time) - times.get());
                    if (index + 1 < count)
                    {
                        float lambda = (time - times[index]) / (times[index + 1] - times[index]);
                        result = Interpolate(samples[index], samples[index + 1], lambda);
                    }
                    else
                    {
                        result = time >= times[count - 1] ? samples[count - 1] : samples[0];
                    }
                }
            };

            sampleAnim(position, channel->positions, channel->posTime, channel->num_positions, instance->time);
            sampleAnim(rotation, channel->rotations, channel->rotTime, channel->num_rotations, instance->time);

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

