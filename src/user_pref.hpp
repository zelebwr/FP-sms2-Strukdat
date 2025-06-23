#ifndef USER_PREF_HPP
#define USER_PREF_HPP

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <iomanip>

struct UserPreference {
    std::string profileName;
    double timeWeight = 1.0;
    double costWeight = 1.0;
    double distanceWeight = 1.0;
};

struct TreeNode {
    std::string question;
    bool isLeaf = false;
    UserPreference preferences;
    std::map<std::string, std::unique_ptr<TreeNode>> children;

    TreeNode(std::string q) : question(std::move(q)) {}
    TreeNode(UserPreference prefs) : isLeaf(true), preferences(std::move(prefs)) {}
};

class DecisionTree {
private:
    std::unique_ptr<TreeNode> root;

    void printTree(const TreeNode* node, int indent = 0) const {
        if (!node) return;
        for (int i = 0; i < indent; ++i) std::cout << "  ";
        if (node->isLeaf) {
            std::cout << "-> " << node->preferences.profileName << "\n";
        } else {
            std::cout << "Q: " << node->question << "\n";
            for (const auto& [opt, child] : node->children) {
                for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
                std::cout << "[" << opt << "]\n";
                printTree(child.get(), indent + 2);
            }
        }
    }

public:
    DecisionTree() { build(); }

    void build() {
        auto fast = UserPreference{"Business Traveler", 10, 2, 1};
        auto cheap = UserPreference{"Budget Traveler", 1, 10, 2};
        auto scenic = UserPreference{"Scenic Explorer", 2, 3, 10};
        auto balanced = UserPreference{"Balanced", 5, 5, 5};

        root = std::make_unique<TreeNode>("Apa prioritas utama perjalanan Anda?");

        auto nodeFast = std::make_unique<TreeNode>("Apakah biaya penting bagi Anda?");
        nodeFast->children["1"] = std::make_unique<TreeNode>(balanced);
        nodeFast->children["2"] = std::make_unique<TreeNode>(fast);

        auto nodeCheap = std::make_unique<TreeNode>("Apakah waktu fleksibel?");
        nodeCheap->children["1"] = std::make_unique<TreeNode>(cheap);
        nodeCheap->children["2"] = std::make_unique<TreeNode>(balanced);

        auto nodeScenic = std::make_unique<TreeNode>("Apakah Anda ingin rute langsung?");
        nodeScenic->children["1"] = std::make_unique<TreeNode>(scenic);
        nodeScenic->children["2"] = std::make_unique<TreeNode>(balanced);

        root->children["1"] = std::move(nodeFast);
        root->children["2"] = std::move(nodeCheap);
        root->children["3"] = std::move(nodeScenic);
    }

    UserPreference run() const {
        TreeNode* current = root.get();
        std::string input;

        while (!current->isLeaf) {
            std::cout << "\n" << current->question << "\n";
            for (const auto& [opt, child] : current->children) {
                std::string label;
                if (child->isLeaf) {
                    label = "Profil: " + child->preferences.profileName;
                } else {
                    label = "(lanjut pertanyaan...)";
                }
                std::cout << "  " << opt << ". " << label << "\n";
            }
            std::cout << "Pilihan Anda (masukkan angka saja): ";
            std::getline(std::cin, input);
            if (current->children.count(input)) {
                current = current->children.at(input).get();
            } else {
                std::cout << "Input tidak valid. Coba lagi.\n";
            }
        }

        std::cout << "\nProfil: " << current->preferences.profileName << "\n";
        return current->preferences;
    }

    void visualize() const {
        std::cout << "\n=== Struktur Decision Tree ===\n";
        printTree(root.get(), 0);
        std::cout << "===============================\n";
    }
};

#endif