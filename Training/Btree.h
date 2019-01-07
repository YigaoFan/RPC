#pragma once

#include <vector> // for vector
#include <functional> // for lambda
#include <initializer_list> // for initializer_list
#include <memory> // for shared_ptr
#include <utility> // for pair
#include "CommonFlag.h"
#include "Node.h"

namespace btree {
    // should provide the default Compare type. Learn the less<arg>.
    template<typename Key, 
             typename Value,
             unsigned BtreeOrder,
             typename Compare = std::function<bool(const Key&, const Key&)>>
    class Btree {
    private:
        using node_instance_type = Node<Key, Value, BtreeOrder, Btree>;
        using predicate = std::function<bool(std::shared_ptr<node_instance_type>)>;
        // Friend
        friend node_instance_type;

    public:
        Btree(Compare&, const std::initializer_list<std::pair<Key, Value>>&);
        ~Btree();
        Value search(const Key&);
        RESULT_FLAG add(const std::pair<Key, Value>&);
        RESULT_FLAG modify(const std::pair<Key, Value>&);
        std::vector<Key> explore();
        void remove(const Key&);

    private:
        // Field
        std::shared_ptr<node_instance_type> root_ = std::make_shared<node_instance_type>(false, this);
        const Compare compare_func_;

        std::shared_ptr<node_instance_type> check_out(const Key&);
        std::shared_ptr<node_instance_type> check_out_recur(const Key&, 
            const std::shared_ptr<node_instance_type>&);
        // provide some all-leaf-do operation
        std::vector<node_instance_type> traverse_leaf(const predicate&);
        std::shared_ptr<node_instance_type> smallest_leaf_back();
    };
}
