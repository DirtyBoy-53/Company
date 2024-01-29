#include "ActiveXyComponents.h"

#include <Windows.h>
CActiveXyComponents::CActiveXyComponents()
{
	m_handle = nullptr;
}


CActiveXyComponents::~CActiveXyComponents()
{
	if (m_handle) {
        FreeLibrary((HMODULE)m_handle);
		m_handle = nullptr;
	}
}

std::string CActiveXyComponents::currentPath()
{
    char szPath[1024] = { 0 };
    GetModuleFileNameA(NULL, szPath, 1023);
    strrchr(szPath, '\\')[1] = '\0';
    return std::string(szPath);
}

bool CActiveXyComponents::get(std::string dllname, const char *iid, void **p)
{
    std::string path = currentPath();
    path.append(dllname);
    m_handle = ::LoadLibraryA(path.c_str());
    if (!m_handle) {
        return false;
    }

    typedef bool (*GetObj)(const char*, void**);
    GetObj obj = (GetObj)::GetProcAddress((HMODULE)m_handle, "getXyObject");
    if (!obj) {
        return false;
    }

    if (obj(iid, p)) {
        if (*p) {
            return true;
        }
    }

    return false;
}
