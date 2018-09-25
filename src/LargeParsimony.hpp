//
//  LargeParsimony.hpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//

#ifndef LargeParsimony_hpp
#define LargeParsimony_hpp

#include <stdio.h>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "SmallParsimony.hpp"
#endif /* LargeParsimony_hpp */
using namespace std;

class LargeParsimony {
public:
  // for global
  int num_char_trees_;
  // not including the root
  int num_nodes_;
  int num_leaves_;
  int num_edges_;
  int unrooted_undirectional_tree_len_;
  // n nodes, leaf has 1 edge, other 3, always chage after calling
  // nearest_neighbor_interchage(int a, int b, int b_child)
  shared_ptr<int> unrooted_undirectional_tree_;
  // n nodes, never change!!!
  shared_ptr<int> unrooted_undirectional_idx_arr_;
  // (str_len)*(N + 1), never change!!!
  shared_ptr<char> rooted_char_list_;

  // for final result
  int min_large_parsimony_score_;
  deque<shared_ptr<int>> unrooted_undirectional_tree_queue_;
  deque<shared_ptr<string>> string_list_queue_;

  // for internal use
  // must have a copy of
  shared_ptr<int> cur_unrooted_undirectional_tree_;
  // unrooted_undirectional_tree for internal exchange use
  // (n+1) nodes, parent-children arr
  shared_ptr<int> rooted_directional_tree_;
  // (n+1) nodes
  shared_ptr<int> rooted_directional_idx_arr_;
  // (n+1) * (str_len) nodes
  shared_ptr<char> cur_rooted_char_list_;
  // for get_edges_from_unrooted_undirectional_tree() use
  shared_ptr<int> edges_;
  // for get_edges_from_unrooted_undirectional_tree use
  shared_ptr<bool> visited_;
  deque<shared_ptr<int>> tmp_unrooted_undirectional_tree_queue_;
  deque<shared_ptr<string>> tmp_string_list_queue_;

  LargeParsimony(shared_ptr<int> unrooted_undirectional_tree,
                 shared_ptr<int> unrooted_undirectional_idx_arr,
                 shared_ptr<char> rooted_char_list, int num_nodes,
                 int num_leaves, int num_char_trees)
      : num_char_trees_{num_char_trees}, num_nodes_{num_nodes},
        num_leaves_{num_leaves}, num_edges_{num_nodes - num_leaves - 1},
        unrooted_undirectional_tree_len_{(num_nodes - 1) * 2},
        unrooted_undirectional_tree_{unrooted_undirectional_tree},
        unrooted_undirectional_idx_arr_{unrooted_undirectional_idx_arr},
        rooted_char_list_{rooted_char_list},
        min_large_parsimony_score_{int(1e8)} {

    cur_unrooted_undirectional_tree_ = shared_ptr<int>(
        new int[unrooted_undirectional_tree_len_], [](int *p) { delete[] p; });
    for (int i = 0; i < unrooted_undirectional_tree_len_; i++) {
      cur_unrooted_undirectional_tree_.get()[i] =
          unrooted_undirectional_tree_.get()[i];
    }

    rooted_directional_tree_ = shared_ptr<int>(
        new int[(num_nodes + 1 - num_leaves) * 2], [](int *p) { delete[] p; });
    rooted_directional_idx_arr_ =
        shared_ptr<int>(new int[num_nodes + 1], [](int *p) { delete[] p; });
    int rooted_char_list_len = (num_nodes + 1) * num_char_trees;
    // need to get the char list copy below
    cur_rooted_char_list_ = shared_ptr<char>(new char[rooted_char_list_len],
                                             [](char *p) { delete[] p; });
    for (int i = 0; i < rooted_char_list_len; i++) {
      cur_rooted_char_list_.get()[i] = rooted_char_list_.get()[i];
    }
    // below for get_edges_from_unrooted_undirectional_tree() use

    edges_ =
        shared_ptr<int>(new int[num_edges_ * 2], [](int *p) { delete[] p; });
    visited_ =
        shared_ptr<bool>(new bool[num_nodes_], [](bool *p) { delete[] p; });
  }

  ~LargeParsimony() = default;

