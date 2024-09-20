#ifndef YBUF_H
#define YBUF_H

#include <algorithm>
#include <string.h>
#include <stdlib.h>
typedef struct YBuf_s {
    char*  base{nullptr};
    size_t len{0};

    YBuf_s() {
        base = nullptr;
        len  = 0;
    }

    YBuf_s(void* data, size_t len) {
        this->base = (char*)data;
        this->len  = len;
    }

} YBuf_t;

typedef struct offset_buf_s {
    char*   base;
    size_t  len;
    size_t  offset;

    offset_buf_s() {
        base = nullptr;
        len = offset = 0;
    }

    offset_buf_s(void* data, size_t len) {
        this->base = (char*)data;
        this->len = len;
    }

} offset_buf_t;


class YBuf : public YBuf_t {
public:
    YBuf() : YBuf_t() {
        cleanup_ = false;
    }
    YBuf(void* data, size_t len) : YBuf_t(data, len) {
        cleanup_ = false;
    }
    YBuf(size_t cap) { resize(cap); }

    virtual ~YBuf() {
        cleanup();
    }

    void*  data() { return base; }
    size_t size() { return len; }

    bool isNull() { return base == nullptr || len == 0; }

    void cleanup() {
        if (cleanup_) {
            free(base);
            base = nullptr;
            len = 0;
            cleanup_ = false;
        }
    }

    void resize(size_t cap) {
        if (cap == len) return;
        if (base == nullptr) {
            base = (char*)malloc(cap);
        }
        else {
            base = (char*)realloc(base, cap);
        }
        len = cap;
        cleanup_ = true;
    }

    void copy(void* data, size_t len) {
        resize(len);
        memcpy(base, data, len);
    }

    void copy(YBuf_t* buf) {
        copy(buf->base, buf->len);
    }
private:
    bool cleanup_;
};

// VL: Variable-Length
class YLBuf : public YBuf {
public:
    YLBuf() : YBuf() {_offset = _size = 0;}
    YLBuf(void* data, size_t len) : YBuf(data, len) {_offset = 0; _size = len;}
    YLBuf(size_t cap) : YBuf(cap) {_offset = _size = 0;}
    virtual ~YLBuf() {}

    char* data() { return base + _offset; }
    size_t size() { return _size; }

    void push_front(void* ptr, size_t len) {
        if (len > this->len - _size) {
            size_t newsize = std::max(this->len, len)*2;
            base = (char*)realloc(base, newsize);
            this->len = newsize;
        }

        if (_offset < len) {
            // move => end
            memmove(base+this->len-_size, data(), _size);
            _offset = this->len-_size;
        }

        memcpy(data()-len, ptr, len);
        _offset -= len;
        _size += len;
    }

    void push_back(void* ptr, size_t len) {
        if (len > this->len - _size) {
            size_t newsize = std::max(this->len, len)*2;
            base = (char*)realloc(base, newsize);
            this->len = newsize;
        }
        else if (len > this->len - _offset - _size) {
            // move => start
            memmove(base, data(), _size);
            _offset = 0;
        }
        memcpy(data()+_size, ptr, len);
        _size += len;
    }

    void pop_front(void* ptr, size_t len) {
        if (len <= _size) {
            if (ptr) {
                memcpy(ptr, data(), len);
            }
            _offset += len;
            if (_offset >= len) _offset = 0;
            _size   -= len;
        }
    }

    void pop_back(void* ptr, size_t len) {
        if (len <= _size) {
            if (ptr) {
                memcpy(ptr, data()+_size-len, len);
            }
            _size -= len;
        }
    }

    void clear() {
        _offset = _size = 0;
    }

    void prepend(void* ptr, size_t len) {
        push_front(ptr, len);
    }

    void append(void* ptr, size_t len) {
        push_back(ptr, len);
    }

    void insert(void* ptr, size_t len) {
        push_back(ptr, len);
    }

    void remove(size_t len) {
        pop_front(nullptr, len);
    }

private:
    size_t _offset;
    size_t _size;
};

class YRingBuf : public YBuf {
public:
    YRingBuf() : YBuf() {_head = _tail = _size = 0;}
    YRingBuf(size_t cap) : YBuf(cap) {_head = _tail = _size = 0;}
    virtual ~YRingBuf() {}

    char* alloc(size_t len) {
        char* ret = NULL;
        if (_head < _tail || _size == 0) {
            // [_tail, this->len) && [0, _head)
            if (this->len - _tail >= len) {
                ret = base + _tail;
                _tail += len;
                if (_tail == this->len) _tail = 0;
            }
            else if (_head >= len) {
                ret = base;
                _tail = len;
            }
        }
        else {
            // [_tail, _head)
            if (_head - _tail >= len) {
                ret = base + _tail;
                _tail += len;
            }
        }
        _size += ret ? len : 0;
        return ret;
    }

    void free(size_t len) {
        _size -= len;
        if (len <= this->len - _head) {
            _head += len;
            if (_head == this->len) _head = 0;
        }
        else {
            _head = len;
        }
    }

    void clear() {_head = _tail = _size = 0;}

    size_t size() {return _size;}

private:
    size_t _head;
    size_t _tail;
    size_t _size;
};


#endif // YBUF_H
