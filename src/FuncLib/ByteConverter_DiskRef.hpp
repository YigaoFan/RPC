#pragma once

namespace FuncLib
{
	template <typename T>
	struct ByteConverter<DiskRef<T>, false>
	{
		using ThisType = DiskRef<T>;
		using DataMemberType = decltype(declval<ThisType>()._ptr);
		static constexpr bool SizeStable = All<GetSizeStable, DataMemberType>::Result;

		static void WriteDown(ThisType const& t, shared_ptr<FileWriter> writer)
		{
			ByteConverter<DataMemberType>::WriteDown(t._ptr, writer);
		}

		static ThisType ReadOut(shared_ptr<FileReader> reader)
		{
			return ByteConverter<DataMemberType>::ReadOut(reader);
		}
	};
}
