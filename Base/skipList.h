#ifndef XINDB_MEM_SKIPLIST_H
#define XINDB_MEM_SKIPLIST_H


#include "arenaa.h"

#include <atomic>
#include <random>
#include <cassert>
#include <cstdlib>

namespace xindb {

template<typename Key, class Comparator>
class SkipList {
private:
    struct Node;            //内部的node类

public:
    //跳表需要有一个用于比较的类， 和内存的分配器
    explicit SkipList(Comparator cmp, Arena* arena);

    //Refuse copy behavior 
    SkipList(const SkipList&) = delete;
    SkipList& operator =(const SkipList&) = delete;

    //need: 没有相同的部分
    void Insert(const Key& key);

    //Return true iff equls to key in the list 
    bool Contains(const Key& key) const;


    //Iterator
    class Iterator {
    public:
        explicit Iterator(const SkipList* list);        //明确是所属于哪一个跳表
        
        //node是否是合法的
        bool Valid() const;

        //Return key at the position， 返回迭代器指向的node节点的值
        const Key& key() const;

        //Advance to next position 
        void Next();

        //Advance to prev position 
        void Prev();

        //前进寻找第一个大于等于target 的node
        void Seek(const Key& target);

        //List 的入口
        void SeekForFirst();

        void SeekForLast();

    private:    
        const SkipList* list_;
        Node* node_;    
    };


private:
    static const int kMaxHeight = 12;


//Functions
    //不保证并发性质的读取height
    inline int GetMaxGeight() const {
        return max_height_.load(std::memory_order_relaxed);
    }

    bool Equal(const Key& a, const Key& b) const {
        return (compare_(a, b) == 0);
    }


    //Return true if key > node->value 
    bool KeyIsAfterNode(const Key& key, Node* node) const;

    Node* FindgreaterOrEqual(const Key& key, Node** prev) const;

    Node* FindLessThan(const Key& key) const;

    Node* FindLast() const;

    Node* NewNode(const Key& key, int height);


//Varaiable 
    Comparator const compare_;

    Arena* const arena_;        //用于进行node的内存的分配

    Node* const header_;

    int RandomHeight();

    std::atomic<int> max_height_;

};

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
  list_ = list;
  node_ = nullptr;
}



template<typename Key, class Comparator> 
inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
    return node_ != nullptr;
}

template<typename Key, class Comparator> 
inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
    assert(Valid());    //Ensure the node is valid
    return node_->key;
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Next() {
    assert(Valid());
    node_ = node_->Next(0);         //最底层的指针的变换
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Prev() {
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if (node_ == list_->header_){
        node_ = nullptr;
    }
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
    //seek for the node that greater than target
    node_ = list_->FindgreaterOrEqual(target, nullptr);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekForFirst() {
    node_ = list_->header_->Next(0);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekForLast() {
    node_ = list_->FindLast();
    if (node_ == list_->header_) {
        node_ = nullptr;
    }
}


template<typename Key, class Comparator> 
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& k) :key(k) {}

    Key const key;              //Node节点的数值

    Node* Next(int n) {
        assert(n >= 0);
        return next_[n].load(std::memory_order_acquire);
    }

    //Anyone read can observe the full version 
    void SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].store(x, std::memory_order_release);
    }

    //no barrier: read next node 
    Node* NoBarrier_Next(int n) {
        assert(n >= 0);
        return next_[n].load(std::memory_order_relaxed);
    }

    //no barrier: store the node
    void NoBarrier_SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].store(x, std::memory_order_relaxed);
    }


private:
    
    //next[0] 就是最低层次的指针, 数组的值就是对应的层次的高度, 柔性指针的技巧
    std::atomic<Node*> next_[1];

};



template<typename Key, class Comparator> 
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
    //因为最底层Node节点已经有了node* ，因此还是需要height -1 个
    char* const node_mem = arena_->AllocateAligned(sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
    return new(node_mem) Node(key);
} 



template<typename Key, class Comparator> 
SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena) 
    : compare_(cmp), arena_(arena), header_(NewNode(0, kMaxHeight)), max_height_(1) {
        //初始化所有的next
        for (int i = 0; i < kMaxHeight; i++) {
            header_->SetNext(i, nullptr);
        }
    }



//获得随机的高度，随机进行高度的增长，最高12层
template<typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
    //1/4的概率进行高度的增长
    static const unsigned int kBranching = 4;

    int height = 1;
    //height的增长概率是四分之一
    while (height < kMaxHeight && (rand() % kBranching == 0)) {
        height++;
    }
    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
}



template<typename Key, class Comparator> 
void SkipList<Key, Comparator>::Insert(const Key& key) {
    //初始化一下prev
    Node* prev[kMaxHeight];

    //查找节点并且将prev安置好
    Node* node = FindgreaterOrEqual(key, prev);
    //不能够进行重复的插入操作
    assert(node == nullptr || !Equal(key, node->key));

    int insertHeight = RandomHeight();

    //如果随机的高度比当前最大的节点还是要大的，更新一下层数
    if (insertHeight > GetMaxGeight()) {
        //update prev node 
        for (int i = GetMaxGeight(); i < insertHeight; i++) {
            prev[i] = header_;
        }

        //不需要进行加读锁，leveldb中的特殊的性质决定如此
        max_height_.store(insertHeight, std::memory_order_relaxed);
    }   

    //进行插入操作
    node = NewNode(key, insertHeight);
    for (int i = 0; i < insertHeight; i++) {

        //no barrier 
        node->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
        //here will ensure sync
        prev[i]->SetNext(i, node);
    }
}



template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
    //小于给定key的节点
    Node* pNode = header_;
    int level = GetMaxGeight() - 1;
    while(true) {
        //要么是头，要么是小于的
        assert(pNode == header_ || compare_(pNode->key, key) < 0);
        Node* next = pNode->Next(level);
        if (compare_(next->key, key) >= 0) {
            //advance to low hright
            if (level == 0) {
                return pNode;
            }else {
                level--;
            }
        }else {
            //advance to next node 
            pNode = next;
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
    Node* pNode = header_;
    int level = GetMaxGeight() - 1;
    while (true) {
        Node* next = pNode->Next(level);
        if (next == nullptr) {
            if (level == 0) {
                return pNode;
            }else {
                level--;
            }
        } else {
            pNode = next;
        }
    }
}


//每层查找最后一个小于Key的节点
template<typename Key, class Comparator> 
typename SkipList<Key, Comparator>::Node*               //typename 用在这里来指明后面接的是一种类型
SkipList<Key, Comparator>::FindgreaterOrEqual(const Key& key, Node** prev) const {
    //双重指针就相当于单指针的数组
    Node* pNode = header_;               //begin from header node 
    int level = GetMaxGeight() - 1;
    while (true) {
        Node* next = pNode->Next(level);    

        if (KeyIsAfterNode(key, next)) {        //如果value大于node的节点值
            //那么就继续在这一层次进行寻找
            pNode = next;                       //移动一下索引的路径node
        } else {
            //如果不是的话，那么就要下降层次
            if (prev != nullptr) prev[level] = pNode;       //维护好前面的节点，用于插入

            if (level == 0) {
                return next;
            } else {
                level --;
            }
        }
    }
}

//判断Key的值是不是大于node的值
template<typename Key, class Comparator> 
bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* node) const {
    return (node != nullptr && compare_(node->key, key) < 0);
}


template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) const {
    Node* tmp = FindgreaterOrEqual(key, nullptr);       
    if (tmp != nullptr && Equal(key, tmp->key)) {
        return true;
    }
    return false;
}




}   // namespace xindb


#endif