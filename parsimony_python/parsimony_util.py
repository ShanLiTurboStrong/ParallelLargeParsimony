from collections import deque


def hamming_distance(a, b):
    n = len(a)
    count = 0
    for i in range(n):
        if a[i] != b[i]:
            count += 1
    return count


def not_equal(a, b):
    if a == b:
        return 0
    return 1


# T is undirected, rooted
# T format: {a : [b, c], b: [a], ...}
# tag format: {a : 0, b : 1, c : 0}
def find_ripe_node_one_tree_unrooted(T, tag):
    for each in T:
        if tag[each] == 0:
            count = 0
            children = []
            for child in T[each]:
                if tag[child] == 1:
                    count += 1
                    children.append(child)
            if count == 2:
                return True, each, children
    return False, None, []


def get_neighbor_pair(line, assign, counter):
    division = line.strip().split("->")

    if division[0].isdigit() and division[1].isdigit():
        father = int(division[0])
        son = int(division[1])
    elif division[0].isdigit():
        father = int(division[0])
        b = division[1]
        if b not in assign:
            assign[b] = counter[0]
            counter[0] += 1
        son = assign[b]
    elif division[1].isdigit():
        a = division[0]
        son = int(division[1])
        if a not in assign:
            assign[a] = counter[0]
            counter[0] += 1
        father = assign[a]
    else:
        a = division[0]
        b = division[1]
        if a not in assign:
            assign[a] = counter[0]
            counter[0] += 1
        if b not in assign:
            assign[b] = counter[0]
            counter[0] += 1
        father = assign[a]
        son = assign[b]

    return father, son


def connect_neighbor_pair(father, son, T):
    if father in T:
        if son not in T[father]:
            T[father].append(son)
    else:
        T[father] = [son]

    if son in T:
        if father not in T[son]:
            T[son].append(father)
    else:
        T[son] = [father]


def construct_char_list(character_list, assign):
    for each in assign:
        while len(character_list) < len(each):
            character_list.append({})
        for i in range(len(each)):
            character_list[i][assign[each]] = each[i]


def insert_root(T):
    new_node = max([i for i in T]) + 1
    left = [i for i in T][0]
    right = T[left][0]

    if len(T[left]) == 1:
        del T[left]
    else:
        T[left] = [i for i in T[left] if i != right]

    if len(T[right]) == 1:
        del T[right]
    else:
        T[right] = [i for i in T[right] if i != left]

    T.setdefault(left, []).append(new_node)
    T.setdefault(right, []).append(new_node)
    T.setdefault(new_node, []).append(left)
    T[new_node].append(right)


def read_lines(file_name):
    result = []
    with open(file_name, "r") as data:
        for line in data:
            result.append(line.strip())
    return result


def write_result(file_name, result):
    with open(file_name, "w") as output:
        output.write(result)


def serialize_tree(score, tree):
    edge_list = []
    node_str_list = []
    node_id_map = {0: 0}

    q = deque()
    next_id = 1

    q.append(0)

    while len(q) > 0:
        top = q.popleft()
        top_new_id = node_id_map[top]
        top_str = tree[top]["str"]
        node_str_list.append("{}->{}".format(top_new_id, top_str))
        top_children = tree[top]["children"]
        for child in top_children:
            if child not in node_id_map:
                node_id_map[child] = next_id
                next_id += 1
                q.append(child)

            child_new_id = node_id_map[child]
            edge_list.append("{}->{}".format(top_new_id, child_new_id))

    edge_list = sorted(edge_list, key=lambda x: int(x.split("->")[0]))

    return "{}\n{}\n{}".format(score, "\n".join(edge_list), "\n".join(node_str_list))
