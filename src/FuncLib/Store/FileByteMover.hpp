#pragma once
#include <memory>
#include <filesystem>
#include "StaticConfig.hpp"

namespace FuncLib::Store
{
	using ::std::shared_ptr;
	using ::std::filesystem::path;

	class FileByteMover
	{
	protected:
		shared_ptr<path> filename_;
		pos_int pos_;
	public:
		FileByteMover(shared_ptr<path> filename, pos_int startPos);
	};
}
