static inline int8 map_char_idx(int8 ref_c) {
    switch (ref_c) {
        case 65: return 0;
        case 67: return 1;
        case 71: return 2;
        case 84: return 3;
        default: return -1;
    }
}

export void array_copy_ispc(uniform int arr_len, uniform int input[], uniform int output[]) {
    foreach (i = 0 ... arr_len) {
        output[i] = input[i];
    }
}

export void initialize_small_parsimony_ispc(
                        uniform int num_nodes, 
                        uniform int infinity,
                        uniform int s_v_k[], 
                        uniform unsigned int8 tag[],
                        uniform int8 rooted_char_list[],
                        uniform int rooted_directional_idx_arr[]) {
    foreach (i = 0 ... num_nodes, j = 0 ... 4) {
        int bias = 4 * i;
        int8 leaf_char = rooted_char_list[i];
        int node_idx = rooted_directional_idx_arr[i];
        s_v_k[bias + j] = infinity * (int)(node_idx == -1 && j != leaf_char);
    }

    foreach (i = 0 ... num_nodes) {
        int node_idx = rooted_directional_idx_arr[i];
        tag[i] = (unsigned int8)(node_idx == -1);
    }
}

export void array_init_ispc(uniform int arr_len, uniform int output[]) {
    foreach (i = 0 ... arr_len) {
        output[i] = -1;
    }
}

export void map_char_idx_ispc(uniform int arr_len, uniform int8 input[], uniform int8 output[]) {
    foreach (i = 0 ... arr_len) {
        int8 ref_c = input[i];
        output[i] = map_char_idx(ref_c);
    }
}
