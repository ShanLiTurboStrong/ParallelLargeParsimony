#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

using namespace std;

/**
 * Read lines from a file
 *
 * @param file_name : the input file
 * @return a queue of strings, each of which is a line in file_name, with ending
 * '\n' stripped
 */
queue<string> readLines(string file_name) {
  queue<string> lines;
  ifstream in(file_name.c_str());

  if (!in) {
    return lines;
  }

  string str;
  while (getline(in, str)) {
    lines.emplace(str);
  }

  return lines;
}

/**
 * Check whether a string is a number
 *
 * @param s : the input string
 * @return if the string is a number
 */
bool isNumber(const string &s) {
  return !s.empty() &&
         find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) ==
             s.end();
}

/**
 * Given an input string (a node), find its corresponding node id
 *
 * @param assign : the assignment map for leaves
 * @param cur_leave : the node id of the next input leave
 * @param input_str : the input ndoe
 * @param output_num : the id of the node in the tree
 */
void assign_number(unordered_map<string, int> &assign, int &cur_leave,
                   const string &input_str, int &output_num) {
  if (isNumber(input_str)) {
    output_num = stoi(input_str);
  } else {
    if (assign.find(input_str) == assign.end()) {
      output_num = cur_leave--;
      assign.emplace(input_str, output_num);
    } else {
      output_num = assign.at(input_str);
    }
  }
}

/**
 * Given one line, extract the neighbors and store the information
 *
 * @param line : the input string line
 * @param assign : the assignment map for leaves
 * @param cur_leave : the node id of the next input leave
 * @return : the pair of node ids in the line
 */
tuple<int, int> getNeighborPair(string line, unordered_map<string, int> &assign,
                                int &cur_leave) {
  string delimiter = "->";
  auto del_idx = line.find(delimiter);
  string first = line.substr(0, del_idx);
  string second = line.substr(del_idx + 2, line.length());
  int first_number, second_number;

  assign_number(assign, cur_leave, first, first_number);
  assign_number(assign, cur_leave, second, second_number);

  return tuple<int, int>(first_number, second_number);
}

/**
 * Given a pair of node ids, store their neighboring information in a map
 *
 * @param neighbors : the map storing neighboring information
 * @param first : first node id
 * @param second : second node id
 */
void connectNeighborPair(unordered_map<int, unordered_set<int>> &neighbors,
                         int first, int second) {
  if (neighbors.find(first) != neighbors.end()) {
    neighbors[first].insert(second);
  } else {
    neighbors[first] = {second};
  }

  if (neighbors.find(second) != neighbors.end()) {
    neighbors[second].insert(first);
  } else {
    neighbors[second] = {first};
  }
}

/**
 * Given a neighboring map, generate two arrays for the neighboring
 * relationships.
 *
 * There are N nodes and N - 1 edges in the tree, thus there are 2 * (N - 1)
 * neighboring relationships
 *
 * @param neighbors : the map storing neighboring information
 * @param undirected_idx : undirected_idx[a] == b means that the neighbors of a
 * are stored at neighbor_arr[b]
 * @param neighbor_arr : self-explained
 */
void convertNeighborsToUndirectedArr(
    const unordered_map<int, unordered_set<int>> &neighbors,
    shared_ptr<int> &undirected_idx, shared_ptr<int> &neighbor_arr) {
  int next_neighbor = 0;
  auto temp_idx = undirected_idx.get();
  auto temp_arr = neighbor_arr.get();

  for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
    auto node_num = it->first;
    auto temp_start = next_neighbor;
    temp_idx[node_num] = temp_start;
    for (auto sit = (it->second).begin(); sit != (it->second).end(); ++sit) {
      temp_arr[temp_start++] = *sit;
    }
    next_neighbor = temp_start;
  }
}

/**
 * Given an unrooted undirected tree (N nodes, N - 1 edges, 2 * (N - 1)
 * neighboring relationships), generate a rooted directed tree (N + 1 nodes, N
 * edges, N directed relationships)
 *
 * @param num_undirected_nodes : N
 * @param num_leaves : number of leaves
 * @param undirected_idx : undirected_idx[a] == b means that the neighbors of a
 * are stored at neighbor_arr[b]
 * @param neighbor_arr : self-explained
 * @param directed_idx : directed_idx[a] == b means that the children of a are
 * stored at children_arr[b]
 * @param children_arr : self-explained
 */
void covertUndirectedToDirected(const int num_undirected_nodes,
                                const int num_leaves,
                                shared_ptr<int> &undirected_idx,
                                shared_ptr<int> &neighbor_arr,
                                shared_ptr<int> &directed_idx,
                                shared_ptr<int> &children_arr) {
  auto tmp_undirected_idx = undirected_idx.get();
  auto tmp_neighbor_arr = neighbor_arr.get();
  auto tmp_directed_idx = directed_idx.get();
  auto tmp_children_arr = children_arr.get();

  int root = num_undirected_nodes;
  int left = num_undirected_nodes - 1;
  int right = tmp_neighbor_arr[tmp_undirected_idx[left]];
  int next_children = 0;

  for (int i = 0; i < num_undirected_nodes + 1; ++i) {
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

    if (cur_node < num_leaves) {
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

/**
 * Given a assignment map, populate the chars to a char list
 * @param char_list : length: (str_len) * (N + 1)
 * @param assign : the assignment map for leaves
 * @param num_char_trees : str_len
 * @param num_directed_nodes : N + 1
 */
void initializeCharList(shared_ptr<char> &char_list,
                        unordered_map<string, int> &assign, int num_char_trees,
                        int num_directed_nodes) {
  auto tmp_char_list = char_list.get();
  for (auto it = assign.begin(); it != assign.end(); ++it) {
    const char *chars = (it->first).c_str();
    int node = it->second;
    for (int tree = 0; tree < num_char_trees; ++tree) {
      auto char_pos = tree * num_directed_nodes + node;
      tmp_char_list[char_pos] = chars[tree];
    }
  }
}