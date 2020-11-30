#include <string_view>
#include <optional>
#include <cctype>
#include <array>
#include <utility>
#include <functional>
#include "../../Basic/Exception.hpp"
#include "ParseFunc.hpp"
#include "../../Json/JsonConverter/WordEnumerator.hpp"

namespace FuncLib::Compile
{
	using Json::JsonConverter::WordEnumerator;
	using ::std::array;
	using ::std::function;
	using ::std::move;
	using ::std::optional;
	using ::std::pair;
	using ::std::string_view;

	/// 不支持全局变量
	/// 不支持模板，以及非 JSON 包含的基本类型作为参数和返回值
	/// return type, name, args
	optional<array<string, 3>> ParseFuncSignature(FuncDefTokenReader* reader)
	{
		array<pair<bool, function<bool(char)>>, 3> parseConfigs
		{
			// pair: 允许为空字符串, delimiter predicate
			pair<bool, function<bool(char)>>{ false, [](char c) { return std::isspace(c); } },
			{ false, [](char c) { return c == '('; } },
			{ true, [](char c) { return c == ')'; } },
		};
		
		array<string, 3> infos;
		auto g = reader->GetTokenGenerator();

		for (auto i = 0; auto c : parseConfigs)
		{
			reader->DelimiterPredicate(c.second);
			while (g.MoveNext())
			{
				auto t = g.Current();
				if (c.first or not t.empty())
				{
					infos[i++] = move(g.Current());
					goto Continue;
				}
			}
			
			return {};
		Continue:
			continue;
		}

		return infos;
	}

	int Count(char c, string_view rangeStr)
	{
		int n = 0;
		for (auto x : rangeStr)
		{
			if (c == x)
			{
				++n;
			}
		}

		return n;
	}

	// WordEnumerator 还是有问题的，因为 start 到 end 可能跨 string_view 了，这就不行了
	/// body 是不包含函数的 { } 的
	vector<string> ParseFuncBodyAfterSignature(FuncDefTokenReader* defReader)
	{
		vector<string> funcBody;
		auto g = defReader->GetTokenGenerator();
		int unpairedCount = 0;
		defReader->DelimiterPredicate([](char c) { return c == '}'; });

		while (g.MoveNext())
		{
			auto part = g.Current();
			unpairedCount += Count('{', part);
			--unpairedCount;

			if (unpairedCount == 0)
			{
				funcBody.push_back(move(part));
				return move(funcBody);
			}
			else
			{
				funcBody.push_back(move(part) + '}');
			}
		}

		return funcBody;
		// throw Basic::InvalidOperationException("{} in func body is unpaired");
	}

	vector<string> ParseOutArgs(string argsStr)
	{
		auto divideArg = [](string_view s, char delimiter)
		{
			auto i = s.find_first_of(delimiter);
			return array<string_view, 2>
			{
				s.substr(0, i), 
				s.substr(i)
			};
		};

		WordEnumerator e{ vector<string_view>{ argsStr }, ','};
		vector<string> args;
		while (e.MoveNext())
		{
			if (not e.Current().empty())
			{
				auto [type, name] = divideArg(e.Current(), ' ');
				args.push_back(string(type));
			}
		}

		return args;
	}

	/// 使用前要保证函数编译正确，比如可以先编译一遍
	/// pair: FuncType, func body 
	vector<pair<FuncType, vector<string>>> ParseFunc(FuncDefTokenReader* defReader)
	{
		vector<pair<FuncType, vector<string>>> funcs;

		while (not defReader->AtEnd())
		{
			if (auto sign = ParseFuncSignature(defReader); sign.has_value())
			{
				auto body = ParseFuncBodyAfterSignature(defReader);
				auto& s = sign.value();
				funcs.push_back({ { move(s[0]), move(s[1]), ParseOutArgs(move(s[2])) }, move(body) });
			}
		}

		return funcs;
	}
}
