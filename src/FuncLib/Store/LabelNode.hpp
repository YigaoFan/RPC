#pragma once
#include <vector>
#include <optional>
#include "StaticConfig.hpp"
#include "../../Btree/Enumerator.hpp"
#include "VectorUtil.hpp"
#include "PosLabelNodeConcept.hpp"
#include "LabelNodeBase.hpp"

namespace FuncLib::Store
{
	using ::std::optional;
	using ::std::vector;

	class LabelNode : private LabelNodeBase<LabelNode>
	{
	private:
		using Base = LabelNodeBase<LabelNode>;

	public:
		static LabelNode ConsNodeWith(PosLabelNode auto* labelNode)
		{
			vector<LabelNode> subNodes;

			auto e = labelNode->CreateSortedSubNodeEnumerator();
			while (e.MoveNext())
			{
				subNodes.push_back(ConsNodeWith(&e.Current()));
			}

			return LabelNode(labelNode->Label(), move(subNodes));
		}

		LabelNode(pos_label label);
		LabelNode(pos_label label, vector<LabelNode> subNodes);

		using Base::AddSub;
		using Base::CreateSortedSubNodeEnumerator;
		using Base::Label;
		void SetSubs(vector<LabelNode> subNodes);
		vector<LabelNode> GiveSubs();
		bool SubsEmpty() const noexcept;
		/// 由于是 inside 的，所以不检查当前 node 的 label
		optional<LabelNode> TakeInside(pos_label label);
		// 这样哪些需要 release 是不是就不用那个 toDoDelete set 来记了？还需要，有的没 Store 就要 release
		// 那这里就要有某种方法标记它已经 Store 了，不要让那个 set 来 release 了
	
		template <typename Releaser>
		void ReleaseAll(Releaser const& releaser)
		{
			releaser(_label);
			for (auto& n : _subNodes)
			{
				n.ReleaseAll(releaser);
			}

			_subNodes.clear();
		}
	};	
}