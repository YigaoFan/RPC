#include <utility>
#include "Node.h"

using namespace btree;
using std::shared_ptr;
using std::make_shared;
using std::pair;
#define NODE_TEMPLATE_DECLARATION template <typename Key, typename Value, unsigned BtreeOrder, typename BtreeType>
#define NODE_INSTANCE Node<Key, Value, BtreeOrder, BtreeType>

// the below function's scope is leaf
//void
//Node::exchangeTheBiggestEleOut(shared_ptr<Ele> e)
//{
//	// also need to think the intermediate_node situation
//	// doesn't exist up situation todo: think it!
//    this->doInsert(e);
//    auto& eleVectorRef = this->getVectorOfElesRef();
//	*e = *eleVectorRef.back();
//	eleVectorRef.pop_back();
//}
//
//shared_ptr<Node::Ele>
//Node::constructToMakeItFitTheFatherInsert(shared_ptr<Ele> e)
//{
//	// construct Ele Pointing To Node Packaging This Ele
//	shared_ptr<Node>&& node = make_shared<Node>(intermediate_node, e);
//	return make_shared<Ele>(e->key, node);
//}
//
//// external caller should ensure the insert node is leaf
//RESULT_FLAG
//Node::insert(shared_ptr<Ele> e)
//{
//	if (!this->isFull()) {
//		// todo: do_insert need to attention to this Node is leaf or not
//		this->doInsert(e);
//		return 0;
//	} else {
//		this->exchangeTheBiggestEleOut(e);
//		if (this->nextBrother != nullptr) {
//			return this->nextBrother->insert(e);
//		} else {
//			// package this leaf to a withe
//			shared_ptr<Ele> newEle = constructToMakeItFitTheFatherInsert(e);
//			if (this->father != nullptr) {
//				// todo: usually when move to father to insert Ele,
//				// todo: it's the end of this level. Think it!
//				return this->father->insert(newEle);
//			} else {
//				// create new branch
//				return createNewRoot(this->father, newEle);
//			}
//		}
//	}
//}
//
//
//void
//Node::doInsert(shared_ptr<Ele> e)
//{
//    auto& eleVectorRef = this->getVectorOfElesRef();
//    eleVectorRef.push_back(e);
//	std::sort(eleVectorRef.begin(), eleVectorRef.end(),
//	        [] (shared_ptr<Ele> a, shared_ptr<Ele> b)
//            {
//	            return a->key < b->key;
//            });
//}
//
//// todo: how to ensure only the root 
//// todo: have and can trigger this method
//RESULT_FLAG
//Node::createNewRoot(const shared_ptr<Node>& oldRoot, const shared_ptr<Ele>& risingEle)
//{
//	// todo: maybe the argument below is wrong, the keyType, not the bool_
//	shared_ptr<Ele>&& ele = make_shared<Ele>(/*intermediate_node*/, oldRoot);
//	shared_ptr<Node>&& node = make_shared<Node>(/*intermediate_node*/, risingEle);
//
//}

// public method part:

NODE_TEMPLATE_DECLARATION
NODE_INSTANCE::Node(const bool& middle, const BtreeType* btree) 
:middle(middle), btree_(btree)
{

}

NODE_TEMPLATE_DECLARATION
NODE_INSTANCE::Node(const bool& middle, const BtreeType* btree, const pair<Key, Value>& pair) : Node(middle, btree)
{
    // todo: process the pair
    //this->elements_.push_back(e);
}

NODE_TEMPLATE_DECLARATION
NODE_INSTANCE::~Node()
{
}

//for constructing iterator
NODE_TEMPLATE_DECLARATION
NodeIter<typename NODE_INSTANCE::ele_instance_type>
NODE_INSTANCE::begin()
{
    return NodeIter<ele_instance_type>(&elements_[0]);
}

