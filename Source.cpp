/**
 * BMP Image Compression Tool
 * IDs: k24-2581 (Muhammad Musaddiq Arbi), K24-2525 (Hasan Ayaz)
 */

#include <iostream>
#include <fstream>  

using namespace std;

// ==========================================
// 1. UTILITIES
// ==========================================
void strCopy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int strLen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

// ==========================================
// 2. DATA STRUCTURES
// ==========================================
struct Node {
    unsigned char ch;
    long long freq;
    Node* left, * right;

    Node(unsigned char c, long long f, Node* l = nullptr, Node* r = nullptr) {
        ch = c;
        freq = f;
        left = l;
        right = r;
    }
    bool isLeaf() { return !left && !right; }
};

class MinHeap {
private:
    Node* array[512];
    int size;

public:
    MinHeap() { size = 0; }

    void push(Node* node) {
        int i = size;
        array[size++] = node;
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (array[i]->freq < array[parent]->freq) {
                Node* temp = array[i];
                array[i] = array[parent];
                array[parent] = temp;
                i = parent;
            }
            else { break; }
        }
    }

    Node* pop() {
        if (size == 0) return nullptr;
        Node* minNode = array[0];
        array[0] = array[size - 1];
        size--;
        int i = 0;
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;
            if (left < size && array[left]->freq < array[smallest]->freq) smallest = left;
            if (right < size && array[right]->freq < array[smallest]->freq) smallest = right;
            if (smallest != i) {
                Node* temp = array[i];
                array[i] = array[smallest];
                array[smallest] = temp;
                i = smallest;
            }
            else { break; }
        }
        return minNode;
    }

    int getSize() { return size; }
};

// ==========================================
// 3. CORE LOGIC (Delta + Huffman)
// ==========================================
class HuffmanCompressor {
private:
    long long freqMap[256];
    char codeMap[256][256];
    Node* root;

