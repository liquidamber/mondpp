#ifndef MONDPP_MEMORY_H
#define MONDPP_MEMORY_H

#include <mond.h>

namespace mondpp
{

  /**
   * Pointer to global memory.
   */
  template <class ValueType>
  class global_ptr
  {
  private:
    mondptr_t ptr;

  public:
    global_ptr() = default;
    global_ptr(mondptr_t ptr) : ptr(ptr) {}
    global_ptr(std::nullptr_t) : ptr(MOND_NULL) {}

    global_ptr<ValueType> & operator=(std::nullptr_t)
    { ptr = MOND_NULL; }

    inline global_ptr<ValueType> & operator+=(size_t offset)
    { return global_ptr<ValueType>(mond_ptr_move(ptr, offset)); }

    inline global_ptr<ValueType> operator+(size_t offset) const
    {
      global_ptr<ValueType> tmp = *this;
      tmp += offset;
      return tmp;
    }

    int get(ValueType * dst, size_t byte)
    { return mond_get(dst, ptr, byte); }

    int put(ValueType * src, size_t byte)
    { return mond_put(dst, ptr, byte); }

    static int fence() const
    { return mond_fence(); }

    void alloc(size_t byte)
    { ptr = mond_malloc(byte); }

    static global_ptr<ValueType> malloc(size_t byte)
    { global_ptr<ValueType> p; p.alloc(byte); return; }

    int free()
    { return mond_free(ptr); }
  };

  /**
   * Reference to global memory.
   */
  template <class ValueType>
  class global_ref
  {
  private:
    const global_ptr<ValueType> ptr;

  public:
    explicit global_ref(global_ptr<ValueType> ptr) : ptr(ptr) {}

    operator ValueType() const
    {
      ValueType tmp;
      ptr.get(&tmp, sizeof(ValueType));
      return tmp;
    }

    ValueType operator=(ValueType val)
    {
      ptr.put(&val, sizeof(ValueType));
      return val;
    }
  };

  /**
   * global variable.
   */
  template <class ValueType>
  class global_var
  {
  private:
    global_ref<ValueType> ref;

  public:
    global_var()
      : ref(global_ptr<ValueType>::malloc(sizeof(ValueType))) {}

    global_var(ValueType var)
      : ref(global_ptr<ValueType>::malloc(sizeof(ValueType)))
    { ref = var; }

    // global_var(const global_var<ValueType> & other)
    //   : ref(global_ptr<ValueType>::malloc(sizeof(ValueType)))
    // { ref = static_cast<ValueType>(other.ref); }

    global_var(global_var<ValueType> && other)
    { std::swap(ref, other.ref); }

    /* Assign operation don't have to overwritten, maybe. */
    // global_var<ValueType> & operator=(const global_var<ValueType> & x);
    // global_var<ValueType> & operator=(global_var<ValueType> && x);

    global_var<ValueType> & operator=(ValueType val)
    { ref = val; return *this; }

    operator ValueType() const
    { return ref; }
  };

  /**
   * Static length array to global memory.
   */
  template <class ValueType>
  class global_array
  {
  private:
    global_ptr<ValueType> ptr;
    const size_t length;

  public:
    explicit global_array(size_t length) : length(length)
    { ptr = global_ptr<ValueType>::malloc(sizeof(ValueType)*length); }

    global_array(const global_array<ValueType> & other)
      : length(length)
    {
      ptr = global_ptr<ValueType>::malloc(sizeof(ValueType)*length);
      std::array<ValueType, array.size()> temp;
      other.ptr.get(&temp[0], sizeof(ValueType)*array.size());
      this->ptr.put(&temp[0], sizeof(ValueType)*array.size());
    }

    global_array(global_array<ValueType> && other)
    { std::swap(this->ptr, other.ptr); }

    const global_ref<ValueType> operator[](size_t index) const
    { return global_ref<ValueType>(ptr+index); }

    global_ref<ValueType> operator[](size_t index)
    { return global_ref<ValueType>(ptr+index); }
  };
}

#endif // MONDPP_MEMORY_H
