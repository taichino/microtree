#pragma once

#include <iostream>
#include <map>
#include "picojson.h"

namespace microtree {

class treenode {
  public:
    typedef picojson::object map;
  public:
    treenode* parent;
    treenode* first_child;
    treenode* last_child;
    treenode* prev_sibling;
    treenode* next_sibling;
    std::string* key;
    map props;
  public:
    treenode();
    treenode(const std::string& nodeid);
    ~treenode();
    int depth();
};
treenode::treenode() {
    parent = first_child = last_child = prev_sibling = next_sibling = NULL;
    key = new std::string();
}
treenode::treenode(const std::string& key) {
    parent = first_child = last_child = prev_sibling = next_sibling = NULL;
    this->key = new std::string(key);
}
treenode::~treenode() {
    delete key;
}
int treenode::depth() {
    treenode* cur = this;
    int depth = 0;
    while (cur->parent) {
	depth++;
	cur = cur->parent;
    }
    return depth;
}

class tree {
    class dfs_iterator {
	friend class tree;
	
      protected:
	treenode* node_;

      public:
	dfs_iterator();
	dfs_iterator(treenode* node);
	treenode& operator*() const;
	treenode* operator->() const;
	bool operator==(const dfs_iterator& itr) const;
	bool operator!=(const dfs_iterator& itr) const;
	dfs_iterator operator++();
    };
  public:
    typedef dfs_iterator iterator;

  protected:
    treenode* head_;
    treenode* tail_;
    std::map<std::string, treenode*> nodemap_;

  public:
    tree();
    ~tree();
    
    dfs_iterator begin() const;
    dfs_iterator end() const;
    dfs_iterator insert(dfs_iterator pos, const std::string& key);
    dfs_iterator add_child(dfs_iterator pos, const std::string& key);
    dfs_iterator erase(dfs_iterator pos);
    dfs_iterator find(const std::string& key);

    enum MoveDir {
	TO_BEFORE,
	TO_AFTER,
	TO_FIRSTCHILD,
	TO_LASTCHILD
    };
    dfs_iterator move(dfs_iterator dst, dfs_iterator src, MoveDir dir);

