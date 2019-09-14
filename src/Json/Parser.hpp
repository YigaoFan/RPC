#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <memory>
#include "ParseException.hpp"
#include "Json.hpp"
#include "LocationInfo.hpp"

namespace Json {
	using std::string;
	using std::make_shared;

	// 我想的是有针对当前正在处理的对象的情境这个概念，
	// 比如当前正在处理一个空格（也就是说现在不进行去除空格的处理了），如果是在字符串里的空格，那就加入进去
	// 如果是项与项之间的，就忽略，如果是其他项内的空格，则报错
	// 还有一个当前期望的东西存在，如果是数字则期望数字，这样不是数字则能比较容易和清楚的报出语法错误
	class Parser {
	public:
		inline
		static
		Json
		parse(const string& jsonStr)
		{
			return Parser(jsonStr).doParse();
		}

		Parser(const string& str)
			: Str(str)
		{ }

		/**
		 * should ensure the string to be parsed exists in current scope
		 */
		inline
		Json
		doParse()
		{
			_parsingLocation = 0;
			if (auto len = Str.length()) {
				return parseJson(_parsingLocation, len - 1);
			}

			throw ParseEmptyStringException();
		}

	private:
		const char* TrueStr = "rue";
		const char* FalseStr = "alse";
		const char* NullStr = "ull";
		const string& Str;
		size_t _parsingLocation;

		inline
		LocationInfo
		parsingLocationInfo() const
		{
			return locationInfoAt(_parsingLocation);
		}

		inline
		LocationInfo
		locationInfoAt(size_t i) const
		{
			return LocationInfo(Str, i);
		}

		Json
		parseJson(size_t& start, size_t end)
		{
			size_t e;
			auto parseType = routeParse(start, e, end)
			return parseByType(parseType, i, e);
		}

		inline
		Json
		parseByType(Json::Type parseType, size_t& start, size_t end)
		{
			switch (parseType) {
				case Object:
					return Json(Object(), parseObject(start, e));

				case Array:
					return Json(Array(), parseArray(start));

				case Number: // TODO
				case String:
					return Json(String(), parseString(start));

				case True:
					return Json(True());

				case False:
					return Json(False());

				case Null:
					return Json(Null());
			}
		}

		// 所有的位置应该是偏向类型内的，比如 Object 的位置包含}，Number 都在 Number 内部上
		Json::Type
		routeParse(size_t& rangeLeftToChange, size_t& rangeRightToSet, size_t rangeEnd)
		{
			auto& i = rangeLeftToChange;
			for (; i < rangeEnd; ++i) {
				auto c = Str[i++];
				switch (c) {
					case '{':
						rangeRightToSet = findRightPair(i, rangeEnd, '}');
						return Json::Type::Object;

					case '[':
						rangeRightToSet = findRightPair(i, rangeEnd, ']');
						return Json::Type::Array;

					case '"':
						// find " from rangeEnd, because single root
						rangeRightToSet = findRightPair(i, rangeEnd, '"');
						reuturn Json::Type::String;

					case 't':
						rangeRightToSet = i + 2; // rue
						ensureSingleRoot(rangeRightToSet+1, rangeEnd);
						return Json::Type::True;

					case 'f':
						rangeRightToSet = i + 3; // alse
						ensureSingleRoot(rangeRightToSet+1, rangeEnd);
						return Json::Type::False;

					case 'n':
						rangeRightToSet = i + 2; // ull
						ensureSingleRoot(rangeRightToSet+1, rangeEnd);
						reuturn Json::Type::Null;

					default:
						if (isSpace(c)) {
							continue;
						} else if (isDigit(c)/* which num includes */) { // TODO handle -1.12
							// find num or other thing from end
							return parseNum(i); // TODO move place
						} else {
							throw InvalidStringException(parsingLocationInfo());
						}
				}
			}

			throw ParseExpectValueException();
		}

		inline
		static
		bool
		isSpace(char c)
		{
			 // check if work on which char
			return std::isblank(static_cast<unsigned char>(c));
		}

		inline
		static
		bool
		isDigit(char c)
		{
			return std::isdigit(static_cast<unsigned char>(c));
		}

		bool
		inJsonString(size_t i)
		{
			// search around
		}

		/**
		 * Parse the object string without {} on both sides
		 */
		Json::_Object
		parseObject(size_t& start, size_t end)
		{
			map<string, Json> objectMap;

			auto expectString = true, expectColon = false, expectJson = false, expectComma = false;
			string key;
			shared_ptr<Json> value;
			for (auto& i = start; i <= end; ++i) {
				auto c = Str[i];
				if (isSpace(c)) {
					continue;
				} else if (expectString && c == '"') {
					key = parseString(++i, end);
					expectString = false;
					expectColon = true;
				} else if (expectColon && c == ':') {
					++i;
					expectColon = false;
					expectJson = true;
				} else if (expectJson) {
					value = make_shared(parseJson(i, end));
					objectMap.emplace(std::move(key), std::move(value));
					key = ""; value.reset();
					expectComma = true;
				} else if (expectComma && c == ',') {
					// maybe object end and support ',' as end
					++i;
					expectComma = false;
					expectString = true;
				} else {
					// throw ... exception
				}
			}

			return objectMap;
		}

		Json::_Array
		parseArray(size_t& i)
		{
			// search pair and judge	
		}

		string
		parseString(size_t& start, size_t end)
		{
			for (auto& i = start, subStart = start, countOfSub = 0;
				i <= end; ++i, ++countOfSub) {
				if (Str[i++] == '"') {
					return { Str, subStart, countOfSub }
				}
			}

			throw InvalidStringException(); // how to record current parse info: mainly position and Str and expect
		}

		inline
		void
		parseTrue(size_t& i)
		{
			parseSimpleType(TrueStr, i);
		}

		inline
		void
		parseFalse(size_t& i)
		{
			parseSimpleType(FalseStr, i);
		}

		inline
		void
		parseNull(size_t& i)
		{
			parseSimpleType(NullStr, i);
		}

		Json
		parseNum(size_t& i)
		{
			
		}

		void
		parseSimpleType(const char* target, size_t& i)
		{
			auto len = target.length();
			for (int j = 0; j < len; ++j, ++i) {
				auto t = target[j];
				auto c = Str[i];
				if (t != s) {
					// TODO more detail
					throw WrongCharException(c);
				}
			}
		}

		/**
		 * will check single root inner
		 * @return Index of expected char
		 */
		size_t
		findRightPair(size_t start, size_t end, char expected)
		{
			for (auto i = end; i >= start; --i) {
				auto c = Str[i];
				if (c == expected) {
					return i;
				} else if (!isSpace(c)) {
					throw ParseNotSingleRootException(LocationInfo);
				}
			}

			throw WrongPairException();
		}

		void
		ensureSingleRoot(size_t checkStart, size_t checkEnd)
		{

		}
	};

	// may use namespace to hide some functions
	namespace {

	}
}