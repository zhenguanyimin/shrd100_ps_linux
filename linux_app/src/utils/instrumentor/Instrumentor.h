#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <thread>

#ifdef USE_MSVC
    #define FUNCTION_DETAIL __FUNCSIG__
#else
    #define FUNCTION_DETAIL __PRETTY_FUNCTION__
#endif

#define PROFILING 1
#if PROFILING
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(FUNCTION_DETAIL)
#else 
#define PROFILE_SCOPE(name)
#endif

//当前监测的会话
struct InstrumentationSession
{
	std::string name;
};

//检测结果数据
struct ProfileResult
{
	std::string name;		//调用栈函数名称
	long long start, stop;	//起止时间
	uint32_t threadID;		//线程ID
};

class Instrumentor
{
public:
	Instrumentor();
    void BeginSession(const std::string& name, const std::string& filepath = "result.json");
	void EndSession();
	void WriteProfile(const ProfileResult& result);
	void WriteHeader();
	void WriteFooter();
	static Instrumentor& Get();
private:
	InstrumentationSession* m_CurrentSession;
	std::ofstream m_OutputStream;
	int m_ProfileCount;
};

//计时器
class InstrumentationTimer
{
public:
	InstrumentationTimer(const std::string& title);
	~InstrumentationTimer();
	void stop();
private:
	std::string title;
	std::chrono::high_resolution_clock::time_point m_start;
	bool m_stoped;
};
