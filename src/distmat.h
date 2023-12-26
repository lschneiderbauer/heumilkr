#include <vector>

#ifndef DISTMAT_H
#define DISTMAT_H

template <class T>
class distmat
{
public:
  distmat(int n = 0, T def = T());
  distmat(const std::vector<T> &vec);
  T &acc(const int i, const int j);
  T get(const int i, const int j) const;
  int size() const;

private:
  std::vector<T> data;
  int m_size;
};

#endif
