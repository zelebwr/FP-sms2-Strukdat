#pragma once
#include <string>
#include <iostream>
#include <map>
#include <memory>

struct UserPreference {
    std::string profileName;
    double timeWeight;
    double costWeight;
    double distanceWeight;
};

struct TreeNode {
    std::string question;
    bool isLeaf = false;
    UserPreference preference;
    std::map<std::string, std::unique_ptr<TreeNode>> children;

    TreeNode(std::string q) : question(q) {}
    TreeNode(UserPreference p) : isLeaf(true), preference(p) {}
};

class DecisionTree {
private:
    std::unique_ptr<TreeNode> root;

    void buildTree() {
        auto fast = UserPreference{"Cepat", 10, 1, 1};
        auto murah = UserPreference{"Murah", 2, 10, 3};
        auto dekat = UserPreference{"Dekat", 1, 3, 10};

        root = std::make_unique<TreeNode>("Apa prioritas utama Anda?");
        root->children["1"] = std::make_unique<TreeNode>(fast);
        root->children["2"] = std::make_unique<TreeNode>(murah);
        root->children["3"] = std::make_unique<TreeNode>(dekat);
    }

    void printTree(const TreeNode* node, int depth = 0) const {
        if (!node) return;
        for (int i = 0; i < depth; ++i) std::cout << "  ";
        if (node->isLeaf) {
            std::cout << "-> " << node->preference.profileName << "\n";
        } else {
            std::cout << node->question << "\n";
            for (const auto& [k, child] : node->children) {
                for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
                std::cout << k << ". ";
                printTree(child.get(), depth + 2);
            }
        }
    }

public:
    DecisionTree() { buildTree(); }

    UserPreference run() const {
        TreeNode* current = root.get();
        std::string input;
        while (!current->isLeaf) {
            std::cout << current->question << "\n";
            for (const auto& [key, node] : current->children) {
                std::cout << "  " << key << ". " << node->preference.profileName << "\n";
            }
            std::cout << "Masukkan pilihan (angka): ";
            std::cin >> input;
            if (current->children.count(input)) {
                current = current->children.at(input).get();
            } else {
                std::cout << "Pilihan tidak valid, coba lagi.\n";
            }
        }
        return current->preference;
    }

    void visualize() const { printTree(root.get()); }
};