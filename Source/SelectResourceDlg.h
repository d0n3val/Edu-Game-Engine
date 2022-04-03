#ifndef __SELECT_RESOURCE_DLG_H__
#define __SELECT_RESOURCE_DLG_H__

#include <string>

class SelectResourceDlg
{
public:

    UID         resource     = 0;
    bool        selection    = false;
    bool        open_flag    = false;
    int         type         = 0;
    int         openUniqueId = 0;
    std::string open_name;

public:

    void Open           (int resourceType, const char* popupName, int uniqueId);
    void Display        ();
    void ClearSelection ();

    bool HasSelection   (int uniqueId) const { return openUniqueId == uniqueId ? selection : false; }
    UID  GetResource    () const { return resource; }


private:
    UID DrawResource();

};

#endif /* __SELECT_RESOURCE_DLG_H__ */
