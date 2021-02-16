#ifndef __IMPORT_ANIMATION_DLG_H__
#define __IMPORT_ANIMATION_DLG_H__

#include <vector>
#include <string>

class ImportAnimationDlg
{
public:

    struct Clip
    {
        char        name[128];
        uint        first = 0;
        uint        last  = uint(INT_MAX);

        Clip()
        {
            name[0] = 0;
        }
    };

    std::string         file;
    std::string         clips_name;
    std::vector<Clip>   clips;
    bool                selection = false;
    float               scale     = 1.0f;

public:

    void Open           (const std::string& _file, const std::string& name);
    void Display        ();
    void ClearSelection ();

    bool                     HasSelection   () const { return selection; }
    const std::string&       GetFile        () const { return file; }
    const std::vector<Clip>& GetClips       () const { return clips; }
    float                    GetScale       () const { return scale; }

};

#endif /* __IMPORT_ANIMATION_DLG_H__ */
