#pragma once
#include <tuple>
#include <string>
#include <sstream>
#include <type_traits>
#include "Curry.hpp"
#include "FormatMap.hpp"
#include "FormatParser.hpp"

namespace Log
{
	using ::std::forward;
	using ::std::remove_reference_t;
	using ::std::string;
	using ::std::stringstream;
	using ::std::tie;
	using ::std::tuple;
	using ::std::tuple_cat;
	using ::std::tuple_size_v;

	// 分离 hpp 和 cpp
	// 每天存档前一日的 log 文件，触发 log 操作的时候检查下，或者能设置定时回调吗？设个定时任务
	class Logger
	{
	private:
		// 可以设置向控制台和文件输出，输出后清空
		stringstream _strStream { stringstream::out | stringstream::app };

	public:
		Logger()
		{

		}

		// 用前三个作为基础 log 内容
		template <size_t BasicInfoLimitCount, typename CurryedAccessLogger, typename... CurrentBasicInfos>
		struct SubLogger;

		template <typename... Infos>
		void WriteAccessLog(Infos... infos)
		{
			// TODO
		}

		template <char... FormatChars>
		auto GenerateSubLogger(integer_sequence<char, FormatChars...> format)
		{
			auto typeList = ParseFormat(format);
			using TypeList = decltype(typeList);
			constexpr int size = Length<TypeList>::Result;
			auto idxs = make_index_sequence<size>();
			constexpr size_t basicInfoLimitCount = 3;
			return MakeSubLogger<basicInfoLimitCount>(GetCurryedAccessLog(typeList, idxs), this, tuple());
		}
		
		// below with some basic info
		void Info(string message)
		{

		}

		void Warn(string message)
		{

		}

		template <typename Exception>
		void Warn(string message, Exception const& exception)
		{

		}

		template <typename Exception>
		void Error(string message, Exception const& exception)
		{
			
		}
	
	private:
		template <typename TypeList, size_t... Idxs>
		auto GetCurryedAccessLog(TypeList, index_sequence<Idxs...>)
		{
			// Logger::WriteAccessLog<typename Get<TypeList, Idxs>::Result...>();
			auto funcAddr = &Logger::WriteAccessLog<typename Get<TypeList, Idxs>::Result...>;
			return Curry(funcAddr)(this);
		}

		template <size_t BasicInfoLimitCount, typename CurryedAccessLogger, typename... CurrentBasicInfos>
		static auto MakeSubLogger(CurryedAccessLogger curryedAccessLog, Logger *parentLogger, tuple<CurrentBasicInfos...> infos)
		{
			return SubLogger<BasicInfoLimitCount, CurryedAccessLogger, CurrentBasicInfos...>(
				move(curryedAccessLog), move(parentLogger), move(infos));
		}
	};

	template <size_t BasicInfoLimitCount, typename CurryedAccessLogger, typename... CurrentBasicInfos>
	struct Logger::SubLogger
	{
		CurryedAccessLogger CurryedAccessLog;
		Logger* ParentLogger;
		tuple<CurrentBasicInfos...> BasicInfo; // 作为后来 log 的前缀

		// 这里的构造函数推导要怎么用？可以直接用吗而不完全指定类型的模板参数，可见当时委员会的人也可能比较困惑
		// 所以后来才支持这个特性
		// template <typename... Infos>
		SubLogger(decltype(CurryedAccessLog) curryedAccessLog, decltype(ParentLogger) parentLogger, tuple<CurrentBasicInfos...> infos)
			: CurryedAccessLog(move(curryedAccessLog)), ParentLogger(parentLogger), BasicInfo(move(infos))
		{ }
		// move constructor and set ParentLogger to nullptr

		template <typename Info, typename T>
		auto BornNewWith(Info&& info)
		{
			auto newerAccessLog = CurryedAccessLog(forward<Info>(info));
			auto parent = ParentLogger;
			ParentLogger = nullptr;

			if constexpr (tuple_size_v<decltype(BasicInfo)> < BasicInfoLimitCount)
			{
				auto newBasicInfo = tuple_cat(BasicInfo, tie(remove_reference_t<Info>(info)));
				return SubLogger{ move(newerAccessLog), parent, move(newBasicInfo) };
			}
			else
			{
				return SubLogger{ move(newerAccessLog), parent, move(BasicInfo) };
			}
		}

		void Info(string message);
		void Warn(string message);
		template <typename Exception>
		void Warn(string message, Exception const &exception);
		template <typename Exception>
		void Error(string message, Exception const &exception);

		~SubLogger()
		{
			if (ParentLogger != nullptr)
			{
				// cons message
				string message;
				ParentLogger->Info(message);
			}
		}
	};

	// https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/

	Logger MakeLogger()
	{
		return Logger();
	}
}
