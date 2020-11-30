#include "FunctionLibrary.hpp"
#include "../Basic/Exception.hpp"
#include "Compile/CompileProcess.hpp"

namespace FuncLib
{
	using Basic::InvalidOperationException;
	using ::std::move;
	using ::std::filesystem::is_directory;

	FunctionLibrary::FunctionLibrary(decltype(_index) index, decltype(_binLib) binLib)
		: _index(move(index)), _binLib(move(binLib))
	{
	}

	FunctionLibrary FunctionLibrary::GetFrom(path dirPath)
	{
		if (not is_directory(dirPath))
		{
			throw InvalidOperationException(string("Cannot operate on path: ") + dirPath.c_str() + " which is not directory");
		}

		auto indexFilePath = dirPath / "func.idx";
		auto binFilePath = dirPath / "func_bin.lib";
		auto i = FuncBinaryLibIndex::GetFrom(indexFilePath);
		auto b = FuncBinaryLib::GetFrom(binFilePath);
		return FunctionLibrary(move(i), move(b));
	}

	void FunctionLibrary::Add(vector<string> packageHierarchy, FuncDefTokenReader defReader, string summary)
	{
		auto [funcs, bin] = Compile::Compile(&defReader);
		auto p = _binLib.Add(move(bin));

		for (auto& f : funcs)
		{
			f.Type.PackageHierarchy(packageHierarchy);
			if (_index.Contains(f.Type))
			{
				throw InvalidOperationException("Function already exist: " + f.Type.ToString());
			}
			else
			{
				// 那这样要考虑多个引用一个的问题，删的时候要注意
				_binLib.AddRefCount(p);
				_index.Add(f.Type, { p.Label(), summary });
			}
		}
	}

	bool FunctionLibrary::Contains(FuncType const& type) const
	{
		return _index.Contains(type);
	}

#define FUNC_NOT_EXIST_EXCEPTION(FUNC_TYPE) throw InvalidOperationException("Function not exist: " + FUNC_TYPE.ToString())
	void FunctionLibrary::ModifyFuncName(FuncType const& type, string newFuncName)
	{
		if (_index.Contains(type))
		{
			return _index.ModifyFuncName(type, move(newFuncName));
		}

		FUNC_NOT_EXIST_EXCEPTION(type);
	}

	void FunctionLibrary::ModifyPackageNameOf(FuncType const &type, vector<string> packageHierarchy)
	{
		if (_index.Contains(type))
		{
			return _index.ModifyPackageOf(type, move(packageHierarchy));
		}

		FUNC_NOT_EXIST_EXCEPTION(type);
	}

	void FunctionLibrary::Remove(FuncType const& type)
	{
		if (_index.Contains(type))
		{
			auto l = GetStoreLabel(type);
			_binLib.DecreaseRefCount(l);
			_index.Remove(type);
			return;
		}

		FUNC_NOT_EXIST_EXCEPTION(type);
	}

	JsonObject FunctionLibrary::Invoke(FuncType const& type, JsonObject args)
	{
		auto l = GetStoreLabel(type);
		auto bytes = _binLib.Read(l);
		// call func_wrapper function
		// load byte and call func

		// return result
		return JsonObject(1.0);
	}

	// keyword maybe part package name, 需要去匹配，所以返回值可能不能做到返回函数的相关信息
	Generator<pair<string, pair<pos_label, string>>> FunctionLibrary::Search(string const& keyword)
	{
		return _index.Search(keyword);
	}

	pos_label FunctionLibrary::GetStoreLabel(FuncType const& type)
	{
		if (_funcInfoCache.contains(type))
		{
			return _funcInfoCache[type];
		}

		if (_index.Contains(type))
		{
			auto l = _index.GetStoreLabel(type);
			_funcInfoCache.insert({ type, l });
			return l;
		}

		FUNC_NOT_EXIST_EXCEPTION(type);
	}
#undef FUNC_NOT_EXIST_EXCEPTION
}