#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include <Windows.h>
#include <varargs.h>

class ETool {
public:
	static void ETrace(const char* format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string sBuffer;
		sBuffer.resize(1024 * 10);
		vsprintf((char*)(sBuffer.c_str()), format, ap);
		OutputDebugStringA(sBuffer.c_str());
		va_end(ap);
	}
};

#ifndef TRACE
#define TRACE ETool::ETrace
#endif

class ThreadFuncBase {};
typedef int (ThreadFuncBase::* FUNCTYPE)();

class ThreadWorker {
public:
	ThreadWorker():thiz(NULL), func(NULL) {}
	ThreadWorker(void* obj, FUNCTYPE f):thiz((ThreadFuncBase*)obj), func(f){}
	ThreadWorker(const ThreadWorker& worker){
		thiz = worker.thiz;
		func = worker.func;
	}

	ThreadWorker& operator=(const ThreadWorker& worker){
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	int operator()() {
		if (IsValid()) {
			return (thiz->*func)();
		}
		return -1;
	}
	bool IsValid() const{
		return (thiz != NULL) && (func != NULL);
	}

private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};

class EdyThread
{
public:
	EdyThread(){
		m_hThread = NULL;
		m_bStatus = false;
	}
	~EdyThread() {
		Stop();
	}

	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&ThreadEntry, 0, this);
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus;
	}

	bool IsValid() { // 返回true表示有效
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
			return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}

	bool Stop() {
		if (m_bStatus == false)
			return false;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, 1000);
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread, -1);
		}
		UpdateWorker();
		return ret == WAIT_OBJECT_0;
	}

	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker()) {
		if (m_worker.load() != NULL && (m_worker.load() != &worker)) {
			::ThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		if (m_worker.load() == &worker)
			return;
		if (!worker.IsValid()) {
			m_worker.store(NULL);
			return;
		}
		m_worker.store(new ::ThreadWorker(worker));
	}

	// true表示空闲，false表示已经分配了工作
	bool IsIdle() { 
		if (m_worker.load() == NULL) {
			return true;
		}
		return !m_worker.load()->IsValid();
	}

private:
	void ThreadWorker() {
		while (m_bStatus) {
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker(); //重载运算符()，执行函数
					if (ret != 0) {
						TRACE("thread found warning code %d\r\n", ret);
					}
					if (ret < 0) {
						m_worker.store(NULL);
					}
				}
			}
			else {
				Sleep(1);
			}
		}
	}

	static void ThreadEntry(void* arg) {
		EdyThread* thiz = (EdyThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus; // false:表示线程将要关闭; true:线程正在执行
	std::atomic<::ThreadWorker*> m_worker;
};



class EdyThreadPool {
public:
	EdyThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++) {
			m_threads[i] = new EdyThread();
		}
	}

	EdyThreadPool(){}
	~EdyThreadPool(){
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++) {
			delete m_threads[i];
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}

	bool Invoke() {
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->Start() == false) {
				ret = false;
				break;
			}
		}
		if (ret == false) {
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		return ret;
	}

	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}

	//返回-1表示分配失败，所有线程都在忙，大于等于0表示第n个线程分配完成
	int DispatchWorker(const ThreadWorker& worker) {
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) {
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<EdyThread*> m_threads;
};