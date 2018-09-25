import copy
from collections import deque
from parsimony_util import *


# T is undirected, rooted
# T format: {a : [b, c], b: [a], ...}
# character format: {a: 'A', b: 'C', 'd': 'C', ...} (for all leaf nodes)
def small_parsimony_one_tree_unrooted(T, character):
    backtrack = {}
    s_k_v = {}
    tag = {}
    for v in T:
        # v is leaf node
        if len(T[v]) == 1:
            tag[v] = 1
            for k in {"A", "T", "C", "G"}:
                if character[v] == k:
                    s_k_v.setdefault(v, {})[k] = 0
                else:
                    s_k_v.setdefault(v, {})[k] = 100000000
        else:
            tag[v] = 0

    while True:
        option = find_ripe_node_one_tree_unrooted(T, tag)
        if not option[0]:
            break

        v = option[1]
        tag[v] = 1

        daughter = option[2][0]
        son = option[2][1]

        for k in {"A", "T", "C", "G"}:

            backtrack.setdefault(v, {})[k] = {}

            first = 1000000000
            second = 1000000000

            for cur_char in {"A", "T", "C", "G"}:
                temp = s_k_v[daughter][cur_char] + \
                    not_equal(cur_char, k)
                if temp < first:
                    first = temp
                    min_daughter_char = cur_char

            for cur_char in {"A", "T", "C", "G"}:
                temp = s_k_v[son][cur_char] + not_equal(cur_char, k)
                if temp < second:
                    second = temp
                    min_son_char = cur_char

            # if we choose char 'k' as our current node 'v'
            # the score for node 'v' is 'first + second'
            # the daughter of node 'v' is 'min_daughter_char'
            # the son of node 'v' is 'min_son_char'
            s_k_v.setdefault(v, {})[k] = first + second
            backtrack[v][k][daughter] = min_daughter_char
            backtrack[v][k][son] = min_son_char

    # last 'v' is the root
    # 'root_char' is the char of root for minimal score
    minimal_score = 1000000000
    root = v
    for k in s_k_v[root]:
        if s_k_v[root][k] < minimal_score:
            minimal_score = s_k_v[root][k]
            root_char = k

    left = T[root][0]
    right = T[root][1]
    left_str = backtrack[root][root_char][left]
    right_str = backtrack[root][root_char][right]

    # remove 'root' from backtrack
    backtrack.setdefault(left, {}).setdefault(left_str, {})[right] = right_str
    backtrack.setdefault(right, {}).setdefault(right_str, {})[left] = left_str

    del T[root]

    T[left] = [i for i in T[left] if i != root]
    T[left].append(right)
    T[right] = [i for i in T[right] if i != root]
    T[right].append(left)

    root = left
    root_char = left_str

    # New Tree format, undirected, unrooted
    # new_tree = {node: {"str": xxx, "children": [...]}, ...}
    new_tree = {root: {"str": root_char, "children": []}}
    store = deque()
    store.append((root, root_char))
    visited = set()

    # use backtrack to obtain the new tree with minimal score
    while len(store) > 0:
        options = store.popleft()
        father = options[0]
        father_char = options[1]
        new_tree[father]["children"] = T[father]
        if father == root or len(T[father]) > 1:
            for son in T[father]:
                if son in backtrack[father][father_char] and son not in visited:
                    son_char = backtrack[father][father_char][son]
                    new_tree[son] = {"str": son_char, "children": []}
                    store.append((son, son_char))

        visited.add(father)

    return minimal_score, new_tree, root


