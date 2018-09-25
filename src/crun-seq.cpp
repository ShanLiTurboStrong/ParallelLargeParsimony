//
//  main.cpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//
//
#include <fstream>
#include <iostream>
#include "LargeParsimony.hpp"
#include "util.h"

void runBaseline(string file_name, string outfile_name) {
  auto lines = readLines(file_name);
  int num_leaves = stoi(lines.front());
  int cur_leave = num_leaves - 1;

  lines.pop();
  unordered_map<string, int> assign;
  unordered_map<int, unordered_set<int>> neighbors;
  int max_node_idx = -1;
  while (!lines.empty()) {
    auto line = lines.front();
    lines.pop();

    auto pair = getNeighborPair(line, assign, cur_leave);
    int first = get<0>(pair);
    int second = get<1>(pair);
    max_node_idx = first > max_node_idx ? first : max_node_idx;
    max_node_idx = second > max_node_idx ? second : max_node_idx;

    connectNeighborPair(neighbors, first, second);
  }

  int num_char_trees = (assign.begin()->first).length();

  // Convert from Neighbor Map to Undirected Tree
  int num_undirected_nodes = max_node_idx + 1;
  int num_undirected_edges = num_undirected_nodes - 1;

  auto undirected_idx = shared_ptr<int>(new int[num_undirected_nodes],
                                        [](int *p) { delete[] p; });
  auto neighbor_arr = shared_ptr<int>(new int[num_undirected_edges * 2],
                                      [](int *p) { delete[] p; });
  convertNeighborsToUndirectedArr(neighbors, undirected_idx, neighbor_arr);

  // Directed Tree
  int num_directed_nodes = max_node_idx + 2;
  int num_directed_internal_nodes = num_directed_nodes - num_leaves;

  auto directed_idx =
      shared_ptr<int>(new int[num_directed_nodes], [](int *p) { delete[] p; });

  auto children_arr = shared_ptr<int>(new int[num_directed_internal_nodes * 2],
                                      [](int *p) { delete[] p; });
  auto char_list =
      shared_ptr<char>(new char[num_directed_nodes * num_char_trees],
                       [](char *p) { delete[] p; });

  covertUndirectedToDirected(num_undirected_nodes, num_leaves, undirected_idx,
                             neighbor_arr, directed_idx, children_arr);

  initializeCharList(char_list, assign, num_char_trees, num_directed_nodes);

  // run large parsimony
  shared_ptr<LargeParsimony> large_parsimony = make_shared<LargeParsimony>(
      neighbor_arr, undirected_idx, char_list, num_undirected_nodes, num_leaves,
      num_char_trees);
  large_parsimony.get()->run_large_parsimony();

  int min_large_parsimony_score =
      large_parsimony.get()->min_large_parsimony_score_;
  int *unrooted_undirectional_idx_arr =
      large_parsimony.get()->unrooted_undirectional_idx_arr_.get();
  deque<shared_ptr<int>> unrooted_undirectional_tree_queue =
      large_parsimony.get()->unrooted_undirectional_tree_queue_;
  deque<shared_ptr<string>> string_list_queue =
      large_parsimony.get()->string_list_queue_;

  ofstream myfile;
  myfile.open(outfile_name);
  auto tree_i_ptr = unrooted_undirectional_tree_queue.begin();
  auto tree_end = unrooted_undirectional_tree_queue.end();
  auto string_i_ptr = string_list_queue.begin();

  for (; tree_i_ptr != tree_end; ++tree_i_ptr, ++string_i_ptr) {
    shared_ptr<int> cur_tree = *tree_i_ptr;
    shared_ptr<string> cur_string_list = *string_i_ptr;
    // begin writing to file
    myfile << min_large_parsimony_score << "\n";
    for (int i = 0; i < num_undirected_nodes; i++) {
      if (i < num_leaves) {
        myfile << i << "->" << cur_tree.get()[unrooted_undirectional_idx_arr[i]]
               << "\n";
      } else {
        for (int j = 0; j < 3; j++) {
          myfile << i << "->"
                 << cur_tree.get()[unrooted_undirectional_idx_arr[i] + j]
                 << "\n";
        }
      }
    }
    for (int i = 0; i < num_undirected_nodes; i++) {
      myfile << i << "->" << cur_string_list.get()[i] << "\n";
    }
    // end of write
    myfile << "-----\n";
  }
  myfile.close();
}

int main(int argc, const char *argv[]) { runBaseline(argv[1], argv[2]); }
