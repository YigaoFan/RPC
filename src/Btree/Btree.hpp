#pragma once
#include <vector>       // for vector
#include <functional>   // for function
#include <memory>       // for unique_ptr
#include <utility>      // for pair
#include <algorithm>    // for sort
#include <array>        // for array
#include <exception>    // for exception
#include "Basic.hpp"
#include "NodeFactory.hpp"
#include "Exception.hpp"
#include "SiblingFunc.hpp"

namespace Collections
{
	using ::std::function;
	using ::std::array;
	using ::std::pair;
	using ::std::make_pair;
	using ::std::vector;
	using ::std::sort;
	using ::std::exception;
	using ::std::runtime_error;
	using ::std::unique_ptr;
	using ::std::make_shared;
	using ::std::size_t;
	using ::std::move;
	using ::std::make_index_sequence;
	using ::std::index_sequence;

	template <auto Total, auto ItemCapacity>
	struct PerNodeCountGenerator
	{
		constexpr static int Current = Total == 0 ? 
			0 : (Total % ItemCapacity == 0 ? 
				ItemCapacity : ((Total % ((Total / ItemCapacity) + 1) == 0 ? 
					(Total / ((Total / ItemCapacity) + 1)) : ((Total / ((Total / ItemCapacity) + 1)) + 1))));
		using Next = PerNodeCountGenerator<Total - Current, ItemCapacity>;
	};

	template <auto ItemCapacity>
	struct PerNodeCountGenerator<0, ItemCapacity>
	{
		constexpr static int Current = 0;
	};

	template <auto Index, auto Total, auto ItemCapacity>
	struct PositionGetter
	{
		using Position = typename PositionGetter<Index - 1, Total, ItemCapacity>::Position::Next;
	};

	template <auto Total, auto ItemCapacity>
	struct PositionGetter<0, Total, ItemCapacity>
	{
		using Position = PerNodeCountGenerator<Total, ItemCapacity>;
	};

	template <auto Total, auto ItemCapacity, auto Index>
	constexpr auto GetItemsCount()
	{
		return PositionGetter<Index, Total, ItemCapacity>::Position::Current;
	}

	/*template <auto Total, auto ItemCapacity, size_t I, size_t... Is>
	constexpr auto GetPreItemsCountHelper()
	{
		if constexpr (sizeof...(Is) == 0) 
		{
			return GetItemsCount<Total, ItemCapacity, I>();
		}
		else
		{
			return GetItemsCount<Total, ItemCapacity, I>() + GetPreItemsCountHelper<Total, ItemCapacity, Is...>();
		}

	}*/

	template <auto Total, auto ItemCapacity, size_t... Is>
	constexpr auto GetPreItemsCountImp(index_sequence<Is...>)
	{
		return (0 + ... + GetItemsCount<Total, ItemCapacity, Is>());
		//return GetPreItemsCountHelper<Total, ItemCapacity, Is...>();
	}

	template <auto Total, auto ItemCapacity, auto Index>
	constexpr auto GetPreItemsCount()
	{
		return GetPreItemsCountImp<Total, ItemCapacity>(make_index_sequence<Index>());
	}

	template <auto Total, auto ItemCapacity>
	constexpr auto GetNodeCount()
	{
		return Total == 0 ?
			0 : (Total % ItemCapacity == 0 ? (Total / ItemCapacity) : (Total / ItemCapacity + 1));
	}

	/*template <bool Condition, typename A, typename B>
	struct CompileIf;

	template <typename A, typename B>
	struct CompileIf<true, A, B>
	{
		using Type = A;
	};

	template <typename A, typename B>
	struct CompileIf<true, A, B>
	{
		using Type = B;
	};*/
	// TODO use Enumerator to refactor code
	

	template <order_int BtreeOrder, typename Key, typename Value>
	class Btree 
	{
	private:
		using Base   = NodeBase  <Key, Value, BtreeOrder>;
		using Leaf   = LeafNode  <Key, Value, BtreeOrder>;
		using Middle = MiddleNode<Key, Value, BtreeOrder>;
		using NodeFactoryType = NodeFactory<Key, Value, BtreeOrder>;

	public:
		using LessThan = typename Base::LessThan;

