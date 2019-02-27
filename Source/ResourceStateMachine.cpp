#include "Globals.h"
#include "ResourceStateMachine.h"

#include "ModuleResources.h"
#include "Application.h"

#include "mmgr/mmgr.h"

// ---------------------------------------------------------
ResourceStateMachine::ResourceStateMachine(UID id) : Resource(id, Resource::state_machine)
{
}

// ---------------------------------------------------------
ResourceStateMachine::~ResourceStateMachine()
{
}

// ---------------------------------------------------------
bool ResourceStateMachine::LoadInMemory() 
{
	return true;
}

// ---------------------------------------------------------
void ResourceStateMachine::ReleaseFromMemory()
{
	for (std::vector<Clip>::iterator it = clips.begin(), end = clips.end(); it != end; ++it)
	{
		Resource* res = App->resources->Get(it->resource);

		if (res != nullptr)
		{
			res->Release();
		}
	}
}

// ---------------------------------------------------------
bool ResourceStateMachine::Save()
{
    return true;
}

// ---------------------------------------------------------
bool ResourceStateMachine::Save(std::string& output) const
{
	return true;
}

// ---------------------------------------------------------
void ResourceStateMachine::AddClip(const HashString& name, UID resource, bool loop)
{
    clips.push_back(Clip(name, resource, loop));
}

// ---------------------------------------------------------
void ResourceStateMachine::RemoveClip(uint index)
{
    std::vector<Node>::iterator it = nodes.begin();

    while(it != nodes.end())
    {
        if(it->clip == clips[index].name)
        {
            RemoveNodeTransitions(it->name);
            it = nodes.erase(it);
        }
        else
        {
            ++it;
        }
    }

    clips.erase(clips.begin()+index);
}

// ---------------------------------------------------------
uint ResourceStateMachine::FindClip(const HashString& name) const
{
    uint i=0;

    for(uint count = clips.size(); i < count; ++i)
    {
        if(clips[i].name == name)
            break;
    }

    return i;
}

// ---------------------------------------------------------
void ResourceStateMachine::AddNode(const HashString& name, const HashString& clip, float speed)
{
    nodes.push_back(Node(name, clip, speed));
}

// ---------------------------------------------------------
void ResourceStateMachine::RemoveNode(uint index)
{
    RemoveNodeTransitions(nodes[index].name);
    nodes.erase(nodes.begin()+index);
}

// ---------------------------------------------------------
void ResourceStateMachine::RemoveNodeTransitions(const HashString& name)
{
    std::vector<Transition>::iterator it = transitions.begin();

    while(it != transitions.end())
    {
        if(it->source == name || it->target == name)
        {
            it = transitions.erase(it);
        }
        else
        {
            ++it;
        }
    }

}

// ---------------------------------------------------------
uint ResourceStateMachine::FindNode(const HashString& name) const
{
    uint i=0;

    for(uint count = nodes.size(); i < count; ++i)
    {
        if(nodes[i].name == name)
            break;
    }

    return i;
}

// ---------------------------------------------------------
void ResourceStateMachine::AddTransition(const HashString& source, const HashString& target, const HashString& trigger, uint blend)
{
    transitions.push_back(Transition(source, target, trigger, blend));
}

// ---------------------------------------------------------
void ResourceStateMachine::RemoveTransition(uint index)
{
    transitions.erase(transitions.begin()+index);
}

// ---------------------------------------------------------
uint ResourceStateMachine::FindTransition(const HashString& source, const HashString& trigger) const
{
    uint i=0;

    for(uint count = transitions.size(); i < count; ++i)
    {
        if(transitions[i].source == source && transitions[i].trigger == trigger)
            break;
    }

    return i;
}