  /*
      get a new (unrooted_undirectional_tree,) from old
     (unrootedundirectional_tree) by interchange of an given edge the return
     tree is deep copies of the original one cause gonna reuse it for all the
     edges in it
   */
  void nearest_neighbor_interchage(int a, int b, int a_child, int b_child) {
    // given an edge (node1, node2)
    // b_child is the child of b to exchange with a's left child
    int idx_a = unrooted_undirectional_idx_arr_.get()[a];
    int idx_b = unrooted_undirectional_idx_arr_.get()[b];
    int idx_a_child = unrooted_undirectional_idx_arr_.get()[a_child];
    int idx_b_child = unrooted_undirectional_idx_arr_.get()[b_child];
    while (cur_unrooted_undirectional_tree_.get()[idx_a] != a_child) {
      idx_a++;
    }
    while (cur_unrooted_undirectional_tree_.get()[idx_b] != b_child) {
      idx_b++;
    }
    while (cur_unrooted_undirectional_tree_.get()[idx_a_child] != a) {
      idx_a_child++;
    }
    while (cur_unrooted_undirectional_tree_.get()[idx_b_child] != b) {
      idx_b_child++;
    }
    cur_unrooted_undirectional_tree_.get()[idx_a] = b_child;
    cur_unrooted_undirectional_tree_.get()[idx_b] = a_child;
    cur_unrooted_undirectional_tree_.get()[idx_a_child] = b;
    cur_unrooted_undirectional_tree_.get()[idx_b_child] = a;
  }

  // make the unrooted & undirectional tree rooted & directional
  // call this function every time before small parsimony to generate input for
  // it
  void make_tree_rooted_directional() {
    // a deep copy for rooted_char_list (we want to keep a clean original copy
    // of this) unrooted_undirectional_tree to rooted_directional_tree
    // unrooted_undirectional_idx_arr to rooted_directional_idx_arr
    auto tmp_undirected_idx = unrooted_undirectional_idx_arr_.get();
    auto tmp_neighbor_arr = cur_unrooted_undirectional_tree_.get();
    auto tmp_directed_idx = rooted_directional_idx_arr_.get();
    auto tmp_children_arr = rooted_directional_tree_.get();

    int root = num_nodes_;
    int left = num_nodes_ - 1;
    int right = tmp_neighbor_arr[tmp_undirected_idx[left]];
    int next_children = 0;

    for (int i = 0; i < num_nodes_ + 1; ++i) {
      tmp_directed_idx[i] = -1;
    }

    auto temp_start = next_children;
    tmp_directed_idx[root] = temp_start;
    tmp_children_arr[temp_start++] = left;
    tmp_children_arr[temp_start++] = right;
    next_children = temp_start;

    queue<int> q;
    q.emplace(left);
    q.emplace(right);
    unordered_set<int> visited;
    visited.insert(root);
    visited.insert(left);
    visited.insert(right);

    while (!q.empty()) {
      auto cur_node = q.front();
      q.pop();

      if (cur_node < num_leaves_) {
        continue;
      }

      auto undirected_start_pos = tmp_undirected_idx[cur_node];
      auto directed_start_pos = next_children;
      tmp_directed_idx[cur_node] = directed_start_pos;
      for (int i = undirected_start_pos; i < undirected_start_pos + 3; ++i) {
        auto neighbor = tmp_neighbor_arr[i];
        if (visited.find(neighbor) == visited.end()) {
          tmp_children_arr[directed_start_pos++] = neighbor;
          q.emplace(neighbor);
          visited.insert(neighbor);
        }
      }
      next_children = directed_start_pos;
    }
  }

  // return edges array containing only the internal edges denoted as (a, b) for
  // the internal exchange for unrooted & undirectional tree
  shared_ptr<int> get_edges_from_unrooted_undirectional_tree() {
    for (int i = num_leaves_; i < num_nodes_; i++) {
      visited_.get()[i] = false;
    }
    int edges_ptr = 0;
    for (int i = num_leaves_; i < num_nodes_; i++) {
      int a = i;
      visited_.get()[a] = true;
      int b_idx = unrooted_undirectional_idx_arr_.get()[i];
      for (int j = b_idx; j < b_idx + 3; j++) {
        int b = unrooted_undirectional_tree_.get()[j];

        if (b >= num_leaves_ && !visited_.get()[b]) {
          // edge(a-b) is an internal edge
          edges_.get()[edges_ptr++] = a;
          edges_.get()[edges_ptr++] = b;
        }
      }
    }
    return edges_;
  }

