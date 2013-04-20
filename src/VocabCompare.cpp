/* VocabBuildDB.cpp */
/* Driver for building a database with a vocabulary tree */

#include <string>
#include <vector>
#include <map>

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
    if (argc != 5 && argc != 6) {
        printf("Usage: %s <tree.in> <image1.key> <image2.key> <matches.out> "
               "[distance_type]\n", 
               argv[0]);

        return 1;
    }
    
    char *tree_in = argv[1];
    char *image1_in = argv[2];
    char *image2_in = argv[3];
    char *matches_out = argv[4];
    DistanceType distance_type = DistanceMin;

    if (argc >= 6)
        distance_type = (DistanceType) atoi(argv[5]);

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

    printf("[VocabBuildDB] Reading tree %s...\n", tree_in);
    fflush(stdout);

    VocabTree tree;
    tree.Read(tree_in);

    printf("[VocabCompare] Flattening tree...\n");
    tree.Flatten();

    tree.m_distance_type = distance_type;
    tree.SetInteriorNodeWeight(0.0);

    /* Initialize leaf weights to 1.0 */
    tree.SetConstantLeafWeights();

    const int dim = 128;

    tree.ClearDatabase();

    int num_keys_1 = 0, num_keys_2 = 0;
    unsigned char *keys1 = ReadKeys(image1_in, dim, num_keys_1);
    unsigned char *keys2 = ReadKeys(image2_in, dim, num_keys_2);

    unsigned long *ids1 = new unsigned long[num_keys_1];
    unsigned long *ids2 = new unsigned long[num_keys_2];

    printf("[VocabCompare] Adding image 0 (%d keys)\n", num_keys_1);
    tree.AddImageToDatabase(0, num_keys_1, keys1, ids1);

    printf("[VocabCompare] Adding image 1 (%d keys)\n", num_keys_2);
    tree.AddImageToDatabase(1, num_keys_2, keys2, ids2);

    if (num_keys_1 > 0) 
        delete [] keys1;

    if (num_keys_2 > 0)
        delete [] keys2;

    // tree.ComputeTFIDFWeights();
    tree.NormalizeDatabase(0, 2);

    /* Find collisions among visual word IDs */
    std::multimap<unsigned long, unsigned int> word_map;
    
    for (unsigned int i = 0; i < (unsigned int) num_keys_1; i++) {
        printf("0 %d -> %lu\n", i, ids1[i]);
        std::pair<unsigned long, unsigned int> elem(ids1[i], i);
        word_map.insert(elem);
    }

    /* Count number of matches */
    int num_matches = 0;
    for (unsigned int i = 0; i < (unsigned int) num_keys_2; i++) {
        unsigned long id = ids2[i];
        printf("1 %d -> %lu\n", i, ids2[i]);

        std::pair<std::multimap<unsigned long, unsigned int>::iterator,
                  std::multimap<unsigned long, unsigned int>::iterator> ret;
        ret = word_map.equal_range(id);

        unsigned int size = 0;
        std::multimap<unsigned long, unsigned int>::iterator iter;
        for (iter = ret.first; iter != ret.second; iter++) {
            size++;
            num_matches++;
        }

        if (size > 0)
            printf("size[%lu] = %d\n", id, size);
    }

    printf("number of matches: %d\n", num_matches);
    fflush(stdout);

    FILE *f = fopen(matches_out, "w");
    
    if (f == NULL) {
        printf("Error opening file %s for writing\n", matches_out);
        return 1;
    }

    fprintf(f, "%d\n", num_matches);

    /* Write matches */
    for (unsigned int i = 0; i < (unsigned int) num_keys_2; i++) {
        unsigned long id = ids2[i];
        
        std::pair<std::multimap<unsigned long, unsigned int>::iterator,
                  std::multimap<unsigned long, unsigned int>::iterator> ret;
        ret = word_map.equal_range(id);

        std::multimap<unsigned long, unsigned int>::iterator iter;
        for (iter = ret.first; iter != ret.second; iter++) {
            fprintf(f, "%d %d\n", iter->second, i);
        }
    }

    fclose(f);

    // printf("[VocabBuildDB] Writing tree...\n");
    // tree.Write(tree_out);

    return 0;
}
