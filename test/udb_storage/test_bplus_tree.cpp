#include "include/storage/index/bplus_tree.h"
#include "include/storage/index/int_comparator.h"
#include "include/common/rid.h"
#include "include/storage/index/ktype/Integer.h"
#include "include/storage/disk_manager.h"
#include <gtest/gtest.h>

namespace udb
{
    struct node_backlog {
        /* Node backlogged */
        BPlusTreePage *node;
        /* The index next to the backtrack point, must be >= 1 */
        int next_sub_idx;
    };

    inline int children(struct BPlusTreePage *node)
    {
        return reinterpret_cast<BPTreeInternalPage *>(node)->GetChildren();
    }

    void node_key_dump(struct BPlusTreePage *node)
    {
            int i;
            if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
                    for (i = 0; i < node->GetCount(); i++) {
                            printf("%d ", reinterpret_cast<BPTreeLeafPage *>(node)->KeyAt(i));
                    }
            } else {
                    for (i = 0; i < node->GetCount() - 1; i++) {
                            printf("%d ", reinterpret_cast<BPTreeInternalPage *>(node)->KeyAt(i));
                    }
            }
            printf("\n");
    }

    int node_key(BPlusTreePage *node, int i)
    {
            if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
                    return reinterpret_cast<BPTreeLeafPage *>(node)->KeyAt(i);
            } else {
                    return reinterpret_cast<BPTreeInternalPage *>(node)->KeyAt(i);
            }
    }

    static void key_print(BPlusTreePage *node)
    {
            int i;
            if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
                    BPTreeLeafPage *leaf = reinterpret_cast<BPTreeLeafPage *>(node);
                    printf("leaf:");
                    for (i = 0; i < leaf->GetEntry(); i++) {
                            printf(" %d", leaf->KeyAt(i));
                    }
            } else {
                    BPTreeInternalPage *non_leaf = reinterpret_cast<BPTreeInternalPage *>(node);
                    printf("node:");
                    for (i = 0; i < non_leaf->GetChildren() - 1; i++) {
                            printf(" %d", non_leaf->KeyAt(i));
                    }
            }
            printf("\n");
    }

    void bplus_tree_dump(BPlusTree *tree)
    {
            int level = 0;
            BPlusTreePage *node = tree->GetRoot();
            struct node_backlog *p_nbl = NULL;
            struct node_backlog nbl_stack[MAX_ORDER];
            struct node_backlog *top = nbl_stack;

            for (; ;) {
                    if (node != NULL) {
                            /* non-zero needs backward and zero does not */
                            int sub_idx = p_nbl != NULL ? p_nbl->next_sub_idx : 0;
                            /* Reset each loop */
                            p_nbl = NULL;

                            /* Backlog the path */
                            if (node->GetType() == NodeType::BPLUS_TREE_LEAF || sub_idx + 1 >= children(node)) {
                                    top->node = NULL;
                                    top->next_sub_idx = 0;
                            } else {
                                    top->node = node;
                                    top->next_sub_idx = sub_idx + 1;
                            }
                            top++;
                            level++;

                            /* Draw the whole node when the first entry is passed through */
                            if (sub_idx == 0) {
                                    int i;
                                    for (i = 1; i < level; i++) {
                                            if (i == level - 1) {
                                                    printf("%-8s", "+-------");
                                            } else {
                                                    if (nbl_stack[i - 1].node != NULL) {
                                                            printf("%-8s", "|");
                                                    } else {
                                                            printf("%-8s", " ");
                                                    }
                                            }
                                    }
                                    key_print(node);
                            }

                            /* Move deep down */
                            node = node->GetType() == NodeType::BPLUS_TREE_LEAF ? NULL : ((struct BPTreeInternalPage *) node)->ValueAt(sub_idx);
                    } else {
                            p_nbl = top == nbl_stack ? NULL : --top;
                            if (p_nbl == NULL) {
                                    /* End of traversal */
                                    break;
                            }
                            node = p_nbl->node;
                            level--;
                    }
            }
    }

    TEST(test_b_plus_tree, test_main){
        BPlusTree* tree = new BPlusTree(5, 5);
        // for(int i = 0; i <= 10; ++i){
        //     tree->insert(i,1);
        // }
        // bplus_tree_dump(tree);
        std::string line;
        getline(std::cin, line);
        while(line != "a"){
                tree->insert(stoi(line), 1);
                bplus_tree_dump(tree);
                getline(std::cin, line);
        }

        while(true){
                getline(std::cin, line);
                tree->remove(stoi(line));
                bplus_tree_dump(tree);
        }
    }
} // namespace udb