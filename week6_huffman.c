#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 64
#define MAX_SYMBOLS 256
#define MAX_NODES 511  // max nodes in huffman tree (2*256 - 1)

// ─── DATA STRUCTURES ──────────────────────────────────────────────────────────

// Huffman tree node
typedef struct Node {
    int symbol;      // pixel value (0-255), -1 if internal node
    int frequency;   // how often this value appears
    int left;        // index of left child (-1 if none)
    int right;       // index of right child (-1 if none)
} Node;

// Min-heap for building the tree
typedef struct MinHeap {
    int size;
    int indices[MAX_NODES];  // stores node indices
} MinHeap;

// Huffman code for each symbol
typedef struct Code {
    char bits[MAX_SYMBOLS];  // binary string e.g. "1010"
    int length;              // number of bits
} Code;

// ─── GLOBAL VARIABLES ─────────────────────────────────────────────────────────
Node nodes[MAX_NODES];
int node_count = 0;
Code codes[MAX_SYMBOLS];

// ─── FREQUENCY COUNTING ───────────────────────────────────────────────────────
void count_frequencies(uint8_t block[], int freq[]) {
    memset(freq, 0, MAX_SYMBOLS * sizeof(int));
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        freq[block[i]]++;
    }
}

// ─── MIN HEAP FUNCTIONS ───────────────────────────────────────────────────────
void heap_swap(MinHeap *h, int i, int j) {
    int temp = h->indices[i];
    h->indices[i] = h->indices[j];
    h->indices[j] = temp;
}

void heap_push(MinHeap *h, int node_idx) {
    h->indices[h->size] = node_idx;
    int i = h->size;
    h->size++;
    // bubble up
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (nodes[h->indices[parent]].frequency > 
            nodes[h->indices[i]].frequency) {
            heap_swap(h, parent, i);
            i = parent;
        } else {
            break;
        }
    }
}

int heap_pop(MinHeap *h) {
    int top = h->indices[0];
    h->size--;
    h->indices[0] = h->indices[h->size];
    // bubble down
    int i = 0;
    while (1) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < h->size && 
            nodes[h->indices[left]].frequency < 
            nodes[h->indices[smallest]].frequency) {
            smallest = left;
        }
        if (right < h->size && 
            nodes[h->indices[right]].frequency < 
            nodes[h->indices[smallest]].frequency) {
            smallest = right;
        }
        if (smallest != i) {
            heap_swap(h, i, smallest);
            i = smallest;
        } else {
            break;
        }
    }
    return top;
}

// ─── BUILD HUFFMAN TREE ───────────────────────────────────────────────────────
int build_huffman_tree(int freq[]) {
    MinHeap heap;
    heap.size = 0;
    node_count = 0;

    // create a leaf node for each symbol with frequency > 0
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

    // merge two smallest nodes until only one remains
    while (heap.size > 1) {
        int left  = heap_pop(&heap);
        int right = heap_pop(&heap);

        // create new internal node
        nodes[node_count].symbol    = -1;
        nodes[node_count].frequency = nodes[left].frequency + 
                                      nodes[right].frequency;
        nodes[node_count].left      = left;
        nodes[node_count].right     = right;
        heap_push(&heap, node_count);
        node_count++;
    }

    // return root of tree
    return heap_pop(&heap);
}

// ─── GENERATE CODES ───────────────────────────────────────────────────────────
void generate_codes(int node_idx, char *current_code, int depth) {
    if (node_idx == -1) return;

    // leaf node — save the code
    if (nodes[node_idx].symbol != -1) {
        current_code[depth] = '\0';
        strcpy(codes[nodes[node_idx].symbol].bits, current_code);
        codes[nodes[node_idx].symbol].length = depth;
        return;
    }

    // go left — add '0'
    current_code[depth] = '0';
    generate_codes(nodes[node_idx].left, current_code, depth + 1);

    // go right — add '1'
    current_code[depth] = '1';
    generate_codes(nodes[node_idx].right, current_code, depth + 1);
}

// ─── CALCULATE COMPRESSED SIZE ────────────────────────────────────────────────
int calculate_compressed_bits(uint8_t block[], int freq[]) {
    int total_bits = 0;
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            total_bits += freq[i] * codes[i].length;
        }
    }
    return total_bits;
}

// ─── PRINT CODES ──────────────────────────────────────────────────────────────
void print_codes(int freq[]) {
    printf("\nHuffman Codes:\n");
    printf("%-12s %-12s %-15s %s\n", 
           "Pixel Value", "Frequency", "Code", "Code Length");
    printf("------------------------------------------------\n");
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            printf("%-12d %-12d %-15s %d bits\n",
                   i, freq[i], codes[i].bits, codes[i].length);
        }
    }
}

// ─── TEST BLOCK ───────────────────────────────────────────────────────────────
void test_huffman(uint8_t block[], int block_num) {
    int freq[MAX_SYMBOLS];
    char current_code[MAX_SYMBOLS];

    // reset codes
    memset(codes, 0, sizeof(codes));

    printf("\n========== Block %d ==========\n", block_num);

    // stage 1: count frequencies
    count_frequencies(block, freq);

    // stage 2: build huffman tree
    int root = build_huffman_tree(freq);

    // stage 3: generate codes
    generate_codes(root, current_code, 0);

    // print codes
    print_codes(freq);

    // calculate sizes
    int compressed_bits  = calculate_compressed_bits(block, freq);
    int compressed_bytes = (compressed_bits + 7) / 8;  // round up to nearest byte
    int original_bytes   = BLOCK_SIZE;

    float ratio = (1.0 - (float)compressed_bytes / original_bytes) * 100;

    printf("\nOriginal size:     %d bytes\n", original_bytes);
    printf("Compressed size:   %d bytes (%d bits)\n", 
           compressed_bytes, compressed_bits);
    printf("Compression ratio: %.2f%%\n", ratio);
}

// ─── MAIN ─────────────────────────────────────────────────────────────────────
int main() {

    // Block 1: uniform — lots of repeating values
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

    // Block 2: mixed — some repetition
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

    // Block 3: varied — almost every pixel different
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

    test_huffman(block1, 1);
    test_huffman(block2, 2);
    test_huffman(block3, 3);

    return 0;
}