//
//  SmallParsimony.hpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//

#ifndef SmallParsimony_hpp
#define SmallParsimony_hpp

#include <stdio.h>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#endif /* SmallParsimony_hpp */

using namespace std;

class SmallParsimony {
  // trees here are all rooted and directed
 public:
  // char set for each tree. All str_len sets.
  // length: (str_len)*(N+1) (N is the original nodes, 1 is the root)
  // data layout: [xxxxxxx][xxxxxxx][xxxxxxx][xxxxxxx] (xxxxxxx is the chars for
  // a tree, [] means a tree)
  shared_ptr<char> char_list_;

  // Tree structure
  // length: N+1 (N is the original nodes, 1 is the root)
  shared_ptr<int> idx_arr_;

  // #internal nodes * 2
  shared_ptr<int> children_arr_;

  // string length for each node
  int num_char_trees_;

  // N + 1, 1 is the root
  int num_nodes_;

  // final results
  int total_score_;

  // length N, assign each of node a string finally
  shared_ptr<string> string_list_;

  SmallParsimony(shared_ptr<int> idx_arr, shared_ptr<int> children_arr,
                 shared_ptr<char> char_list, int num_char_trees, int num_nodes)
      : char_list_{char_list},
        idx_arr_{idx_arr},
        children_arr_{children_arr},
        num_char_trees_{num_char_trees},
        num_nodes_{num_nodes},
        total_score_{0} {
    string_list_ = shared_ptr<string>(new string[num_nodes_ - 1],
                                      [](string *p) { delete[] p; });

    int char_list_len = num_char_trees_ * num_nodes_;
    for (int i = 0; i < char_list_len; i++) {
      char &cur_c = char_list_.get()[i];

      switch (cur_c) {
        case 'A':
          cur_c = 0;
          break;
        case 'C':
          cur_c = 1;
          break;
        case 'G':
          cur_c = 2;
          break;
        case 'T':
          cur_c = 3;
          break;
        default:
          break;
      }
    }
  }

  ~SmallParsimony() = default;

  void run_small_parsimony_string() {
    total_score_ = 0;

    char ACGT_arr[4] = {'A', 'C', 'G', 'T'};

    for (int i = 0; i < num_char_trees_; i++) {
      char *cur_char_list_idx = char_list_.get() + i * num_nodes_;
      int cur_score = run_small_parsimony_char(cur_char_list_idx);

      // add to final total score
      total_score_ += cur_score;

      // append char list to current string list
      for (int i = 0; i < num_nodes_ - 1; i++) {
        string_list_.get()[i] += ACGT_arr[int(cur_char_list_idx[i])];
      }
    }
  }

  /**
   * Use current char list and global tree structure to calculate
   * @param char_list : directional & rooted tree given as children_arr
   * @return the small parsimony score of the char tree and also write the
   * assigned chars to the global char_list
   */
  int run_small_parsimony_char(char *char_list) {
    // local allocation
    // indicate the score of node v choosing k char
    unique_ptr<int[]> s_v_k(new int[num_nodes_ * 4]);
    // indicate if the noed i is ripe
    unique_ptr<unsigned char[]> tag(new unsigned char[num_nodes_]);
    // indicate for each node, chosen a char of 4, what is the best char for its
    // left & right children.
    unique_ptr<unsigned char[]> back_track_arr(
        new unsigned char[num_nodes_ * 8]);

    // initialization (no need to initialize back_track_arr)
    auto infinity = int(1e8);

    for (int i = 0; i < num_nodes_; i++) {
      int bias = 4 * i;
      char leaf_char = char_list[i];
      int node_idx = idx_arr_.get()[i];
      for (int j = 0; j < 4; j++) {
        // if it is a leaves & it is not the leave char, assign it to infinity
        // if -1, then it is a leaf
        s_v_k.get()[bias + j] =
            infinity * int(node_idx == -1 && j != leaf_char);
      }
      // all leaves are ripe already
      tag.get()[i] = node_idx == -1;
    }

    int root = -1, min_parsimony_score = infinity, root_char_idx = '#';

    // main logic
    while (true) {
      // find a ripe node in the tree
      int ripe_node = -1;
      int daughter = -1;
      int son = -1;
      for (int i = 0; i < num_nodes_; i++) {
        if (!tag.get()[i]) {
          int bias = idx_arr_.get()[i];
          daughter = children_arr_.get()[bias];
          son = children_arr_.get()[bias + 1];
          if (tag.get()[daughter] && tag.get()[son]) {
            ripe_node = i;
            break;
          }
        }
      }

      // if no ripe, root has been found and dealed with
      if (ripe_node == -1) {
        break;
      }

      root = ripe_node;
      min_parsimony_score = infinity;
      root_char_idx = '#';
      tag.get()[root] = 1;

      for (int i = 0; i < 4; i++) {
        char min_left_char_idx = '#';
        char min_right_char_idx = '#';

        int left_min_score = infinity;
        int right_min_score = infinity;

        int offset_left = 4 * daughter;
        int offset_right = 4 * son;

        for (int left_i = 0; left_i < 4; left_i++) {
          int tmp_score = s_v_k.get()[offset_left + left_i] + int(i != left_i);
          if (tmp_score < left_min_score) {
            left_min_score = tmp_score;
            min_left_char_idx = (char)left_i;
          }
        }

        for (int right_i = 0; right_i < 4; right_i++) {
          int tmp_score =
              s_v_k.get()[offset_right + right_i] + int(i != right_i);
          if (tmp_score < right_min_score) {
            right_min_score = tmp_score;
            min_right_char_idx = (char)right_i;
          }
        }

        int cur_total_score = left_min_score + right_min_score;
        s_v_k.get()[root * 4 + i] = cur_total_score;

        if (cur_total_score < min_parsimony_score) {
          min_parsimony_score = cur_total_score;
          root_char_idx = i;
        }

        int back_track_arr_offset = root * 8 + i * 2;
        back_track_arr.get()[back_track_arr_offset] = min_left_char_idx;
        back_track_arr.get()[back_track_arr_offset + 1] = min_right_char_idx;
      }
    }
    // No ripe node any more, root is the fianl root now, calcuate the final
    // score and fill up the char array (tree)
    queue<int> q;
    q.push(root);
    char_list[root] = root_char_idx;
    while (!q.empty()) {
      int parent = q.front();
      q.pop();

      char min_char_idx = char_list[parent];
      int child_idx = idx_arr_.get()[parent];
      // if it is not a leaf
      if (child_idx != -1) {
        int left_child_id = children_arr_.get()[child_idx];
        int right_child_id = children_arr_.get()[child_idx + 1];

        int tmp_idx = parent * 8 + 2 * min_char_idx;
        char left_min_char_idx = back_track_arr.get()[tmp_idx];
        char right_min_char_idx = back_track_arr.get()[tmp_idx + 1];

        char_list[left_child_id] = left_min_char_idx;
        char_list[right_child_id] = right_min_char_idx;

        if (idx_arr_.get()[left_child_id] != -1) q.push(left_child_id);
        if (idx_arr_.get()[right_child_id] != -1) q.push(right_child_id);
      }
    }
    return min_parsimony_score;
  }
};