  // creat a deep copy of shared_ptr array and add the ptr to deque
  template <class T>
  void deep_copy_push_back(deque<shared_ptr<T>> &queue, shared_ptr<T> array,
                           int num) {
    // first make a deep copy of array
    shared_ptr<T> array_copy =
        shared_ptr<T>(new T[num], [](T *p) { delete[] p; });
    for (int i = 0; i < num; i++) {
      array_copy.get()[i] = array.get()[i];
    }
    queue.push_back(array_copy);
  }
  // Main entrance function
  void run_large_parsimony() {
    /*
     * input is undirected & unrooted tree; string list
     * each time run a SmallParsimony, we got a directional&rooted array as well
     * as a string list denoted the string for each node. Keep recording the
     * minumum one.
     */

    // write to rooted_directional_tree_ and
    // rooted_directional_idx_arr_
    make_tree_rooted_directional();
    // run small parsimony first
    shared_ptr<SmallParsimony> small_parsimony = make_shared<SmallParsimony>(
        rooted_directional_idx_arr_, rooted_directional_tree_,
        cur_rooted_char_list_, num_char_trees_, num_nodes_ + 1);
    small_parsimony.get()->run_small_parsimony_string();
    // initialization
    int new_score = small_parsimony.get()->total_score_;

    // initialize deque. Noted that (new_score/new_string_list) are always the
    // minimal (score/string_list) in the
    // tmp_unrooted_undirectional_tree_queue_
    deep_copy_push_back<int>(tmp_unrooted_undirectional_tree_queue_,
                             unrooted_undirectional_tree_,
                             unrooted_undirectional_tree_len_);
    deep_copy_push_back<string>(tmp_string_list_queue_,
                                small_parsimony.get()->string_list_,
                                num_nodes_);
    while (!tmp_unrooted_undirectional_tree_queue_.empty()) {

      // record tmp list to final list
      unrooted_undirectional_tree_queue_ =
          tmp_unrooted_undirectional_tree_queue_;
      string_list_queue_ = tmp_string_list_queue_;

      // clear up tmp list
      tmp_unrooted_undirectional_tree_queue_ = deque<shared_ptr<int>>();
      tmp_string_list_queue_ = deque<shared_ptr<string>>();

      // should use new_score -1 is for comparation (here compatible with
      // weichen's code)
      min_large_parsimony_score_ = new_score--;

      auto tree_i_ptr = unrooted_undirectional_tree_queue_.begin();
      auto tree_end = unrooted_undirectional_tree_queue_.end();
      auto string_i_ptr = string_list_queue_.begin();

      for (; tree_i_ptr != tree_end; ++tree_i_ptr, ++string_i_ptr) {
        unrooted_undirectional_tree_ = *tree_i_ptr;
        // get all edges for unrooted_undirectional_tree_
        // write to edges_ visited_
        shared_ptr<int> edges = get_edges_from_unrooted_undirectional_tree();
        // For each edge, exchange the internal edges to get 2 new trees
        int length = num_edges_ * 2;
        for (int i = 0; i < length; i += 2) {
          int a = edges.get()[i];
          int b = edges.get()[i + 1];
          int a_child_idx = unrooted_undirectional_idx_arr_.get()[a];
          int a_child = unrooted_undirectional_tree_.get()[a_child_idx];
          a_child = a_child == b
                        ? unrooted_undirectional_tree_.get()[a_child_idx + 1]
                        : a_child;
          int b_child_idx = unrooted_undirectional_idx_arr_.get()[b];
          int b_child = -1;
          // exchange b's j_th child in unrooted & undirectional tree
          for (int j = 0; j < 2; j++) {
            if (j) {
              for (int k = 2; k >= 0; k--) {
                b_child = unrooted_undirectional_tree_.get()[b_child_idx + k];
                if (b_child != a)
                  break;
              }
            } else {
              for (int k = 0; k < 3; k++) {
                b_child = unrooted_undirectional_tree_.get()[b_child_idx + k];
                if (b_child != a)
                  break;
              }
            }
            // must reinitialize below
            for (int i = 0; i < unrooted_undirectional_tree_len_; i++) {
              cur_unrooted_undirectional_tree_.get()[i] =
                  unrooted_undirectional_tree_.get()[i];
            }
            // begin interchange
            // writed to cur_unrooted_undirectional_tree_
            nearest_neighbor_interchage(a, b, a_child, b_child);
            // write to rooted_directional_idx_arr_; rooted_directional_tree_;
            // need to get the char list copy below
            make_tree_rooted_directional();
            int rooted_char_list_len = (num_nodes_ + 1) * num_char_trees_;
            for (int i = 0; i < rooted_char_list_len; i++) {
              cur_rooted_char_list_.get()[i] = rooted_char_list_.get()[i];
            }
            // run small parsimony
            small_parsimony = make_shared<SmallParsimony>(
                rooted_directional_idx_arr_, rooted_directional_tree_,
                cur_rooted_char_list_, num_char_trees_, num_nodes_ + 1);
            small_parsimony.get()->run_small_parsimony_string();
            // record the minmal one
            if (small_parsimony.get()->total_score_ <= new_score) {
              if (small_parsimony.get()->total_score_ < new_score) {
                // first clear tmp list
                tmp_unrooted_undirectional_tree_queue_.clear();
                tmp_string_list_queue_.clear();
                new_score = small_parsimony.get()->total_score_;
              }

              deep_copy_push_back<int>(tmp_unrooted_undirectional_tree_queue_,
                                       cur_unrooted_undirectional_tree_,
                                       unrooted_undirectional_tree_len_);
              deep_copy_push_back<string>(tmp_string_list_queue_,
                                          small_parsimony.get()->string_list_,
                                          num_nodes_);
            }
          }
        }
      }
    }
  }
};
