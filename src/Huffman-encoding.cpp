#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <string>
#include <io.h>
#include <direct.h>

using namespace std;

// ===================== 哈夫曼树节点 =====================
struct Node {
    unsigned char ch;
    int freq;
    Node* left;
    Node* right;

    Node(unsigned char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// 小根堆比较规则
struct Compare {
    bool operator()(const Node* a, const Node* b) {
        return a->freq > b->freq;
    }
};

// ===================== 构建哈夫曼树 =====================
Node* buildHuffmanTree(const unordered_map<unsigned char, int>& freq) {
    priority_queue<Node*, vector<Node*>, Compare> heap;

    for (auto& p : freq)
        heap.push(new Node(p.first, p.second));

    while (heap.size() > 1) {
        Node* left = heap.top(); heap.pop();
        Node* right = heap.top(); heap.pop();
        Node* merge = new Node(0, left->freq + right->freq);
        merge->left = left;
        merge->right = right;
        heap.push(merge);
    }

    return heap.empty() ? nullptr : heap.top();
}

// ===================== 生成编码表 =====================
void buildCodeTable(Node* root, string code, unordered_map<unsigned char, string>& table) {
    if (!root) return;
    if (!root->left && !root->right) {
        table[root->ch] = code.empty() ? "0" : code;
        return;
    }
    buildCodeTable(root->left, code + "0", table);
    buildCodeTable(root->right, code + "1", table);
}

// ===================== 哈夫曼编码 =====================
vector<unsigned char> huffmanEncode(const vector<unsigned char>& data, unordered_map<unsigned char, string>& table) {
    string bits;
    for (unsigned char c : data)
        bits += table[c];

    vector<unsigned char> result;
    int len = bits.size();
    for (int i = 0; i < len; i += 8) {
        string byteStr = bits.substr(i, 8);
        unsigned char byte = 0;
        for (char b : byteStr)
            byte = (byte << 1) | (b == '1');
        result.push_back(byte);
    }
    return result;
}

// ===================== 统计频率 =====================
unordered_map<unsigned char, int> countFrequency(const vector<unsigned char>& data) {
    unordered_map<unsigned char, int> freq;
    for (unsigned char c : data)
        freq[c]++;
    return freq;
}

// ===================== 内存释放 =====================
void freeHuffmanTree(Node* root) {
    if (!root) return;
    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    delete root;
}

// ===================== 测试函数 =====================
void testHuffman(const vector<unsigned char>& data, const string& name) {
    cout << "\n===== " << name << " 哈夫曼编码测试 =====" << endl;

    auto freq = countFrequency(data);
    Node* root = buildHuffmanTree(freq);
    unordered_map<unsigned char, string> table;
    buildCodeTable(root, "", table);

    auto encoded = huffmanEncode(data, table);

    int original = data.size();
    int compressed = encoded.size();
    double ratio = (double)compressed / original * 100;

    cout << "原始大小: " << original << " 字节" << endl;
    cout << "压缩大小: " << compressed << " 字节" << endl;
    cout << "压缩率: " << ratio << "%" << endl;

    freeHuffmanTree(root);
}

// ===================== 读取文件 =====================
vector<unsigned char> readFile(const string& filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        return {};
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    vector<unsigned char> data(size);
    fread(data.data(), 1, size, f);
    fclose(f);
    return data;
}

// ===================== 递归查找文件 =====================
bool findFileRecursive(const string& dir, const string& targetName, string& outPath) {
    _finddata_t fileInfo;
    string searchPath = dir + "\\*.*";
    intptr_t handle = _findfirst(searchPath.c_str(), &fileInfo);

    if (handle == -1) return false;

    do {
        string name = fileInfo.name;
        if (name == "." || name == "..") continue;

        string fullPath = dir + "\\" + name;

        if (name == targetName) {
            outPath = fullPath;
            _findclose(handle);
            return true;
        }

        if (fileInfo.attrib & _A_SUBDIR) {
            if (findFileRecursive(fullPath, targetName, outPath)) {
                _findclose(handle);
                return true;
            }
        }
    } while (_findnext(handle, &fileInfo) == 0);

    _findclose(handle);
    return false;
}



// ===================== 1. 文本 ASCII 模拟测试=====================
void testSimuTxt() {
    string text = "Hello World! Huffman Coding is the best lossless compression algorithm!";
    vector<unsigned char> data(text.begin(), text.end());
    testHuffman(data, "文本(ASCII)");
}

// ===================== 2. 图像 BMP 模拟测试 =====================
void testSimuBmp() {
    vector<unsigned char> data;
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            for (int c = 0; c < 3; c++)
                data.push_back((x + y) % 256);
    testHuffman(data, "图像(BMP 16x16 RGB)");
}

// ===================== 3. 音频 WAV 模拟测试 =====================
void testSimuWav() {
    vector<unsigned char> data;
    for (int i = 0; i < 8000; i++)
        data.push_back(128 + 50 * (i % 20) / 20);
    testHuffman(data, "音频(WAV 8kHz 单声道)");
}



// ===================== 真实 TXT文件 测试 =====================
void testRealTxt() {
    string path;
    if (!findFileRecursive(".", "test-txt.txt", path)) {
        cout << "\n找不到 test.txt 文本文件" << endl;
        return;
    }
    cout << "\n找到文本：" << path << endl;

    auto fileData = readFile(path);
    if (fileData.empty()) {
        cout << "文本文件为空" << endl;
        return;
    }

    testHuffman(fileData, "真实 TXT 文本文件");
}

// =====================真实 BMP文件 测试 =====================
void testRealBmp() {
    string path;
    if (!findFileRecursive(".", "aerial.bmp", path)) {
        cout << "\n找不到 aerial.bmp" << endl;
        return;
    }
    cout << "\n找到 BMP：" << path << endl;

    auto fileData = readFile(path);
    if (fileData.size() < 54) {
        cout << "BMP 文件无效" << endl;
        return;
    }

    int dataOffset =
        (unsigned char)fileData[10] |
        (unsigned char)fileData[11] << 8 |
        (unsigned char)fileData[12] << 16 |
        (unsigned char)fileData[13] << 24;

    vector<unsigned char> pixelData(fileData.begin() + dataOffset, fileData.end());
    testHuffman(pixelData, "BMP真实像素数据");
}

// ===================== 真实 WAV文件 测试 =====================
void testRealWav() {
    string path;
    if (!findFileRecursive(".", "userSound.wav", path)) {
        cout << "\n找不到 userSound.wav" << endl;
        return;
    }
    cout << "\n找到 WAV：" << path << endl;

    auto fileData = readFile(path);
    if (fileData.size() < 44) {
        cout << "WAV 文件无效" << endl;
        return;
    }

    int pos = 12;
    while (pos + 8 < (int)fileData.size()) {
        unsigned int id =
            ((unsigned int)(unsigned char)fileData[pos] << 24) |
            ((unsigned int)(unsigned char)fileData[pos+1] << 16) |
            ((unsigned int)(unsigned char)fileData[pos+2] << 8) |
            (unsigned int)(unsigned char)fileData[pos+3];

        unsigned int len =
            (unsigned int)(unsigned char)fileData[pos+4] |
            (unsigned int)(unsigned char)fileData[pos+5] << 8 |
            (unsigned int)(unsigned char)fileData[pos+6] << 16 |
            (unsigned int)(unsigned char)fileData[pos+7] << 24;

        if (id == 0x64617461) {
            vector<unsigned char> audioData(
                fileData.begin() + pos + 8,
                fileData.begin() + pos + 8 + len
            );
            testHuffman(audioData, "真实WAV音频数据");
            return;
        }
        pos += 8 + len;
    }
    cout << "未找到音频数据" << endl;
}

int main() {
    cout << "\n==============================以下进行模拟文件测试==========================" << endl;
    testSimuTxt();
    testSimuBmp();
    testSimuWav();
    cout << "\n==============================以下进行真实文件测试===========================" << endl;
    testRealTxt();
    testRealBmp();
    testRealWav();
    return 0;
}