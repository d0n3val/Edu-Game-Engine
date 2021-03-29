#ifndef __SELECT_RESOURCE_DLG_H__
#define __SELECT_RESOURCE_DLG_H__

#include <string>

class SelectResourceDlg
{
public:

    UID         resource = 0;
    bool        selection = false;
    bool        open_flag = false;
    int         type      = 0;
    std::string open_name;

public:

    void Open           (int resourceType, const char* popupName);
    void Display        ();
    void ClearSelection ();

    bool HasSelection   () const { return selection; }
    UID  GetResource    () const { return resource; }


private:
    UID DrawResource();

};

#endif /* __SELECT_RESOURCE_DLG_H__ */
