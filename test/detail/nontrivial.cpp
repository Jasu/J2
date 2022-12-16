#include <detail/preamble.hpp>

#include "nontrivial.hpp"

int nontrivial::alive[24] = {0};

nontrivial::nontrivial(int value) : value(value), self(this) {
  REQUIRE(!alive[value]);
  ++alive[value];
}

void nontrivial::copy(const nontrivial & rhs) {
  value = rhs.value;
  n_copy_constructs = rhs.n_copy_constructs;
  n_move_constructs = rhs.n_move_constructs;
  n_copy_assigns = rhs.n_copy_assigns;
  n_move_assigns = rhs.n_move_assigns;
}

nontrivial::nontrivial(const nontrivial & rhs)
  : self(this) {
  copy(rhs);
  n_copy_constructs++;
  REQUIRE(alive[value] == 1);
  ++alive[value];
}

nontrivial::nontrivial(nontrivial && rhs)
  : self(this)
{
  copy(rhs);
  n_move_constructs++;
  REQUIRE(alive[value] == 1);
  ++alive[value];
}

nontrivial & nontrivial::operator=(const nontrivial & rhs) {
  REQUIRE(self == this);
  REQUIRE(alive[value] > 0);
  --alive[value];
  copy(rhs);
  n_copy_assigns++;
  ++alive[value];
  return *this;
}

nontrivial & nontrivial::operator=(nontrivial && rhs) {
  REQUIRE(self == this);
  REQUIRE(alive[value] > 0);
  --alive[value];
  copy(rhs);
  n_move_assigns++;
  ++alive[value];
  return *this;
}

bool nontrivial::operator==(const nontrivial & rhs) const noexcept {
  REQUIRE(self == this);
  REQUIRE(alive[value] > 0);
  REQUIRE(alive[rhs.value] > 0);
  return value == rhs.value;
}
bool nontrivial::operator<(const nontrivial & rhs) const noexcept {
  REQUIRE(self == this);
  REQUIRE(alive[value] > 0);
  REQUIRE(alive[rhs.value] > 0);
  return value < rhs.value;
}

nontrivial::~nontrivial() {
  REQUIRE(self == this);
  REQUIRE(alive[value] > 0);
  --alive[value];
}

void nontrivial::require(int value,
                         int copy_constructs, int move_constructs,
                         int copy_assigns, int move_assigns) const
{
  REQUIRE(self == this);
  REQUIRE(this->value == value);
  REQUIRE(n_copy_constructs == copy_constructs);
  REQUIRE(n_move_constructs == move_constructs);
  REQUIRE(n_copy_assigns == copy_assigns);
  REQUIRE(n_move_assigns == move_assigns);
}

void clear_alive() {
  for (unsigned int i = 0; i < 24U; ++i) {
    nontrivial::alive[i] = 0;
  }
}

void require_none_alive() {
  for (unsigned int i = 0; i < 24U; ++i) {
    REQUIRE(nontrivial::alive[i] == 0);
  }
}

void require_range_alive(int start, int num, int alive_count) {
  for (int i = 0; i < 24; ++i) {
    REQUIRE(nontrivial::alive[i] == ((i >= start && i < start + num) ? alive_count : 0));
  }
}

void require_range_alive_non_exclusive(int start, int num, int alive_count) {
  for (int i = 0; i < 24; ++i) {
    if (i >= start && i < start + num) {
      REQUIRE(nontrivial::alive[i] == alive_count);
    }
  }
}
