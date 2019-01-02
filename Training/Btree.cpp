// todo: should in implementation file includes header file? 
#include <algorithm>
#include "Btree.h"
using namespace btree;
using std::initializer_list;
using std::pair;
using std::function;
using std::sort;
using std::shared_ptr;
using std::vector;
// seem like the template-type-para is not useful in this file
#define BTREE_TEMPLATE_DECLARATION template <typename Key, typename Value, typename Compare, unsigned BtreeOrder>
#define BTREE_INSTANCE Btree<Key, Value, Compare, BtreeOrder>

// public method part:

BTREE_TEMPLATE_DECLARATION
BTREE_INSTANCE::Btree(Compare compare_function,
        const initializer_list<pair<Key, Value>> pair_list)
: compare_function_(compare_function)
{
	for (auto& pair : pair_list) {
        this->add(pair);
	}
}

BTREE_TEMPLATE_DECLARATION
BTREE_INSTANCE::~Btree()
{

}

BTREE_TEMPLATE_DECLARATION
Value
BTREE_INSTANCE::search(const Key key)
{
    shared_ptr<node_instance_type> node = this->check_out(key);
    if (node->is_leaf()) {
        // todo: should implement []
        // should ensure copy, not reference
        // but (*node)[key] is reference
        return (*node)[key];
    } else {
        // todo: return nullptr or Value();
        return nullptr;
    }
}

BTREE_TEMPLATE_DECLARATION
int
BTREE_INSTANCE::add(const pair<Key, Value> e)
{
    shared_ptr<node_instance_type> node = this->check_out(e.first);
    // todo: node->add should return int
    return node->add(e);
    // if node is a leaf, then add into leaf
    // or it's a intermediate node, basically you can
    // be sure to create a new node
}

BTREE_TEMPLATE_DECLARATION
int
BTREE_INSTANCE::modify(const pair<Key, Value> pair)
{
    shared_ptr<node_instance_type> node = this->check_out(pair.first);
    (*node)[pair.first] = pair.second;
    return 1;
}

BTREE_TEMPLATE_DECLARATION
vector<Key>
BTREE_INSTANCE::explore()
{
    vector<Key> key_collection;

    this->traverse_leaf([key_collection](shared_ptr<node_instance_type> node)
    {
        for (auto && ele : *node) {
            key_collection.push_back(ele.key);
        }
        return false;
    });

    return key_collection;
}

BTREE_TEMPLATE_DECLARATION
void
BTREE_INSTANCE::remove(const Key key)
{
    shared_ptr<node_instance_type> node = this->check_out(key);
    // todo: should implement the Node method remove
    node->remove(key);
}

// private method part:
// todo: private method should declare its use 

//BTREE_TEMPLATE_DECLARATION
//void
//BTREE_INSTANCE::adjust()
//{
//
//}

/// search the key in Btree, return the node that terminated, 
/// maybe not find the key-corresponding one, but the related one.
/// save the search information
BTREE_TEMPLATE_DECLARATION
shared_ptr<typename BTREE_INSTANCE::node_instance_type>
BTREE_INSTANCE::check_out(const Key key)
{
    if (this->root_->is_leaf()) {
        return this->root_;
    } else {
        return check_out_recur_helper(key, this->root_);
    }
}

BTREE_TEMPLATE_DECLARATION
shared_ptr<typename BTREE_INSTANCE::node_instance_type> 
BTREE_INSTANCE::check_out_recur_helper(const Key key, shared_ptr<node_instance_type> node)
{
    // could think how to remove the key parameter after the first call
    shared_ptr<node_instance_type> current_node = node;
    if (current_node->is_leaf()) {
        return current_node;
    } else {
        // traverse the Ele in node
        for (auto && ele : *current_node) {
            if (compare_function_(key, ele.child_value_low_bound)) {
                return check_out_recur_helper(key, ele.child);
            }
        }
        return current_node;
    }
}

/// operate on the true Node, not the copy
BTREE_TEMPLATE_DECLARATION
vector<typename BTREE_INSTANCE::node_instance_type>
BTREE_INSTANCE::traverse_leaf(predicate predicate) 
{
	vector<node_instance_type> result;
	for (shared_ptr<node_instance_type> current_node = this->get_smallest_leaf_back(); 
		current_node->next_brother != nullptr; 
		current_node = current_node->next_brother) {
        // todo: how to predicate a Node? Or should use Ele
		if (predicate(current_node)) {
			result.push_back(current_node);
		}
	}

    return result;
}

BTREE_TEMPLATE_DECLARATION
shared_ptr<typename BTREE_INSTANCE::node_instance_type>
BTREE_INSTANCE::get_smallest_leaf_back()
{
    shared_ptr<node_instance_type> current_node = this->root_;
    for (;
        (!current_node->is_leaf()) && (*current_node)[0].child != nullptr;
        current_node = (*current_node)[0].child);

    return current_node;
}

