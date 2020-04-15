#pragma once
#include <filesystem>
#include <memory>
#include <map>
#include "DiskDataConverter.hpp"

namespace FuncLib
{
	using ::std::shared_ptr;
	using ::std::map;
	using ::std::size_t;

	template <typename T>
	class DiskPos
	{
	private:
		friend struct DiskDataConverter<DiskPos>;
		static map<size_t, shared_ptr<T>> _cache; // 用 start 应该是没事的, if change, should delete

		size_t _start;
	public:
		using Index = decltype(_start);

		// DiskPos 调用 Converter 来转换存储和读取涉及到的转换
		DiskPos(size_t start) : _start(start)
		{ }

		shared_ptr<T> ReadObject()
		{
			return DiskDataConverter::ConvertFromDiskData<T>(_start);
			/*if (!this->_cache.contains(_start))
			{
				auto p = shared_ptr(CurrentFile::Read(_start, _size));
				_cache.insert({ _start, p });
				return p;
			}

			return this->_cache[_start];*/
		}

		void WriteObject()
		{

		}

		~DiskPos();

	private:

	};
}