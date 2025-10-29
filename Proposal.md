[BitSqueezer: Huffman Coding File Compressor]

|id|Name|
|k24-2581|Muhammad Musaddiq Arbi|
|K24-2525|Hasan Ayaz|

## Introduction
[This project is a command-line utility that implements the Huffman Coding algorithm for lossless data compression.It will consist of two main components: a compress program that takes a standard file and creates a new, smaller compressed file, and a decompress program that perfectly reconstructs the original file from the compressed version.]

## Description
[* **Compression Process:**
    * The compress program will first read the entire input file to build a frequency map of every character
    * It will then use a priority queue to build an optimal Huffman tree based on these frequencies.
    * The tree is traversed to generate a unique binary code for each character.
    * The program creates a new compressed file by first writing a header that contains the information needed to rebuild the tree
    * Finally, it re-reads the input file, looks up the code for each character, and writes the corresponding bits to the new file.

* **Decompression Process:**
    * The decompress program will read the header from the compressed file.
    * It will use this header data to reconstruct the exact same Huffman tree that was used for compression.
    * It will then read the rest of the file's compressed data **one bit at a time**, traversing the tree from the root.    
    * When a leaf node is reached, the character stored in that leaf is written to the output file, and the traversal restarts from the root for the next bit.
    * This continues until all compressed data is read, resulting in a perfect copy of the original file.]

## Data Structures that will be used
[This project will primarily utilize three key data structures: Hash Maps, a Priority Queue, and a Binary Tree. During the compression phase, a Hash Map will be essential for efficiently storing the frequency of each character by mapping character keys to their integer counts. Following this, a Priority Queue, implemented as a Min-Heap, will be used to build the Huffman tree; it will store tree nodes and allow for the efficient retrieval and merging of the two nodes with the lowest frequencies, which is central to the greedy algorithm. The Binary Tree is the core structure itself, representing the optimal prefix-free codes; its leaf nodes will store the actual characters, and the path from the root to a leaf will define that character's new binary code. Finally, a second Hash Map will be used to store these generated codes for fast lookup during the file encoding process. During decompression, this same Binary Tree structure will be reconstructed from the file's header and traversed to decode the bitstream back into the original file.] 



