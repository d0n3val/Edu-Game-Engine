#ifndef __IMPORT_MODEL_DLG_H__
#define __IMPORT_MODEL_DLG_H__

#include <string>

class ImportModelDlg
{
    std::string file;
    std::string open_name;
    std::string user_name;
    float       scale      = 1.0f;
    bool        selection  = false;
    bool        open_flag  = false;
    
public:

    ImportModelDlg();

    void Open           (const std::string& _file);
    void Display        ();
    void ClearSelection ();

    bool               HasSelection () const { return selection; }
    const std::string& GetFile      () const { return file; }
    const std::string& GetUserName  () const { return user_name; }
    float              GetScale     () const { return scale; }

};

#endif /* __IMPORT_MODEL_DLG_H__ */
