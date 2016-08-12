#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__
#define DLL_API __declspec(dllexport)  

#include <thread>

class DLL_API CMyThread {

public:
	CMyThread():m_exited(true){
	}

	virtual ~CMyThread() {

	}
	
	void start() {
		if (m_exited) {
			m_exited = false;
			m_impl = std::thread(&CMyThread::run, this);
		}
	}

	void stop() {
		if (!m_exited) {
			m_exited = true;
			m_impl.join();
		}
	}

protected:
	virtual void run() {

	}
	bool			m_exited;

private:
	std::thread		m_impl;
};

#endif // !__PROCESSOR_H__
