#pragma once
#include <string>
class CActiveXyComponents
{
public:
	CActiveXyComponents();
	~CActiveXyComponents();
    std::string currentPath();

    bool get(std::string dllname, const char* iid, void** p);

private:
    void* m_handle;
};

