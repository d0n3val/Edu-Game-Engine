#ifndef _COMPRESSTEXTURE_DLG_H_
#define _COMPRESSTEXTURE_DLG_H_

#include <string>

class CompressTextureDlg
{
    int type = 0;
    bool selection = false;
    bool open_flag = false;
    std::string open_name;
    int         openUniqueId = 0;
    
public:

    CompressTextureDlg();

    void Open           (int uniqueId);
    void Display        ();
    void ClearSelection ();
    
    bool HasSelection (int uniqueId) const { return openUniqueId == uniqueId ? selection : false; }
    int GetType() const {return type;}
};

#endif /* _COMPRESSTEXTURE_DLG_H_ */
