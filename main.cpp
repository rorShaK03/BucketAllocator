#include <iostream>
#include <vector>
#include <set>

template <typename T>
class bucket_allocator {
    struct bucket {
        void* ptr;
        size_t sz;
        size_t n_alloc;
        size_t n_dealloc;
        std::set<void*> objs;
    };
public:
    typedef T value_type;
    explicit bucket_allocator(const size_t threshold_size = 1024) : _thr_sz(threshold_size) {}
    bucket_allocator(const bucket_allocator& other) = delete;
    bucket_allocator& operator= (const bucket_allocator& other) = delete;
    ~bucket_allocator() = default;

    T* allocate(const size_t n) {
        if(n > _thr_sz) {
            _buckets.push_back(bucket{operator new[] (n * sizeof(T)), n, n, 0});
            _buckets.back().objs.insert(_buckets.back().ptr);
            return reinterpret_cast<T*>(_buckets.back().ptr);
        }
        for(bucket& b : _buckets) {
            if(b.sz - b.n_alloc >= n) {
                b.n_alloc += n;
                b.objs.insert(reinterpret_cast<T*>(b.ptr) + (b.n_alloc - n));
                return reinterpret_cast<T*>(b.ptr) + (b.n_alloc - n);
            }
        }
        _buckets.push_back(bucket{operator new[] (_thr_sz * sizeof(T)), _thr_sz, n, 0});
        _buckets.back().objs.insert(_buckets.back().ptr);
        return reinterpret_cast<T*>(_buckets.back().ptr);
    }

    void deallocate(T* ptr, size_t n) {
        for(auto it = _buckets.begin(); it != _buckets.end(); it++) {
            if(it->objs.count(ptr) > 0) {
                it->n_dealloc += n;
                it->objs.erase(ptr);
                if(it->n_dealloc == it->sz) {
                    operator delete[] (it->ptr);
                    _buckets.erase(it);
                }
                break;
            }
        }
    }
private:
    size_t _thr_sz;
    std::vector<bucket> _buckets;
};

int main() {
    std::allocator<int> a;
    bucket_allocator<int> b;
    int* arr0 = b.allocate(10);
    int* arr1 = b.allocate(10);
    int* arr2 = b.allocate(10);
    int* arr3 = b.allocate(10);
    int* arr4 = b.allocate(10);
    for(int i = 0; i < 10; i++)
        arr0[i] = i;
    for(int i = 0; i < 10; i++)
        arr1[i] = i;
    for(int i = 0; i < 10; i++)
        arr2[i] = i;
    for(int i = 0; i < 10; i++)
        arr3[i] = i;
    for(int i = 0; i < 10; i++)
        std::cout << arr3[i] << ' ';
    return 0;
}