		template <size_t NumOfEle>
			// not should be array, a type can be iterate is OK?
		Btree(LessThan lessThan, array<pair<Key, Value>, NumOfEle> keyValueArray)
			: _lessThanPtr(make_shared<LessThan>(move(lessThan)))
		{
			if constexpr (NumOfEle == 0) { return; }

			// 可以自己实现一个排序算法，这样找重复的容易些
			// 而且反正 pairArray 是在成功的情况下是要复制的，
			// 这个构造函数这也要复制，不如构造函数传引用，排序算法确定不重复的情况下，就直接复制到堆上
			// 可以确定好几个后一起构造
			sort(keyValueArray.begin(), keyValueArray.end(),
				 [&](const auto& p1, const auto& p2)
			{
				return (*_lessThanPtr)(p1.first, p2.first);
			});

			if (const Key *dupKeyPtr; DuplicateIn(keyValueArray, dupKeyPtr))
			{
				throw DuplicateKeyException(*dupKeyPtr, "Duplicate key in constructor keyValueArray");
			}

			ConstructFromLeafToRoot(move(keyValueArray));
			_keyNum += NumOfEle;
		}

		// TODO Enumerator constructor
		// Btree(LessThan lessThan, )

		Btree(const Btree& that)
			: _keyNum(that._keyNum), _root(that._root->clone()), _lessThanPtr(that._lessThanPtr)
		{ }

		Btree(Btree &&that) noexcept
			: _keyNum(that._keyNum), _root(that._root.release()), _lessThanPtr(that._lessThanPtr)
		{
			that._keyNum = 0;
		}

		Btree& operator=(const Btree& that)
		{
			this->_root.reset(that._root->clone());
			this->_keyNum = that._keyNum;
			this->_lessThanPtr = that._lessThanPtr;
		}

		Btree& operator=(Btree&& that) noexcept 
		{
			this->_root.reset(that._root.release());
			this->_keyNum = that._keyNum;
			this->_lessThanPtr = that._lessThanPtr;
		}

		Value search(const Key &key) const
		{
			if (empty())
			{
				throw runtime_error("The tree is empty");
			}

			return _root->search(key);
		}

		vector<Key> explore() const 
		{
			vector<Key> keys;
			keys.reserve(_keyNum);

			TraverseLeaf([&keys](Leaf *l)
			{
				auto &&ks = l->allKey();
				keys.insert(keys.end(), ks.begin(), ks.end());

				return false;
			});

			return keys;
		}

		bool have(const Key &key) const
		{
			if (!empty())
			{
				return _root->have(key);
			}

			return false;
		}

		bool empty() const
		{
			return _keyNum == 0;
		}

		void add(pair<Key, Value> p) 
		{
			if (empty())
			{
				auto leaf = make_unique<Leaf>(&p, &p + 1, _lessThanPtr);
				_root.reset(leaf.release());
			}
			else
			{
				vector<Base *> passedNodeTrackStack;
				if (_root->have(p.first, passedNodeTrackStack))
				{
					throw runtime_error("The key-value has already existed, can't be added.");
				}
				else
				{
					_root->add(std::move(p), passedNodeTrackStack);
				}
			}

			++_keyNum;
		}

		// void        tryAdd(pair<Key, Value>);
		void modify(pair<Key, Value> pair)
		{
			if (!empty())
			{
				_root->modify(pair.first, std::move(pair.second));
			}
		}

		void remove(const Key &key)
		{
			vector<Base *> passedNodeTrackStack;
			auto &stack = passedNodeTrackStack;

			if (empty())
			{
				return;
			}
			if (_root->have(key, stack)) 
			{
				_root->remove(key, stack);
				--_keyNum;
			}
		}

	private:
		shared_ptr<LessThan> _lessThanPtr;
		uint32_t             _keyNum{ 0 };
		unique_ptr<Base>     _root  { nullptr };

		vector<Leaf*> TraverseLeaf(const function<bool(Leaf *)>& predicate) const
		{
			vector<Leaf*> leafCollection{};

			if (empty())
			{
				return leafCollection;
			}

			auto current = minLeaf(_root.get());
			while (current != nullptr)
			{
				if (predicate(current))
				{
					leafCollection.emplace_back(current);
				}

				current = current->nextLeaf();
			}

			return leafCollection;
		}
		
