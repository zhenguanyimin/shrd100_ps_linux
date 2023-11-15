#include "Instrumentor.h"

Instrumentor::Instrumentor() : m_CurrentSession(nullptr), m_ProfileCount(0)
{
}
void Instrumentor::BeginSession(const std::string& name, const std::string& filepath)
{
	m_OutputStream.open(filepath);
	WriteHeader();
	m_CurrentSession = new InstrumentationSession{ name };
}
void Instrumentor::EndSession()
{
	WriteFooter();
	m_OutputStream.close();
	delete m_CurrentSession;
	m_CurrentSession = nullptr;
	m_ProfileCount = 0;
}
void Instrumentor::WriteProfile(const ProfileResult& result)
{
	if (m_ProfileCount++ > 0)
	{
		m_OutputStream << ",";
	}
	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');
	m_OutputStream << "{";
	m_OutputStream << "\"cat\":\"function\",";
	m_OutputStream << "\"dur\":" << (result.stop - result.start) << ",";
	m_OutputStream << "\"name\":\"" << name << "\",";
	m_OutputStream << "\"ph\":\"X\",";
	m_OutputStream << "\"pid\": 0,";
	m_OutputStream << "\"tid\": \"" << result.threadID << "\",";
	m_OutputStream << "\"ts\": " << result.start;
	m_OutputStream << "}";
	m_OutputStream.flush();
}
void Instrumentor::WriteHeader()
{
	m_OutputStream << "{\"otherData\": {}, \"traceEvents\": [";
	m_OutputStream.flush();
}
void Instrumentor::WriteFooter()
{
	m_OutputStream << "]}";
	m_OutputStream.flush();
}
Instrumentor& Instrumentor::Get()
{
	static Instrumentor* instance = new Instrumentor();
	return *instance;
}

InstrumentationTimer::InstrumentationTimer(const std::string& title) : title(title), m_stoped(false)
{
	m_start = std::chrono::high_resolution_clock::now();
}
InstrumentationTimer::~InstrumentationTimer()
{
	if (!m_stoped)
	{
		stop();
	}
}
void InstrumentationTimer::stop()
{
	auto m_stop = std::chrono::high_resolution_clock::now();
	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
	long long stop = std::chrono::time_point_cast<std::chrono::microseconds>(m_stop).time_since_epoch().count();
	
	uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
	Instrumentor::Get().WriteProfile({ title, start, stop, threadID});
	m_stoped = true;
}