NODE_TEMPLATE_DECLARATION
NodeIter<typename NODE_INSTANCE::ele_instance_type>
NODE_INSTANCE::end()
{
    return NodeIter<ele_instance_type>(&elements_[elements_count_]);
}

/// may return nullptr when not found
NODE_TEMPLATE_DECLARATION
// I don't know exactly whether I could use ele_instance_type like this.
shared_ptr<typename NODE_INSTANCE::ele_instance_type>
NODE_INSTANCE::operator[](Key k)
{
    for (ele_instance_type& ele : *this) {
        if (ele->key == k) {
            return make_shared<ele_instance_type>(ele);
        }
    }
    return nullptr;
}

// if node is a leaf, then add into leaf
// or it's a intermediate node, basically you can
// be sure to create a new node
NODE_TEMPLATE_DECLARATION
RESULT_FLAG
NODE_INSTANCE::add(const pair<Key, Value>& pair)
{
    // Fan said: We focus the operation of so many data structure on one abstract data structure

    // Because only one leaf node is generated in the Btree, 
    // all the nodes are actually generated here.
    if (this->middle) {
        return this->intermediate_node_add(pair);
    } else {
        // add Ele, there is only one situation--
        if (this->is_full()) {
            return this->leaf_full_then_add(pair);
        } else {
            return this->leaf_has_area_add(pair);
        }
    }
}

/// for the upper level Btree::remove, so
NODE_TEMPLATE_DECLARATION
void
NODE_INSTANCE::remove(const Key& key)
{
    if (this->middle) {
        // when not a leaf-node, no need to remove. 
        // the Btree::check_out ensure the correctness.
        return;
    } else {
        if ((*this)[key] != nullptr) {
            // do some memory copy
            --(this->elements_count_);
            return;
        } else {
            // key isn't exist in this Node
            return;
        }
    }
}

// private method part:

NODE_TEMPLATE_DECLARATION
RESULT_FLAG
NODE_INSTANCE::leaf_full_then_add(pair<Key, Value> pair)
{
    // todo: if key is the same, cover the old value
    // need to think initialization about temp
    NodeIter<ele_instance_type> endIter = this->end();
    // todo: care here is rvalue reference and modify other place
    for (NodeIter<ele_instance_type>&& iter = this->begin(); iter != endIter; ++iter) {
        if (compare(pair.first, iter->key)) {
            // save a copy of the end ele_instance_type
            ele_instance_type temp = *endIter;
            // todo: need to implement the operator-()
            this->move_Ele(iter, endIter - 1, 1);
            *iter = pair;
            // todo: call another way to process the temp ele_instance_type
        }
    }
}

NODE_TEMPLATE_DECLARATION
RESULT_FLAG
NODE_INSTANCE::leaf_has_area_add(const pair<Key, Value>& pair)
{
    NodeIter<ele_instance_type> end = this->end();

    for (NodeIter<ele_instance_type> iter = this->begin();
         iter != end;
         ++iter) {
        // once the pair.key < e.key, arrive the insert position
        if (compare(pair.first, iter->key)) {
            this->move_Ele(iter, this->end(), 1);
            iter->key = pair.first;
            iter->data = pair.second;
            ++(this->elements_count_);
            return OK;
        }
    }
}

NODE_TEMPLATE_DECLARATION
RESULT_FLAG
NODE_INSTANCE::intermediate_node_add(const pair<Key, Value>& pair)
{
    
}

NODE_TEMPLATE_DECLARATION
void
NODE_INSTANCE::move_Ele(const NodeIter<ele_instance_type>& begin,
        const NodeIter<ele_instance_type>& end, unsigned distance)
{
    //  memory back shift, there may be problems
    // , because relate to object function pointer address maybe not correct
    ele_instance_type* src = begin.operator->();
    // todo: should make NodeIter support operator-()
    size_t length = (end - begin) * sizeof(NodeIter<ele_instance_type>::value_type);
    memcpy(src + distance, src, length);
}