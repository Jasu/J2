#pragma once

struct nontrivial {
  static int alive[24];

  int value, n_copy_constructs = 0, n_move_constructs = 0, n_copy_assigns = 0, n_move_assigns = 0;
  nontrivial * self;
  nontrivial(int value);

  void copy(const nontrivial & rhs);

  nontrivial(const nontrivial & rhs);

  nontrivial(nontrivial && rhs);

  nontrivial & operator=(const nontrivial & rhs);

  nontrivial & operator=(nontrivial && rhs);

  bool operator==(const nontrivial & rhs) const noexcept;
  bool operator<(const nontrivial & rhs) const noexcept;

  ~nontrivial();

  void require(int value,
               int copy_constructs, int move_constructs,
               int copy_assigns, int move_assigns) const;
};

void clear_alive();

void require_none_alive();

void require_range_alive(int start, int num, int alive_count = 1);

void require_range_alive_non_exclusive(int start, int num, int alive_count = 1);