		template <auto Total, auto Num, auto... nums>
		static void ForEachCons(function<void(int, int, int)> func)
		{
			func(Num, GetItemsCount<Total, BtreeOrder, Num>(), GetPreItemsCount<Total, BtreeOrder, Num>());
			if constexpr (sizeof...(nums) > 0)
			{
				ForEachCons<Total, nums...>(func);
			}
		}

		template <typename T, auto Count, size_t... Is>
		static auto ConsNodeInArrayImp(array<T, Count> srcArray, shared_ptr<LessThan> lessThan, index_sequence<Is...> is)
		{
			array<pair<Key, unique_ptr<Base>>, is.size()> consNodes;
			ForEachCons<Count, Is...>([&srcArray, &consNodes, &lessThan](auto index, auto itemsCount, auto preItemsCount)
			{
				auto node = NodeFactoryType::MakeNode(&srcArray[preItemsCount], &srcArray[preItemsCount + itemsCount], lessThan);
				consNodes[index] = make_pair(move(node->maxKey()), move(node));
			});

			return consNodes;
		}

		template <typename T, auto Count>
		static auto ConsNodeInArray(array<T, Count> src, shared_ptr<LessThan> lessThan)
		{
			return ConsNodeInArrayImp(move(src), move(lessThan), make_index_sequence<GetNodeCount<Count, BtreeOrder>()>());
		}

		template <bool FirstCall=true, typename T, size_t Count>
		void ConstructFromLeafToRoot(array<T, Count> ItemsToConsNode)
		{
			if constexpr (Count <= BtreeOrder)
			{
				_root = NodeFactoryType::MakeNode(ItemsToConsNode.begin(), ItemsToConsNode.end(), _lessThanPtr);
				return;
			}

			auto newNodes = ConsNodeInArray(move(ItemsToConsNode), _lessThanPtr);
			// TODO wait to set next leaf of leaf
			ConstructFromLeafToRoot<false>(move(newNodes));
		}

		template <size_t NumOfEle>
		static bool DuplicateIn(array<pair<Key, Value>, NumOfEle> const &sortedPairArray, Key const *&duplicateKey)
		{
			auto &array = sortedPairArray;

			for (auto i = 1; i < NumOfEle; ++i) 
			{
				// should use LessThan ? TODO
				if (array[i].first == array[i - 1].first) 
				{
					duplicateKey = &array[i].first;
					return true;
				}
			}

			duplicateKey = nullptr;
			return false;
		}

		static Leaf* minLeaf(Base *node)
		{
			function<Base *(Middle *)> min = [](auto n) 
			{
				return n->minSon();
			};

			return recurSelectNode(node, min);
		}

		static Leaf* recurSelectNode(Base *node, function<Base *(Middle *)> &choose)
		{
			while (node->middle())
			{
				node = choose(static_cast<Middle *>(node));
			}

			return static_cast<Leaf *>(node);
		}
	};
}
//
// 	template <auto Total, auto DivNum>
// 	constexpr
// 	auto Cal()
// 	{
// 		if constexpr (constexpr auto average = Total / DivNum; auto remainder = Total % DivNum == 0) {
// 			// make a seq that contains DivNum copies average
// 		} else {
// 			// make a seq that contains DivNum copies average, too
// 			// allocate the remainder to each item in part of seq in suitable way
// 		}
//
// 	}
// }
//
// template <int Element>
// struct GetEle
// {
// 	static constexpr auto get(int n)
// 	{
// 		return Element;
// 	}
// };
//
// template <uint32_t Num, int Item, size_t... I>
// constexpr auto
// ImpDupItem(index_sequence<I...>)
// {
// 	return integer_sequence<uint32_t, GetEle<Item>::get(I)...>();
// }
//
// template <uint32_t Num, int Item>
// constexpr auto
// DupEle()
// {
// 	constexpr auto indexs = make_index_sequence<Num>();
// 	return ImpDupItem<Num, Item, decltype(indexs)>(indexs);
// }
//
// int main()
// {
// 	auto eles = DupEle<3, 4>();
// 	using T = decltype(eles);
// 	static_assert(!is_same_v<T, integer_sequence<int, 4, 4, 4>>, "Not same");
// }
