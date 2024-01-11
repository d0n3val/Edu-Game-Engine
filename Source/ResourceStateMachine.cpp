#include "Globals.h"
#include "ResourceStateMachine.h"

#include "ModuleResources.h"
#include "Application.h"
#include "ModuleFileSystem.h"

#include "Leaks.h"

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
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;

        uint size = App->fs->Load(LIBRARY_STATE_MACHINE_FOLDER, GetExportedFile(), &buffer);

        if (size > 0)
        {
            simple::mem_istream<std::true_type> read_stream(buffer, size);

            uint array_size = 0;
            read_stream >> array_size;

            clips.resize(array_size);

            for (uint i = 0; i < clips.size(); ++i)
            {
                Clip& clip = clips[i];

                std::string name;
                read_stream >> name;

                clip.name = HashString(name.c_str());

                read_stream >> clip.resource;
                read_stream >> clip.loop;
            }

            read_stream >> array_size;

            nodes.resize(array_size);

            for (uint i = 0; i < nodes.size(); ++i)
            {
                Node& node = nodes[i];

                std::string name;
                read_stream >> name;

                node.name = HashString(name.c_str());

                name.clear();
                read_stream >> name;

                node.clip = HashString(name.c_str());
            }

            read_stream >> array_size;

            transitions.resize(array_size);

            for (uint i = 0; i < transitions.size(); ++i)
            {
                Transition& transition = transitions[i];

                std::string name;
                read_stream >> name;

                transition.source = HashString(name.c_str());
                name.clear();
                read_stream >> name;
                transition.target = HashString(name.c_str());
                name.clear();
                read_stream >> name;
                transition.trigger = HashString(name.c_str());

                read_stream >> transition.blend;
            }

            for (uint i = 0; i < clips.size(); ++i)
            {
                if (clips[i].resource != 0)
                {
                    Resource* anim_res = App->resources->Get(clips[i].resource);

                    if (anim_res)
                    {
                        anim_res->LoadToMemory();
                    }
                }
            }

            return true;
        }
    }

    return false;
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

    clips.clear();
    nodes.clear();
    transitions.clear();
}

// ---------------------------------------------------------
bool ResourceStateMachine::Save()
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    if(exported_file.length() > 0)
    {
		char full_path[250];

		sprintf_s(full_path, 250, "%s%s", LIBRARY_STATE_MACHINE_FOLDER, exported_file.c_str());

        return App->fs->Save(full_path, &data[0], uint(data.size())) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_STATE_MACHINE_FOLDER, "states", "edustates"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }

	return false;
}

// ---------------------------------------------------------
bool ResourceStateMachine::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    return App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_STATE_MACHINE_FOLDER, "states", "edustates");
}

// ---------------------------------------------------------
void ResourceStateMachine::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << uint(clips.size());

    for(uint i=0; i < clips.size(); ++i)
    {
        const Clip& clip = clips[i];

        write_stream << clip.name.C_str();
        write_stream << clip.resource;
        write_stream << clip.loop;
    }

    write_stream << uint(nodes.size());
    for(uint i=0; i< nodes.size(); ++i)
    {
        const Node& node = nodes[i];

        write_stream << node.name.C_str();
        write_stream << node.clip.C_str();
    }

    write_stream << uint(transitions.size());
    for(uint i=0; i< transitions.size(); ++i)
    {
        const Transition& transition = transitions[i];

        write_stream << transition.source.C_str();
        write_stream << transition.target.C_str();

		if (transition.trigger)
		{
			write_stream << transition.trigger.C_str();
		}
        else
        {
            write_stream << "";
        }
        write_stream << transition.blend;
    }
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

    for(uint count = uint(clips.size()); i < count; ++i)
    {
        if(clips[i].name == name)
            break;
    }

    return i;
}

// ---------------------------------------------------------
void ResourceStateMachine::AddNode(const HashString& name, const HashString& clip)
{
    nodes.push_back(Node(name, clip));
}

// ---------------------------------------------------------
void ResourceStateMachine::RemoveNode(uint index)
{
    RemoveNodeTransitions(nodes[index].name);
    nodes.erase(nodes.begin()+index);

    if(nodes.empty())
    {
        default_node = 0;
    }
    else
    {
        default_node = std::min(default_node, uint(nodes.size()-1));
    }
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

    for(uint count = uint(nodes.size()); i < count; ++i)
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

    for(uint count = uint(transitions.size()); i < count; ++i)
    {
        if(transitions[i].source == source && transitions[i].trigger == trigger)
            break;
    }

    return i;
}

