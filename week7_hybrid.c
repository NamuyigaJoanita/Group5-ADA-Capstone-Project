#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 64
#define MAX_SYMBOLS 256
#define MAX_NODES 511

typedef struct Node {
    int symbol;
    int frequency;
    int left;
    int right;
} Node;

typedef struct MinHeap {
    int size;
    int indices[MAX_NODES];
} MinHeap;

typedef struct Code {
    char bits[MAX_SYMBOLS];
    int length;
} Code;

Node nodes[MAX_NODES];
int node_count = 0;
Code codes[MAX_SYMBOLS];

void rle_compress(uint8_t input[], int input_len, uint8_t output[], int *output_len) {
    int i = 0, j = 0;
    while (i < input_len) {
        uint8_t value = input[i];
        int count = 1;
        while (i + count < input_len && input[i + count] == value && count < 255) {
            count++;
        }
        output[j]     = value;
        output[j + 1] = count;
        j += 2;
        i += count;
    }
    *output_len = j;
}

void count_frequencies(uint8_t block[], int freq[]) {
    memset(freq, 0, MAX_SYMBOLS * sizeof(int));
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        freq[block[i]]++;
    }
}

void heap_swap(MinHeap *h, int i, int j) {
    int temp = h->indices[i];
    h->indices[i] = h->indices[j];
    h->indices[j] = temp;
}

void heap_push(MinHeap *h, int node_idx) {
    h->indices[h->size] = node_idx;
    int i = h->size;
    h->size++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (nodes[h->indices[parent]].frequency > nodes[h->indices[i]].frequency) {
            heap_swap(h, parent, i);
            i = parent;
        } else break;
    }
}

int heap_pop(MinHeap *h) {
    int top = h->indices[0];
    h->size--;
    h->indices[0] = h->indices[h->size];
    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < h->size && nodes[h->indices[left]].frequency < nodes[h->indices[smallest]].frequency) smallest = left;
        if (right < h->size && nodes[h->indices[right]].frequency < nodes[h->indices[smallest]].frequency) smallest = right;
        if (smallest != i) {
            heap_swap(h, i, smallest);
            i = smallest;
        } else break;
    }
    return top;
}

int build_huffman_tree(int freq[]) {
    MinHeap heap;
    heap.size = 0;
    node_count = 0;
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            nodes[node_count].symbol    = i;
            nodes[node_count].frequency = freq[i];
            nodes[node_count].left      = -1;
            nodes[node_count].right     = -1;
            heap_push(&heap, node_count);
            node_count++;
        }
    }
    while (heap.size > 1) {
        int left  = heap_pop(&heap);
        int right = heap_pop(&heap);
        nodes[node_count].symbol    = -1;
        nodes[node_count].frequency = nodes[left].frequency + nodes[right].frequency;
        nodes[node_count].left      = left;
        nodes[node_count].right     = right;
        heap_push(&heap, node_count);
        node_count++;
    }
    return heap_pop(&heap);
}

void generate_codes(int node_idx, char *current_code, int depth) {
    if (node_idx == -1) return;
    if (nodes[node_idx].symbol != -1) {
        current_code[depth] = '\0';
        strcpy(codes[nodes[node_idx].symbol].bits, current_code);
        codes[nodes[node_idx].symbol].length = depth;
        return;
    }
    current_code[depth] = '0';
    generate_codes(nodes[node_idx].left, current_code, depth + 1);
    current_code[depth] = '1';
    generate_codes(nodes[node_idx].right, current_code, depth + 1);
}

int huffman_compressed_bytes(uint8_t block[]) {
    int freq[MAX_SYMBOLS];
    char current_code[MAX_SYMBOLS];
    memset(codes, 0, sizeof(codes));
    count_frequencies(block, freq);
    int root = build_huffman_tree(freq);
    generate_codes(root, current_code, 0);
    int total_bits = 0, i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) total_bits += freq[i] * codes[i].length;
    }
    return (total_bits + 7) / 8;
}

void hybrid_compress(uint8_t block[], int block_num) {
    uint8_t rle_output[128];
    int rle_size = 0;
    rle_compress(block, BLOCK_SIZE, rle_output, &rle_size);

    int huffman_size = huffman_compressed_bytes(block);

    float rle_ratio     = (1.0 - (float)rle_size    / BLOCK_SIZE) * 100;
    float huffman_ratio = (1.0 - (float)huffman_size / BLOCK_SIZE) * 100;

    char *winner;
    int best_size;
    float best_ratio;

    if (rle_size <= huffman_size) {
        winner     = "RLE";
        best_size  = rle_size;
        best_ratio = rle_ratio;
    } else {
        winner     = "Huffman";
        best_size  = huffman_size;
        best_ratio = huffman_ratio;
    }

    printf("\n========== Block %d ==========\n", block_num);
    printf("RLE size:          %d bytes (%.2f%%)\n", rle_size,     rle_ratio);
    printf("Huffman size:      %d bytes (%.2f%%)\n", huffman_size, huffman_ratio);
    printf("Winner:            %s\n", winner);
    printf("Best compressed:   %d bytes\n", best_size);
    printf("Best ratio:        %.2f%%\n", best_ratio);
}

int main() {
    uint8_t block1[64] = {
        10,  10,  10,  50,  50,  50,  50, 200,
        200, 200, 100, 100, 100, 100,   5,   5,
          5,   5,   5,  10,  10,  10,  10,  10,
        255, 255, 255, 255,  80,  80,  80,  80,
         80,  30,  30,  30,  30,  30,  30,  70,
         70,  70,  70,  70,  70,  70,  70,  70,
          0,   0,   0,   0,   0,   0,   0,   0,
        128, 128, 128, 128, 128, 128, 128, 128
    };

    uint8_t block2[64] = {
        10,  10,  20,  30,  30,  40,  50,  50,
        60,  60,  60,  70,  80,  80,  90,  90,
       100, 100, 110, 110, 110, 120, 130, 130,
       140, 150, 150, 160, 160, 170, 170, 180,
       180, 180, 190, 200, 200, 210, 210, 220,
       220, 220, 230, 230, 240, 240, 250, 250,
       100, 100, 120, 120, 140, 140, 160, 160,
       180, 180, 200, 200, 220, 220, 240, 240
    };

    uint8_t block3[64] = {
        10,  25,  38,  47,  62,  71,  85,  93,
       104, 113, 127, 136, 148, 155, 169, 178,
       183, 192, 201, 214, 223, 237, 245, 251,
        15,  28,  39,  44,  57,  68,  79,  88,
        97, 106, 119, 132, 143, 158, 167, 176,
       185, 196, 207, 218, 229, 238, 247, 253,
        12,  23,  34,  45,  56,  67,  78,  89,
       100, 111, 122, 133, 144, 155, 166, 177
    };

    printf("HYBRID COMPRESSION SYSTEM\n");
    printf("Automatically selects best method per block\n");
    printf("==========================================");

    hybrid_compress(block1, 1);
    hybrid_compress(block2, 2);
    hybrid_compress(block3, 3);

    return 0;
}