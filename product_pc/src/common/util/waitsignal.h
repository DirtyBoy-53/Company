#pragma once
#include <condition_variable>
class WaitSignal {
public:
	WaitSignal() :m_bFlag(false)
	{
	}
	~WaitSignal()
	{
	}
	bool wait(int iTimeOut = 1) {
		std::unique_lock<std::mutex> lockWait(m_mtxLock);
		if (m_cvNotify.wait_for(lockWait, std::chrono::milliseconds(iTimeOut), [this] {return m_bFlag;}))
		{
			m_bFlag = false;
			return true;
		}
		return false;
	}
	void signal() {
		{std::lock_guard<std::mutex> lockWait(m_mtxLock);
		m_bFlag = true;}
		m_cvNotify.notify_one();
	}
private:
	bool m_bFlag;
	std::condition_variable m_cvNotify;
	std::mutex m_mtxLock;


};