def run_small_parsimony_one_tree_unrooted(T, character_list):
    # T is undirected, rooted
    # T format: {a : [b, c], b: [a], ...}
    # backtrack tree format, undirected, unrooted
    # backtrack_tree = {node: {"str": xxx, "children": [...]}, ...}
    tree_list = []

    total = 0
    for each in character_list:
        # backtrack_tree is undirected
        result, backtrack_tree, root = small_parsimony_one_tree_unrooted(
            copy.deepcopy(T), each)
        tree_list.append(backtrack_tree)
        total += result

    # construct the combined tree
    combined_tree = {}

    store = deque()
    store.append(root)
    visited = set()

    while len(store) > 0:
        cur = store.popleft()
        str_list = [each[cur]["str"] for each in tree_list]

        combined_tree.setdefault(cur, {})["str"] = "".join(str_list)
        combined_tree[cur]["children"] = tree_list[0][cur]["children"]

        for son in combined_tree[cur]["children"]:
            if son not in visited:
                store.append(son)
        visited.add(cur)

    return total, combined_tree


def switch_subtree_combined(a, x, b, y, T):

    if y is not None:
        T[a]["children"].append(y)
        T[y]["children"].append(a)
        T[b]["children"] = [i for i in T[b]["children"] if i != y]
        T[y]["children"] = [i for i in T[y]["children"] if i != b]

    if x is not None:
        T[b]["children"].append(x)
        T[x]["children"].append(b)
        T[a]["children"] = [i for i in T[a]["children"] if i != x]
        T[x]["children"] = [i for i in T[x]["children"] if i != a]

    return T


def nearest_neighbors_combined(a, b, T):

    T_1 = copy.deepcopy(T)
    T_2 = copy.deepcopy(T)

    a_candidates = [i for i in T[a]["children"] if i != b]
    b_candidates = [i for i in T[b]["children"] if i != a]

    if len(a_candidates) == 2 and len(b_candidates) == 2:
        T_1 = switch_subtree_combined(
            a, a_candidates[0], b, b_candidates[0], T_1)
        T_2 = switch_subtree_combined(
            a, a_candidates[0], b, b_candidates[1], T_2)

    return T_1, T_2


def decompose_combined_tree(combined_tree):

    # combined_tree: undirected, unrooted
    # Combined Tree format
    # combined_tree = {node: {"str": xxx, "children": [...]}, ...}
    # nodes are fully assigned with strings

    temp_T = {i: combined_tree[i]["children"] for i in combined_tree}
    insert_root(temp_T)

    assign = {}
    for i in temp_T:
        if len(temp_T[i]) == 1:
            assign[combined_tree[i]['str']] = i

    character_list = []
    construct_char_list(character_list, assign)

    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    return temp_T, character_list


def nearest_neighbor_interchage(T, character_list):
    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    # Combined Tree format, undirected, unrooted
    # combined_tree = {node: {"str": xxx, "children": [...]}, ...}

    score = 1000000000

    # we need this step to transform T to new_tree
    # T is undirected, rooted
    # new_tree is undirected, unrooted
    new_score, new_tree = run_small_parsimony_one_tree_unrooted(
        T, character_list)

    new_tree_list = [new_tree]
    while new_score < score:
        score = new_score
        tree_list = new_tree_list
        new_tree_list = []

        for tree in tree_list:
            Tree_edges = set()
            store = deque()
            store.append(0)
            visited = set()

            while len(store) > 0:
                cur = store.popleft()
                for son in tree[cur]["children"]:
                    if son not in visited:
                        if len(tree[cur]["children"]) > 1 and len(tree[son]["children"]) > 1:
                            Tree_edges.add((cur, son))
                        store.append(son)
                visited.add(cur)

            for e in Tree_edges:
                neighbors = nearest_neighbors_combined(e[0], e[1], tree)
                for NeighborTree in neighbors:
                    rooted_tree, character_list = decompose_combined_tree(
                        NeighborTree)
                    neighborScore, temp_tree = run_small_parsimony_one_tree_unrooted(
                        rooted_tree, character_list)
                    if neighborScore < new_score:
                        new_score = neighborScore
                        new_tree_list = [temp_tree]
                    elif neighborScore == new_score:
                        new_tree_list.append(temp_tree)

    if len(new_tree_list) > 0:
        out_list = new_tree_list
    else:
        out_list = tree_list
    output_str_set = {serialize_tree(score, tree) for tree in out_list}

    return "\n-----\n".join([i for i in output_str_set]) + "\n-----"
