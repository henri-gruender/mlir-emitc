// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file defines tensor classes used by EmitC

#ifndef EMITC_TENSOR_H
#define EMITC_TENSOR_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

namespace {
template <typename nono = void>
constexpr size_t sum() {
  return 0;
}

template <size_t First, size_t... Rest>
constexpr size_t sum() {
  return First + sum<Rest...>();
}

template <size_t First, size_t... Rest>
constexpr size_t first() {
  return First;
}

template <size_t Default>
constexpr size_t first_default() {
  return Default;
}

template <size_t Default, size_t First, size_t... Rest>
constexpr size_t first_default() {
  return First;
}

template <typename none = void>
constexpr bool all_same() {
  return true;
}

template <size_t First, size_t... Rest>
constexpr bool all_same() {
  return First == first_default<First, Rest...>() && all_same<Rest...>();
}
} // namespace

template <typename T, size_t SIZE>
class Tensor {
public:
  using value_type = T;
  using reference_type = typename std::vector<T>::reference;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  Tensor() : data(SIZE) {}

  Tensor(std::initializer_list<T> data) : data(data) {
    assert(data.size() == SIZE);
  }

  size_t size() const { return size_; }

  iterator begin() { return data.begin(); }

  const_iterator begin() const { return data.begin(); }

  iterator end() { return data.end(); }

  const_iterator end() const { return data.end(); }

  // Index into the flat data buffer.
  reference_type operator[](size_t x) {
    assert(0 <= x && x < SIZE);
    return data[x];
  }

  std::vector<T> data;
  static const size_t size_;
};

template <typename T>
class Tensor0D : public Tensor<T, 1> {
public:
  using reference_type = typename Tensor<T, 1>::reference_type;

  Tensor0D() : Tensor<T, 1>() {}

  Tensor0D(std::initializer_list<T> data) : Tensor<T, 1>(data) {}

  reference_type operator()() { return this->data.at(0); }

  static const size_t rank;
  static const std::array<size_t, 0> shape;
};

template <typename T, size_t DimX>
class Tensor1D : public Tensor<T, DimX> {
public:
  using reference_type = typename Tensor<T, DimX>::reference_type;

  Tensor1D() : Tensor<T, DimX>() {}

  Tensor1D(std::initializer_list<T> data) : Tensor<T, DimX>(data) {}

  reference_type operator()(size_t x) {
    assert(0 <= x && x < dimX);

    return this->operator[](x);
  }

  static const size_t dimX;
  static const size_t rank;
  static const std::array<size_t, 1> shape;
};

template <typename T, size_t DimX, size_t DimY>
class Tensor2D : public Tensor<T, DimX * DimY> {
public:
  using reference_type = typename Tensor<T, DimX * DimY>::reference_type;

  Tensor2D() : Tensor<T, DimX * DimY>() {}

  Tensor2D(std::initializer_list<T> data) : Tensor<T, DimX * DimY>(data) {}

  reference_type operator()(size_t x, size_t y) {
    assert(0 <= x && x < dimX);
    assert(0 <= y && y < dimY);

    return this->operator[](x *DimY + y);
  }

  static const size_t dimX;
  static const size_t dimY;
  static const size_t rank;
  static const std::array<size_t, 2> shape;
};

template <typename T, size_t SIZE>
const size_t Tensor<T, SIZE>::size_ = SIZE;

template <typename T, size_t DimX>
const size_t Tensor1D<T, DimX>::dimX = DimX;

template <typename T, size_t DimX, size_t DimY>
const size_t Tensor2D<T, DimX, DimY>::dimX = DimX;

template <typename T, size_t DimX, size_t DimY>
const size_t Tensor2D<T, DimX, DimY>::dimY = DimY;

template <typename T>
const size_t Tensor0D<T>::rank = 0;

template <typename T>
const std::array<size_t, 0> Tensor0D<T>::shape = {};

template <typename T, size_t DimX>
const size_t Tensor1D<T, DimX>::rank = 1;

template <typename T, size_t DimX>
const std::array<size_t, 1> Tensor1D<T, DimX>::shape = {DimX};

template <typename T, size_t DimX, size_t DimY>
const size_t Tensor2D<T, DimX, DimY>::rank = 2;

template <typename T, size_t DimX, size_t DimY>
const std::array<size_t, 2> Tensor2D<T, DimX, DimY>::shape = {DimX, DimY};

template <typename T>
using is_scalar = std::is_arithmetic<T>;

template <typename T>
struct is_tensor_0d : std::false_type {};

