= スレッド

== スレッドを便利につかいやすくする

ThreadBaseは、スレッドを使いやすくするためにラップした基底クラスです。
Runメソッドをオーバーライドして、スレッドで実行する処理を記述します。


//listnum[ThreadBase][ThreadBase.ixx]{
module;
#include <atomic>
#include <thread>

export module ThreadBase;

export class ThreadBase
{
public:
	~ThreadBase();
	void Start();
	void ReStart();
	void Terminate();
	bool IsRunning() const;

protected:
	virtual void Run(std::stop_token) = 0;
	void Sleep(std::chrono::nanoseconds duration) const;
	void Yield() const;

private:
	std::jthread _jthread;
};

ThreadBase::~ThreadBase()
{
	Terminate();
}

void ThreadBase::Start()
{
	if (_jthread.joinable())
	{
		return;
	}
	_jthread = std::jthread(
		[this](std::stop_token st)
		{
			this->Run(st);
		});
}
void ThreadBase::ReStart()
{
	Terminate();
	Start();
}
void ThreadBase::Terminate()
{
	if (_jthread.joinable())
	{
		_jthread.request_stop();
		_jthread.join();
	}
}
bool ThreadBase::IsRunning() const
{
	return _jthread.joinable();
}
void ThreadBase::Sleep(std::chrono::nanoseconds duration) const
{
	std::this_thread::sleep_for(duration);
}
void ThreadBase::Yield() const
{
	std::this_thread::yield();
}
//}

