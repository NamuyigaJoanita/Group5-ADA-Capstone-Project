# Image Compression for Resource-Constrained Devices

> **CPE2206: Analysis and Design of Algorithms**  
> Soroti University · School of Engineering and Technology  
> Department of Electronics and Computer Engineering

---

## Project Overview

This project implements a **lossless image compression pipeline** designed to run on resource-constrained hardware such as a small FPGA or microcontroller with only **256 KB of RAM**.

The system targets **grayscale VGA images (640 x 480 pixels, ~307 KB)** captured by security cameras. Since the raw image exceeds available RAM, compression is done in **8 x 8 pixel blocks** (64 bytes each), keeping active memory usage to just **192 bytes** at any time.

Three algorithms were implemented and evaluated:

| Algorithm | Best For | Time Complexity |
|-----------|----------|-----------------|
| Run-Length Encoding (RLE) | Uniform/repeated regions | O(n) |
| Huffman Coding | Varied pixel distributions | O(n + k log k) |
| Hybrid (RLE + Huffman) | All block types | Both |

---

## Group 5:  Team Members

| Name | Student Number |
|------|----------------|
| Joseph Walusimbi | 2401600068 |
| Joanita Namuyiga | 2401600382 |
| Jude Mishael Matovu | 2401600066 |
| Joshua Benjamin Ssentongo | 2401600236 |

---

## Repository Structure

```
Group5-ADA-Capstone-Project/
│
├── week4_rle_compression.c   # Week 4: Run-Length Encoding — compress & decompress
├── week6_huffman.c           # Week 6: Huffman Coding — tree build, encode, decode
├── week7_hybrid.c            # Week 7: Hybrid selector — picks best method per block
├── .gitignore
└── README.md
```

---

## How It Works

### Block-Based Processing

Because the full 307 KB image exceeds the 256 KB RAM limit, the system processes the image one **8 x 8 block** at a time:

```
Image File (307 KB)
      |
Read 8x8 block (64 bytes)
      |
Compress block (RLE / Huffman / Hybrid)
      |
Write compressed output (max 128 bytes)
      |
Repeat for all 4,800 blocks
      |
Output compressed image + compression ratio
```

### Compression Methods

**RLE** (`week4_rle_compression.c`) replaces runs of repeated pixels with `(value, count)` pairs:
```
Input:  255 255 255 255 100 100 50
Output: (255,4) (100,2) (50,1)       <- 6 bytes instead of 7
```

**Huffman Coding** (`week6_huffman.c`) assigns shorter binary codes to more frequent pixel values, achieving entropy-optimal compression regardless of run structure.

**Hybrid** (`week7_hybrid.c`) runs both algorithms on each block and keeps whichever result is smaller — guaranteeing no block ever expands.

---

## Key Results

| Block Type | RLE | Huffman | Hybrid (Best) |
|------------|-----|---------|---------------|
| Uniform    | 62.5% reduction | 42% reduction | 62.5% |
| Mixed      | -3.1% (expansion) | 16% reduction | 16% |
| Varied     | -100% (expansion) | 0% (no loss) | 0% |

The hybrid approach **never expands** a block, making it the safest choice for embedded deployment.

---

## Building and Running

### Prerequisites

- GCC (or any C99-compatible compiler)

### Compile each file individually

```bash
# Week 4 — RLE
gcc -O2 -Wall -o rle week4_rle_compression.c

# Week 6 — Huffman
gcc -O2 -Wall -o huffman week6_huffman.c

# Week 7 — Hybrid
gcc -O2 -Wall -o hybrid week7_hybrid.c
```

### Run

```bash
./rle
./huffman
./hybrid
```

---

## Memory Usage

| Buffer | Size | Purpose |
|--------|------|---------|
| Input block | 64 bytes | Current 8x8 pixel block |
| Compressed output | 128 bytes | Worst-case RLE output |
| **Total active** | **192 bytes** | Well within 256 KB limit |

---

## References

1. A. S. Rongali, D. Dhabliya, I. Pandey, A. Sachdeva, H. D. Praveena, and S. C. Aich, "Investigating applications of run length encoding in data compression & source coding," in *Proc. 15th Int. Conf. Computing Communication and Networking Technologies (ICCCNT)*, Kamand, India: IEEE, Jun. 2024, pp. 1–5. doi: 10.1109/ICCCNT61001.2024.10725079.

2. D. A. Southard, "Compression of digitized map images," *Comput. Geosci.*, vol. 18, no. 9, pp. 1213–1253, Oct. 1992. doi: 10.1016/0098-3004(92)90041-O.

3. K. Sayood, *Introduction to Data Compression*, 3rd ed. Amsterdam, The Netherlands: Elsevier (Morgan Kaufmann), 2006.

4. D. S. Bhadane and S. Y. Kanawade, "Comparative study of RLE & K-RLE compression and decompression in WSN," in *Proc. 3rd Int. Conf. Advanced Computing and Communication Systems (ICACCS)*, Coimbatore, India: IEEE, Jan. 2016, pp. 1–5. doi: 10.1109/ICACCS.2016.7586319.

5. D. Salomon, Ed., *Data Compression: The Complete Reference*, 4th ed. London, U.K.: Springer, 2007. doi: 10.1007/978-1-84628-603-2.

6. J. Namuyiga et al., "Group 5 ADA Capstone Project — Source Code Repository," GitHub, 2026. [Online]. Available: https://github.com/NamuyigaJoanita/Group5-ADA-Capstone-Project. [Accessed: Apr. 2026].

---

## License

This project was developed for academic purposes as part of the CPE2206 coursework at Soroti University. All rights reserved by the authors.

---

*Submitted: April 2026 · Lecturer: Dr. Stanley Mugisha*
