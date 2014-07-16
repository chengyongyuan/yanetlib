#ifndef YANETLIB_NET_BUFFER_H
#define YANETLIB_NET_BUFFER_H

#include <assert.h>

//A simple link list based buffer template class.
//

namespace yanetlib {
namespace net {

template <int BUF_NODE_SZ = 6536>
class Buffer {
 public:
     struct BufferNode {
         char data[BUF_NODE_SZ];
         int  used;
         struct BufferNode* next;
     };

     Buffer();

     ~Buffer();

     //Init Buffer
     //nodecnt specify how many node for Buffer Node
     //nodecnt must >= 1
     //RETURN: true: success, false: fail
     bool InitBuffer(int nodecnt);

     //Get data buffer ptr and length for data
     //if it ok, put addr of first available data
     //int buf, length of available data in len
     //RETURN: true: success, false: fail
     bool GetData(char *& buf, int & len);

     //Get fre buffer ptr and length for data.
     //if it ok, put addr of first unused data
     //int buf, length of available data in len
     //RETURN: true: success, false: fail
     bool GetSpace(char *& buf, int & len);

     //move the write data ptr forword and
     //push data to buffer
     void PushData(int len);

     //move the read data ptr forword and
     //pop data from buffer
     void PopData(int len); 
 private:
     BufferNode* _head, *_tail;
     BufferNode* _write_node;
     int _read_pos;
};

template <int BUF_NODE_SZ>
Buffer<BUF_NODE_SZ>::Buffer() :
    _head(NULL), _tail(NULL), _write_node(NULL),
    _read_pos(0) { }

template <int BUF_NODE_SZ>
Buffer<BUF_NODE_SZ>::~Buffer() {
    BufferNode* cur = _head;

    while(cur) {
        _head = _head->next; 
        free(cur);
        cur = _head;
    }
}

template <int BUF_NODE_SZ>
bool Buffer<BUF_NODE_SZ>::InitBuffer(int nodecnt) {
    BufferNode* pre = NULL;
    BufferNode *cur = NULL;

    assert(nodecnt >= 1);

    if ((cur = (BufferNode*) malloc(sizeof(BufferNode))) == NULL)
        return false;
    cur->used = 0;
    cur->next = NULL;
    _head = cur;

    pre = _head;
    for (int i = 1; i < nodecnt; ++i) {
        if ((cur = (BufferNode*) malloc(sizeof(BufferNode))) == NULL)
            return false;
        cur->used = 0;
        cur->next = NULL;
        pre->next = cur;
        pre = cur;
    }
    _tail = cur;
    _read_pos = 0;
    _write_node = _head;
    
    return true;
}

//Get Data from head
template <int BUF_NODE_SZ>
bool Buffer<BUF_NODE_SZ>::GetData(char *& buf, int & len) {
    //empty yet.
    if (_head == _write_node && _read_pos == _write_node->used) {
        len = 0;
        return false;
    }
    len = _head->size - _read_pos;
    buf = _head->data + _read_pos;
}

template <int BUF_NODE_SZ>
bool Buffer<BUF_NODE_SZ>::GetSpace(char *& buf, int & len) {
    //full 
    if (_write_node == _tail && _write_node->used == BUF_NODE_SZ) {
        len = 0;
        return false;
    }
    len = BUF_NODE_SZ - _write_node->used;
    buf = _write_node->data + _write_node->used;
    return true;
}

template <int BUF_NODE_SZ>
void Buffer<BUF_NODE_SZ>::PushData(int len) {
    _write_node->used += len;
    if (_write_node->used == BUF_NODE_SZ && _write_node != _tail) {
        //move buffer to next node
        _write_node = _write_node->next;
        printf("DEBUG: MOVE TO NEXT NODE\n");
    }
}

template <int BUF_NODE_SZ>
void Buffer<BUF_NODE_SZ>::PopData(int len) {
    _read_pos += len;
    //move empty head node to tail
    if (_read_pos == _head->used && _head != _write_node) {
        BufferNode* cur = _head;
        _head = cur->next;
        cur->next = _tail->next;
        _tail = cur;
        cur->used = 0;
        cur->next = NULL;
        if (_head == NULL) {//only one chunk node
            _tail = _head;
            return ;
        }
        printf("DEBUG: MOVE HEAD\n");
    } // else, Next Get Data will fail
}

} // namespace net
} // namespace yanetlib

#endif //buffer.hpp
