#ifndef __SF_SINGLETON_H__
#define __SF_SINGLETON_H__

///
/// Generic singleton class
///
template <typename T>
class SfSingleton {
  public:
    static T & Get() {
        // XXX: Non-thread safe
        if (!_instance) {
            _instance = new T();
        }
        return *_instance;
    }

  private:
    static T *_instance;
};

/// Instantiate this in the cpp file
#define SF_INSTANTIATE_SINGLETON(Type)          \
    template<>                                  \
    Type *SfSingleton<Type>::_instance = NULL

#endif // __SF_SINGLETON_H__
