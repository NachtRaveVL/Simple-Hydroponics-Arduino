#ifndef ARX_CONTAINER_H
#define ARX_CONTAINER_H
#define ARX_HAVE_LIBSTDCPLUSPLUS 201703L
#define ARX_VECTOR_DEFAULT_SIZE 8
#define ARX_MAP_DEFAULT_SIZE 8
#include <vector>
#include <map>
#include <utility>
namespace arx { template<class T,size_t N=8> using vector=std::vector<T>; template<class K,class V,size_t N=8> using map=std::map<K,V>; template<class A,class B> using pair=std::pair<A,B>; }
#endif
