#ifndef DISTMAT_H
#define DISTMAT_H

#include <vector>

template <class T>
class Symmat
{
public:
  Symmat(int n = 0, T def = T());
  Symmat(const std::vector<T> &vec);
  T &acc(const int i, const int j);
  T get(const int i, const int j) const;
  int size() const;
  Symmat<T> sub(std::vector<int> &subvec) const;

private:
  std::vector<T> data;
  int m_size; // m_size * m_size elements in matrix
};

typedef Symmat<double> Distmat;

#endif
