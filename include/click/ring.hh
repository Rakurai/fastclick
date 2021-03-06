// -*- c-basic-offset: 4 -*-
#ifndef CLICK_RING_HH
#define CLICK_RING_HH

#include <click/atomic.hh>

CLICK_DECLS

    template <typename T, size_t RING_SIZE> class BaseRing {

    protected:

    inline bool has_space() {
        return head - tail < RING_SIZE;
    }

    inline bool is_empty() {
        return head == tail;
    }

    public:
    int id;
        BaseRing() {
            head = 0;
            tail = 0;
        }


        inline T extract() {
            if (!is_empty()) {
                T &v = ring[tail % RING_SIZE];
                tail++;
                return v;
            } else
                return 0;
        }

        inline bool insert(T batch) {
            if (has_space()) {
                ring[head % RING_SIZE] = batch;
                head++;
                return true;
            } else
                return false;
        }

        inline unsigned int count() {
            return head - tail;
        }

        T ring[RING_SIZE];
        uint32_t head;
        uint32_t tail;


        void pool_transfer(unsigned int thread_from, unsigned int thread_to) {

        }

        inline void hint(unsigned int num, unsigned int thread_id) {

        }


    };

template <typename T, size_t RING_SIZE> class Ring : public BaseRing<T,RING_SIZE> {};

template <typename T> class CircleList {
public:
    CircleList() : _data(0), _maxsize(0),_size(0),_cur(0) {

    }

    CircleList(unsigned int max_size) : _data(0), _maxsize(max_size),_size(0),_cur(0) {

    }

    inline T& get() {
        return _data[_cur % _size];
    }

    inline T& next() {
        return _data[++_cur % _size];
    }

    inline void advance() {
        ++_cur;
    }

    void append(T v) {
        assert(_size < _maxsize);
        if (!_data)
            _data = new T[_maxsize];
        _data[_size++] = v;
    }

    unsigned int size() {
        return _size;
    }

    inline T& begin() {
        return _data[0];
    }

    inline bool is_empty() {
        return _size == 0;
    }
    private :
    T* _data;
    unsigned int _size;
    unsigned int _maxsize;
    unsigned int _cur;
};

#define SPSCRing Ring

template <typename T, size_t MAX_SIZE> class MPMCLIFO {
    SimpleSpinlock _lock;
protected:

inline bool has_space() {
    return _count < MAX_SIZE;
}

inline bool is_empty() {
   return _first == 0;
}

public:
int id;
T _first;
unsigned int _count;
    MPMCLIFO() : _lock(), id(0),_first(0),_count(0) {

    }


    inline T extract() {
        _lock.acquire();
        if (is_empty()) {
            _lock.release();
            return 0;
        }
        T tmp = _first;
        _first = _first->prev();
        _count--;
        _lock.release();
        return tmp;
    }

    inline bool insert(T v) {
        _lock.acquire();
        if (!has_space()) {
            _lock.release();
            return false;
        }
        v->set_prev(_first);
        _first = v;
        _count++;
        _lock.release();
        return true;
    }

    inline unsigned int count() {
        return _count;
    }


    void pool_transfer(unsigned int thread_from, unsigned int thread_to) {

    }

    inline void hint(unsigned int num, unsigned int thread_id) {

    }
};


template <typename T, size_t RING_SIZE> class MPMCRing : public SPSCRing<T, RING_SIZE> {
    Spinlock _lock;


public:
    inline void acquire() {
        _lock.acquire();
    }

    inline void release() {
        _lock.release();
    }

    inline void release_tail() {
        release();
    }

    inline void release_head() {
        release();
    }

    inline void acquire_tail() {
        acquire();
    }

    inline void acquire_head() {
        acquire();
    }

public:

    MPMCRing() : _lock() {

    }

    inline bool insert(T batch) {
        acquire_head();
        if (SPSCRing<T,RING_SIZE>::has_space()) {
            SPSCRing<T,RING_SIZE>::ring[SPSCRing<T,RING_SIZE>::head % RING_SIZE] = batch;
            SPSCRing<T,RING_SIZE>::head++;
            release_head();
            return true;
        } else {
            release_head();
            return false;
        }
    }

    inline T extract() {
        acquire_tail();
        if (!SPSCRing<T,RING_SIZE>::is_empty()) {
            T &v = SPSCRing<T,RING_SIZE>::ring[SPSCRing<T,RING_SIZE>::tail % RING_SIZE];
            SPSCRing<T,RING_SIZE>::tail++;
            release_tail();
            return v;
        } else {
            release_tail();
            return 0;
        }
    }
};

template <typename T, size_t RING_SIZE> class SMPMCRing : public SPSCRing<T, RING_SIZE> {
    SimpleSpinlock _lock_head;
    SimpleSpinlock _lock_tail;

    inline void release_tail() {
        _lock_tail.release();
    }

    inline void release_head() {
        _lock_head.release();
    }

    inline void acquire_tail() {
        _lock_tail.acquire();
    }

    inline void acquire_head() {
        _lock_head.acquire();
    }
public:
    inline bool insert(T batch) {
        acquire_head();
        if (SPSCRing<T,RING_SIZE>::has_space()) {
            SPSCRing<T,RING_SIZE>::ring[SPSCRing<T,RING_SIZE>::head % RING_SIZE] = batch;
            SPSCRing<T,RING_SIZE>::head++;
            release_head();
            click_compiler_fence();
            return true;
        } else {
            release_head();
            click_compiler_fence();
            return false;
        }
    }

    inline T extract() {
        acquire_tail();
        if (!SPSCRing<T,RING_SIZE>::is_empty()) {
            T &v = SPSCRing<T,RING_SIZE>::ring[SPSCRing<T,RING_SIZE>::tail % RING_SIZE];
            SPSCRing<T,RING_SIZE>::tail++;
            release_tail();
            click_compiler_fence();
            return v;
        } else {
            release_tail();
            click_compiler_fence();
            return 0;
        }
    }

};

template <typename T, size_t RING_SIZE> class MPSCRing : public SPSCRing<T, RING_SIZE> {
    SimpleSpinlock _lock_head;

    inline void release_head() {
        _lock_head.release();
    }

    inline void acquire_head() {
        _lock_head.acquire();
    }
public:

    inline bool insert(T batch) {
        acquire_head();
        if (SPSCRing<T,RING_SIZE>::has_space()) {
            SPSCRing<T,RING_SIZE>::ring[SPSCRing<T,RING_SIZE>::head % RING_SIZE] = batch;
            SPSCRing<T,RING_SIZE>::head++;
            release_head();
            return true;
        } else {
            release_head();
            return false;
        }
    }
};

CLICK_ENDDECLS
#endif
