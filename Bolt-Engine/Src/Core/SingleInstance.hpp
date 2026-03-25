#pragma once
#include <windows.h>
#include <string>

namespace Bolt {

    class SingleInstance
    {
    public:
        explicit SingleInstance(const std::string& mutexName)
        {
            m_Handle = CreateMutexA(nullptr, TRUE, mutexName.c_str());

            if (m_Handle && GetLastError() == ERROR_ALREADY_EXISTS)
            {
                m_AlreadyRunning = true;
            }
        }

        ~SingleInstance()
        {
            if (m_Handle)
            {
                ReleaseMutex(m_Handle);
                CloseHandle(m_Handle);
            }
        }

        bool IsAlreadyRunning() const { return m_AlreadyRunning; }

    private:
        HANDLE m_Handle = nullptr;
        bool m_AlreadyRunning = false;
    };
}

