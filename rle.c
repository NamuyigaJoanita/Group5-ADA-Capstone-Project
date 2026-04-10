#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE 8
void rle_compress(uint8_t input[], int input_len,
                  uint8_t output[], int *output_len)
{
    int i = 0;
    int j = 0;

    while (i < input_len) {
        uint8_t value = input[i];
        int count = 1;

        while (i + count < input_len &&
               input[i + count] == value &&
               count < 255) {
            count++;
        }

        output[j]     = value;
        output[j + 1] = count;
        j += 2;

        i += count;
    }

    *output_len = j;
}
void rle_decompress(uint8_t input[], int input_len,
                    uint8_t output[], int *output_len)
{
    int i = 0;
    int j = 0;

    while (i < input_len) {
        uint8_t value = input[i];
        uint8_t count = input[i + 1];

        int k;
        for (k = 0; k < count; k++) {
            output[j] = value;
            j++;
        }

        i += 2;
    }

    *output_len = j;
}

void print_compressed(uint8_t compressed[], int compressed_len)
{
    int i;
    printf("\nCompressed pairs:\n");
    for (i = 0; i < compressed_len; i += 2) {
        printf("(value=%d, count=%d)\n", compressed[i], compressed[i+1]);
    }
}

void test_block(uint8_t block[], int block_num) {
    uint8_t compressed[128];
    uint8_t decompressed[64];
    int compressed_len = 0;
    int decompressed_len = 0;

    printf("\n===== Block %d =====\n", block_num);
    rle_compress(block, 64, compressed, &compressed_len);
    rle_decompress(compressed, compressed_len, decompressed, &decompressed_len);

    printf("Original size:     64 bytes\n");
    printf("Compressed size:   %d bytes\n", compressed_len);
    printf("Decompressed size: %d bytes\n", decompressed_len);
    printf("Compression ratio: %.2f%%\n",
           (1.0 - (float)compressed_len / 64.0) * 100);
}

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

    test_block(block1, 1);
    test_block(block2, 2);
    test_block(block3, 3);

    return 0;
}