    void deleteTree(Node* node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

    void generateCodes(Node* node, char* currentCode, int depth) {
        if (!node) return;
        if (node->isLeaf()) {
            currentCode[depth] = '\0';
            strCopy(codeMap[node->ch], currentCode);
        }
        currentCode[depth] = '0';
        generateCodes(node->left, currentCode, depth + 1);
        currentCode[depth] = '1';
        generateCodes(node->right, currentCode, depth + 1);
    }

public:
    HuffmanCompressor() {
        root = nullptr;
        for (int i = 0; i < 256; i++) freqMap[i] = 0;
    }
    ~HuffmanCompressor() { deleteTree(root); }

    // --- COMPRESSION ---
    bool compress(const char* inputFile, const char* outputFile) {
        ifstream in(inputFile, ios::binary | ios::ate);
        if (!in.is_open()) return false;

        long long fileSize = in.tellg();
        in.seekg(0, ios::beg);

        if (fileSize == 0) return false;

        unsigned char* data = new unsigned char[fileSize];
        in.read((char*)data, fileSize);
        in.close();

        // [STEP 1] DELTA ENCODING PRE-PROCESSING
        // We transform the raw pixel data into "Differences"
        // This makes the data highly repetitive (Huffman friendly)
        unsigned char* deltaData = new unsigned char[fileSize];
        deltaData[0] = data[0];
        for (long long i = 1; i < fileSize; i++) {
            deltaData[i] = data[i] - data[i - 1];
        }

        // [STEP 2] FREQUENCY ANALYSIS (On Delta Data)
        for (int i = 0; i < 256; i++) freqMap[i] = 0;
        for (long long i = 0; i < fileSize; i++) {
            freqMap[deltaData[i]]++;
        }

        // [STEP 3] BUILD TREE
        MinHeap pq;
        for (int i = 0; i < 256; i++) {
            if (freqMap[i] > 0) pq.push(new Node((unsigned char)i, freqMap[i]));
        }

        if (pq.getSize() == 1) {
            Node* child = pq.pop();
            root = new Node(0, child->freq, child, nullptr);
        }
        else {
            while (pq.getSize() > 1) {
                Node* left = pq.pop();
                Node* right = pq.pop();
                pq.push(new Node(0, left->freq + right->freq, left, right));
            }
            root = pq.pop();
        }

        // [STEP 4] GENERATE CODES
        char buffer[256];
        generateCodes(root, buffer, 0);

        // [STEP 5] WRITE FILE
        ofstream out(outputFile, ios::binary);

        int mapSize = 0;
        for (int i = 0; i < 256; i++) if (freqMap[i] > 0) mapSize++;
        out.write((char*)&mapSize, sizeof(int));

        for (int i = 0; i < 256; i++) {
            if (freqMap[i] > 0) {
                unsigned char ch = (unsigned char)i;
                out.write((char*)&ch, sizeof(unsigned char));
                out.write((char*)&freqMap[i], sizeof(long long));
            }
        }
        out.write((char*)&fileSize, sizeof(long long));

        unsigned char byteBuffer = 0;
        int bitCount = 0;

        // Note: We encode deltaData, NOT original data
        for (long long i = 0; i < fileSize; i++) {
            unsigned char ch = deltaData[i];
            char* code = codeMap[ch];

            for (int j = 0; code[j] != '\0'; j++) {
                if (code[j] == '1') byteBuffer |= (1 << (7 - bitCount));
                bitCount++;
                if (bitCount == 8) {
                    out.put(byteBuffer);
                    byteBuffer = 0;
                    bitCount = 0;
                }
            }
        }
        if (bitCount > 0) out.put(byteBuffer);

        out.close();
        delete[] data;
        delete[] deltaData;
        return true;
    }

    // --- DECOMPRESSION ---
    bool decompress(const char* inputFile, const char* outputFile) {
        ifstream in(inputFile, ios::binary);
        if (!in.is_open()) return false;

        int mapSize = 0;
        in.read((char*)&mapSize, sizeof(int));

        for (int i = 0; i < 256; i++) freqMap[i] = 0;
        for (int i = 0; i < mapSize; i++) {
            unsigned char ch;
            long long freq;
            in.read((char*)&ch, sizeof(unsigned char));
            in.read((char*)&freq, sizeof(long long));
            freqMap[ch] = freq;
        }

        long long totalChars = 0;
        in.read((char*)&totalChars, sizeof(long long));

        if (root) { deleteTree(root); root = nullptr; }
        MinHeap pq;
        for (int i = 0; i < 256; i++) {
            if (freqMap[i] > 0) pq.push(new Node((unsigned char)i, freqMap[i]));
        }

        if (pq.getSize() == 0) return false;

        if (pq.getSize() == 1) {
            Node* child = pq.pop();
            root = new Node(0, child->freq, child, nullptr);
        }
        else {
            while (pq.getSize() > 1) {
                Node* left = pq.pop();
                Node* right = pq.pop();
                pq.push(new Node(0, left->freq + right->freq, left, right));
            }
            root = pq.pop();
        }

        // [STEP 1] DECODE TO DELTA DATA
        // We need to store the decoded stream first to reverse the delta
        unsigned char* deltaData = new unsigned char[totalChars];
        Node* curr = root;
        char byte;
        long long charsDecoded = 0;

        while (in.get(byte)) {
            for (int i = 7; i >= 0; i--) {
                if (charsDecoded >= totalChars) break;

                bool bit = (byte >> i) & 1;
                if (bit == 0) curr = curr->left;
                else curr = curr->right;

                if (curr->isLeaf()) {
                    deltaData[charsDecoded] = curr->ch;
                    curr = root;
                    charsDecoded++;
                }
            }
        }
        in.close();

        // [STEP 2] REVERSE DELTA ENCODING
        // Reconstruct the original image pixel by pixel
        unsigned char* originalData = new unsigned char[totalChars];
        originalData[0] = deltaData[0];
        for (long long i = 1; i < totalChars; i++) {
            originalData[i] = originalData[i - 1] + deltaData[i];
        }

        // [STEP 3] SAVE RESTORED FILE
        ofstream out(outputFile, ios::binary);
        out.write((char*)originalData, totalChars);
        out.close();

        delete[] deltaData;
        delete[] originalData;
        return true;
    }
};

// ==========================================
// 4. UI SYSTEM
// ==========================================
void clearScreen() {
    cout << "\033[2J\033[1;1H"; // ANSI Clear
}

void printHeader() {
    clearScreen();
    cout << "\n\t+--------------------------------------------------+\n";
    cout << "\t|    BMP COMPRESSOR PRO (DELTA + HUFFMAN)          |\n";
    cout << "\t+--------------------------------------------------+\n";
    cout << "\t| IDs: k24-2581 & K24-2525                         |\n";
    cout << "\t+--------------------------------------------------+\n\n";
}

void showLoading(const char* txt) {
    cout << "\t[Processing] " << txt << " ... " << endl;
}

int main() {
    HuffmanCompressor tool;
    int choice = 0;
    char inName[100], outName[100];

    while (true) {
        printHeader();
        cout << "\t[1] Compress Image\n";
        cout << "\t[2] Decompress Image\n";
        cout << "\t[3] Exit\n";
        cout << "\n\t>> Option: ";

        cin >> choice;
        cin.ignore();

        if (choice == 3) break;

        if (choice == 1) {
            cout << "\n\tInput File: "; cin.getline(inName, 100);
            cout << "\tOutput File: "; cin.getline(outName, 100);
            showLoading("Applying Delta Encoding");
            showLoading("Compressing Data");
            if (tool.compress(inName, outName)) cout << "\n\t[SUCCESS] File Compressed!\n";
            else cout << "\n\t[ERROR] Failed.\n";
        }
        else if (choice == 2) {
            cout << "\n\tInput File: "; cin.getline(inName, 100);
            cout << "\tOutput File: "; cin.getline(outName, 100);
            showLoading("Decoding & Reconstructing");
            if (tool.decompress(inName, outName)) cout << "\n\t[SUCCESS] File Restored!\n";
            else cout << "\n\t[ERROR] Failed.\n";
        }

        cout << "\n\tPress Enter...";
        cin.get();
    }
    return 0;
}