/* VocabCombine.cpp */
/* Driver for combining several vocab trees into one */

#include <string>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys2.h"
#include "VocabTree.h"

int main(int argc, char **argv) 
{
    if (argc < 3) {
        printf("Usage: %s <tree1.in> <tree2.in> ... <tree.out>\n",
               argv[0]);

        return 1;
    }

    int num_trees = argc - 2;
    
    char *tree_out = argv[argc-1];

    VocabTree tree;
    tree.Read(argv[1]);

    /* Start with the second tree, as we just read the first one */
    for (int i = 1; i < num_trees; i++) {
        printf("[VocabCombine] Adding tree %d [%s]...\n", i, argv[i+1]);
        fflush(stdout);

        VocabTree tree_add;
        tree_add.Read(argv[i+1]);
        tree.Combine(tree_add);
        tree_add.Clear();
    }

    /* Now do the reweighting */
    // if (use_tfidf)
    int total_num_db_images = tree.GetMaxDatabaseImageIndex() + 1;
    printf("Total num_db_images: %d\n", total_num_db_images);
    tree.ComputeTFIDFWeights(total_num_db_images);
    tree.NormalizeDatabase(0, total_num_db_images);
    tree.Write(tree_out);

    /* Write vectors to a file */
    tree.WriteDatabaseVectors("vectors_all.txt", 0, total_num_db_images);

    return 0;
}
