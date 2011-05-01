/* VocabBuildDB.cpp */
/* Driver for building a database with a vocabulary tree */

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys2.h"
#include "VocabTree.h"

/* Read in a set of keys from a file 
 *
 * Inputs:
 *   keyfile      : file from which to read keys
 *   dim          : dimensionality of descriptors
 * 
 * Outputs:
 *   num_keys_out : number of keys read
 *
 * Return value   : pointer to array of descriptors.  The descriptors
 *                  are concatenated together in one big array of
 *                  length num_keys_out * dim 
 */
unsigned char *ReadKeys(const char *keyfile, int dim, int &num_keys_out)
{
    short int *keys;
    keypt_t *info = NULL;
    int num_keys = ReadKeyFile(keyfile, &keys, &info);
    
    unsigned char *keys_char = new unsigned char[num_keys * dim];
        
    for (int j = 0; j < num_keys * dim; j++) {
        keys_char[j] = (unsigned char) keys[j];
    }

    delete [] keys;

    if (info != NULL) 
        delete [] info;

    num_keys_out = num_keys;

    return keys_char;
}

int main(int argc, char **argv) 
{
    if (argc != 4 && argc != 5) {
        printf("Usage: %s <list.in> <tree.in> <tree.out> [distance_type]\n", 
               argv[0]);

        return 1;
    }
    
    char *list_in = argv[1];
    char *tree_in = argv[2];
    char *tree_out = argv[3];
    DistanceType distance_type = DistanceMin;

    if (argc >= 5)
        distance_type = (DistanceType) atoi(argv[4]);

    switch (distance_type) {
    case DistanceDot:
        printf("[VocabMatch] Using distance Dot\n");
        break;        
    case DistanceMin:
        printf("[VocabMatch] Using distance Min\n");
        break;
    default:
        printf("[VocabMatch] Using no known distance!\n");
        break;
    }

    FILE *f = fopen(list_in, "r");
    
    if (f == NULL) {
        printf("Error opening file %s for reading\n", list_in);
        return 1;
    }

    std::vector<std::string> key_files;
    char buf[256];
    while (fgets(buf, 256, f)) {
        /* Remove trailing newline */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        key_files.push_back(std::string(buf));
    }

    printf("[VocabBuildDB] Reading tree %s...\n", tree_in);
    fflush(stdout);

    VocabTree tree;
    tree.Read(tree_in);

    tree.m_distance_type = distance_type;
    tree.SetInteriorNodeWeight(0.0);

    /* Initialize leaf weights to 1.0 */
    tree.SetConstantLeafWeights();

    const int dim = 128;
    int num_db_images = (int) key_files.size();
    unsigned long count = 0;

    tree.ClearDatabase();

    for (int i = 0; i < num_db_images; i++) {
        int num_keys = 0;
        unsigned char *keys = ReadKeys(key_files[i].c_str(), dim, num_keys);

        printf("[VocabBuildDB] Adding vector %d (%d keys)\n", i, num_keys);
        tree.AddImageToDatabase(i, num_keys, keys);

        if (num_keys > 0) 
            delete [] keys;
    }

    printf("[VocabBuildDB] Pushed %lu features\n", count);
    fflush(stdout);

    tree.ComputeTFIDFWeights();
    tree.NormalizeDatabase(num_db_images);

    printf("[VocabBuildDB] Writing tree...\n");
    tree.Write(tree_out);

    return 0;
}