template <typename T>
struct is_tensor_0d<Tensor0D<T>> : std::true_type {};

template <typename T>
struct is_tensor_1d : std::false_type {};

template <typename T, size_t DimX>
struct is_tensor_1d<Tensor1D<T, DimX>> : std::true_type {};

template <typename T>
struct is_tensor_2d : std::false_type {};

template <typename T, size_t DimX, size_t DimY>
struct is_tensor_2d<Tensor2D<T, DimX, DimY>> : std::true_type {};

template <typename T, typename Unused = void>
struct is_tensor : std::false_type {};

template <typename T>
struct is_tensor<T,
                 typename std::enable_if<std::is_base_of<
                     Tensor<typename T::value_type, T::size_>, T>::value>::type>
    : std::true_type {};

template <typename T>
using IsScalar = typename std::enable_if<std::is_scalar<T>::value, bool>::type;

template <typename T>
using IsTensor0D = typename std::enable_if<is_tensor_0d<T>::value, bool>::type;

template <typename T>
using IsTensor1D = typename std::enable_if<is_tensor_1d<T>::value, bool>::type;

template <typename T>
using IsTensor2D = typename std::enable_if<is_tensor_2d<T>::value, bool>::type;

template <typename T>
using IsTensor = typename std::enable_if<is_tensor<T>::value, bool>::type;

template <typename T>
struct get_element_type {
  using type = T;
};

template <typename T>
struct get_element_type<Tensor0D<T>> {
  using type = T;
};

template <typename T, size_t DimX>
struct get_element_type<Tensor1D<T, DimX>> {
  using type = T;
};

template <typename T, size_t DimX, size_t DimY>
struct get_element_type<Tensor2D<T, DimX, DimY>> {
  using type = T;
};

template <typename Dest, typename Src>
struct replace_element_type {
  using type = Dest;
};

template <typename Dest, typename Src>
struct replace_element_type<Dest, Tensor0D<Src>> {
  using type = Tensor0D<Dest>;
};

template <typename Dest, typename Src, size_t DimX>
struct replace_element_type<Dest, Tensor1D<Src, DimX>> {
  using type = Tensor1D<Dest, DimX>;
};

template <typename Dest, typename Src, size_t DimX, size_t DimY>
struct replace_element_type<Dest, Tensor2D<Src, DimX, DimY>> {
  using type = Tensor2D<Dest, DimX, DimY>;
};

template <typename Dest, typename Src>
using UnaryFuncType = Dest (*)(Src);

template <typename Dest, typename SrcLeft, typename SrcRight>
using BinaryFuncType = Dest (*)(SrcLeft, SrcRight);

template <typename Dest, typename Src, typename UnaryOp, IsScalar<Src> = true>
inline Dest unary(Src x, UnaryOp &&op) {
  return op(x);
}

template <typename Dest, typename Src, typename UnaryOp, IsTensor<Src> = true>
inline Dest unary(Src x, UnaryOp &&op) {
  Dest z;
  std::transform(x.begin(), x.end(), z.begin(), op);
  return z;
}

template <typename Dest, typename SrcLeft, typename SrcRight, typename BinaryOp,
          IsScalar<SrcLeft> = true>
inline Dest binary(SrcLeft x, SrcRight y, BinaryOp &&op) {
  return op(x, y);
}

template <typename Dest, typename SrcLeft, typename SrcRight, typename BinaryOp,
          IsTensor<SrcLeft> = true>
inline Dest binary(SrcLeft x, SrcRight y, BinaryOp &&op) {
  Dest z;
  std::transform(x.begin(), x.end(), y.begin(), z.begin(), op);
  return z;
}

template <typename T, size_t D, typename... Ts>
struct concat {};

template <typename T, size_t D, size_t... Xs>
struct concat<T, D, Tensor1D<T, Xs>...> {
  static_assert(0 <= D && D < 1, "Dimension index out of bounds");
  using type = Tensor1D<T, sum<Xs...>()>;
};

template <typename T, size_t D, size_t... Xs, size_t... Ys>
struct concat<T, D, Tensor2D<T, Xs, Ys>...> {
  static_assert(0 <= D && D < 2, "Dimension index out of bounds");
  static_assert((D == 0 && all_same<Ys...>()) || (D == 1 && all_same<Xs...>()),
                "All dimensions except for the dimension index must match");
  using type = typename std::conditional<
      D == 0, Tensor2D<T, sum<Xs...>(), first<Ys...>()>,
      Tensor2D<T, first<Xs...>(), sum<Ys...>()>>::type;
};

#endif // EMITC_TENSOR_H
