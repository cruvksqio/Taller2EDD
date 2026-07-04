#ifndef TRIE_HPP
#define TRIE_HPP

#include <string>
#include <array>
#include <vector>
#include <cctype>

template <typename Value>
class Trie {
private:
    struct Node {
        std::array<Node*, 256> children;
        std::vector<Value> values;
        bool isWord;
        Node() : children(), values(), isWord(false) {
            children.fill(nullptr);
        }
    };

    Node* root;

    void clear(Node* node) {
        if (!node) return;
        for (Node* child : node->children) {
            clear(child);
        }
        delete node;
    }

    static unsigned char normalizeChar(char c) {
        return static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
    }

public:
    Trie() : root(new Node()) {}

    ~Trie() {
        clear(root);
    }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    void clearAll() {
        clear(root);
        root = new Node();
    }

    void insert(const std::string& key, const Value& value) {
        Node* node = root;
        for (char ch : key) {
            unsigned char index = normalizeChar(ch);
            if (node->children[index] == nullptr) {
                node->children[index] = new Node();
            }
            node = node->children[index];
            node->values.push_back(value);
        }
        node->isWord = true;
    }

    std::vector<Value> searchPrefix(const std::string& prefix) const {
        Node* node = root;
        for (char ch : prefix) {
            unsigned char index = normalizeChar(ch);
            node = node->children[index];
            if (node == nullptr) {
                return {};
            }
        }
        return node->values;
    }
};

#endif // TRIE_HPP