    void dump();
};

// implementation of tree

tree::tree() {
    head_ = new treenode();
    tail_ = new treenode();

    head_->next_sibling = tail_;
    tail_->prev_sibling = head_;
}
tree::~tree() {
    if (head_->next_sibling != tail_) {
	treenode* cur = head_->next_sibling;
	treenode* next;
	do {
	    next = cur->next_sibling;
	    erase(cur);
	    cur = next;
	} while(next != tail_);
    }
    
    delete head_;
    delete tail_;
}

tree::dfs_iterator tree::begin() const {
    return dfs_iterator(head_->next_sibling);
}
tree::dfs_iterator tree::end() const {
    return dfs_iterator(tail_);
}
tree::dfs_iterator tree::insert(dfs_iterator pos, const std::string& key) {
    // if tail_ is directed by pos, replace pos with head_
    if (pos.node_ == tail_) {
	pos = dfs_iterator(head_);
    }

    // add node with key after pos
    treenode* newnode = new treenode(key);
    nodemap_.insert(std::make_pair(key, newnode));
    newnode->parent = pos->parent;
    newnode->prev_sibling = pos.node_;
    newnode->next_sibling = pos->next_sibling;
    if (pos.node_->next_sibling) {
	pos.node_->next_sibling->prev_sibling = newnode;
    }
    pos.node_->next_sibling = newnode;
    if (newnode->parent) {
	newnode->parent->last_child = newnode;
    }
    
    return dfs_iterator(newnode);
}

tree::dfs_iterator tree::add_child(dfs_iterator pos, const std::string& key) {
    treenode* newnode = new treenode(key);
    nodemap_.insert(std::make_pair(key, newnode));
    newnode->parent = pos.node_;
    if (pos->last_child) {
	pos->last_child->next_sibling = newnode;
	newnode->prev_sibling = pos->last_child;
	pos->last_child = newnode;	
    }
    else {
	pos->first_child = newnode;
	pos->last_child = newnode;
    }
    
    return dfs_iterator(newnode);
}

tree::dfs_iterator tree::erase(dfs_iterator pos) {
    if (pos.node_ == head_) { return begin(); }
    if (pos.node_ == tail_) { return end(); }
    
    // erase subtree of pos
    while (pos->first_child) {
	erase(tree::dfs_iterator(pos->first_child));
    }

    treenode* deletenode = pos.node_;
    if (!deletenode->prev_sibling) {
	deletenode->parent->first_child = deletenode->next_sibling;
    }
    else {
	deletenode->prev_sibling->next_sibling = deletenode->next_sibling;
    }
    if (!deletenode->next_sibling) {
	deletenode->parent->last_child = deletenode->prev_sibling;
    }
    else {
	deletenode->next_sibling->prev_sibling = deletenode->prev_sibling;
    }
    if (deletenode->parent) {
	if (deletenode->parent->first_child == deletenode &&
	    deletenode->parent->last_child == deletenode) {
	    deletenode->parent->first_child = NULL;
	    deletenode->parent->last_child = NULL;
	}
    }
    nodemap_.erase(*deletenode->key);
    delete deletenode;
}

tree::dfs_iterator tree::find(const std::string& key) {
    std::map<std::string, treenode*>::iterator itr = nodemap_.find(key);
    if (itr != nodemap_.end()) {
	return dfs_iterator(itr->second);
    }
    return this->end();
}

tree::dfs_iterator tree::move(dfs_iterator dst, dfs_iterator src, MoveDir dir) {
    if (dst == src) { return src; }
    if (dst->next_sibling && dst->next_sibling == src.node_) { return src; }

    // disconnect src node
    if (src->prev_sibling) {
	src->prev_sibling->next_sibling = src->next_sibling;
    }
    else {
	if (src->parent) {
	    src->parent->first_child = src->next_sibling;
	}
    }
    if (src->next_sibling) {
	src->next_sibling->prev_sibling = src->prev_sibling;
    }
    else {
	if (src->parent) {
	    src->parent->last_child = src->prev_sibling;
	}
    }

    // connect src node to dst along to dir
    if (dir == TO_AFTER) {
	if (dst->next_sibling) {
	    dst->next_sibling->prev_sibling = src.node_;
	}
	src->parent = dst->parent;	
	src->next_sibling = dst->next_sibling;
	dst->next_sibling = src.node_;
	src->prev_sibling = dst.node_;
    }
    else if (dir == TO_BEFORE) {
	if (dst->prev_sibling) {
	    dst->prev_sibling->next_sibling = src.node_;
	}
	src->parent = dst->parent;	
	src->prev_sibling = dst->prev_sibling;
	src->next_sibling = dst.node_;
	dst->prev_sibling = src.node_;
    }
    else if (dir == TO_FIRSTCHILD) {
	if (dst->first_child) {
	    dst->first_child->prev_sibling = src.node_;
	}
	else {
	    dst->last_child = src.node_;
	}
	src->parent = dst.node_;
	src->next_sibling = dst->first_child;
	dst->first_child = src.node_;
	src->prev_sibling = NULL;	
    }
    else if (dir == TO_LASTCHILD) {
	if (!dst->first_child) {
	    dst->first_child = src.node_;
	}
	if (dst->last_child) {
	    dst->last_child->next_sibling = src.node_;
	}
	src->parent = dst.node_;
	src->prev_sibling = dst->last_child;
	dst->last_child = src.node_;
	src->next_sibling = NULL;
    }

    return src;
}

void tree::dump() {
    std::cout << "=== Tree Dump ===" << std::endl;
    for (dfs_iterator itr = this->begin(); itr != this->end(); ++itr) {
	for (int d = 0; d < itr->depth(); d++) {
	    std::cout << "  ";
	}
	std::cout << *itr->key << "  " << itr->props << std::endl;
    }
    std::cout << std::endl;
}

// implementation of iterator

tree::dfs_iterator::dfs_iterator() : node_(NULL) {
}
tree::dfs_iterator::dfs_iterator(treenode* node) {
    node_ = node;
}
treenode& tree::dfs_iterator::operator*() const {
    return *node_;
}
treenode* tree::dfs_iterator::operator->() const {
    return node_;
}
bool tree::dfs_iterator::operator==(const dfs_iterator& itr) const {
    return (*node_->key == *itr.node_->key);
}
bool tree::dfs_iterator::operator!=(const dfs_iterator& itr) const {
    return (*node_->key != *itr.node_->key);
}
tree::dfs_iterator tree::dfs_iterator::operator++() {
    // traverse following order
    // firstchild => nextsibling => parent (DFS)
    if (node_->first_child) {
	node_ = node_->first_child;
    }
    else {
	while (!node_->next_sibling) {
	    node_ = node_->parent;
	    if (!node_) return *this;
	}
	node_ = node_->next_sibling;
    }
    return *this;
}

} // end of namespace microtree
