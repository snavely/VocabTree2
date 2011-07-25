/* VocabBuildDB.cpp */
/* Driver for building a database with a vocabulary tree */

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys2.h"
#include "VocabTree.h"

unsigned char *ReadAndFilterKeys(const char *keyfile, int dim, 
                                 double min_feature_scale, 
                                 int max_keys, int &num_keys_out)
{
    short int *keys;
    keypt_t *info = NULL;
    int num_keys = ReadKeyFile(keyfile, &keys, &info);
    
    if (num_keys == 0) {
        num_keys_out = 0;
        return NULL;
    }
    
    /* Filter keys */
    unsigned char *keys_char = new unsigned char[num_keys * dim];
        
    int num_keys_filtered = 0;
    if (min_feature_scale == 0.0 && max_keys == 0) {
        for (int j = 0; j < num_keys * dim; j++) {
            keys_char[j] = (unsigned char) keys[j];
        }
        num_keys_filtered = num_keys;
    } else {
        for (int j = 0; j < num_keys; j++) {
            if (info[j].scale < min_feature_scale)
                continue;
            
            for (int k = 0; k < dim; k++) {
                keys_char[num_keys_filtered * dim + k] = 
                    (unsigned char) keys[j * dim + k];
            }
            
            num_keys_filtered++;

            if (max_keys > 0 && num_keys_filtered >= max_keys)
                break;
        }
    }

    delete [] keys;

    if (info != NULL) 
        delete [] info;

    num_keys_out = num_keys_filtered;

    return keys_char;
}

int main(int argc, char **argv) 
{
    if (argc < 4 || argc > 8) {
        printf("Usage: %s <list.in> <tree.in> <tree.out> [use_tfidf:1] "
               "[normalize:1] [start_id:0] [distance_type:1]\n", 
               argv[0]);

        return 1;
    }

    double min_feature_scale = 1.4;
    bool use_tfidf = true;
    bool normalize = true;
    
    char *list_in = argv[1];
    char *tree_in = argv[2];
    char *tree_out = argv[3];
    DistanceType distance_type = DistanceMin;
    int start_id = 0;

    if (argc >= 5)
        use_tfidf = atoi(argv[4]);

    if (argc >= 6)
        normalize = atoi(argv[5]);

    if (argc >= 7)
        start_id = atoi(argv[6]);

    if (argc >= 8)
        distance_type = (DistanceType) atoi(argv[7]);

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

#if 1
    tree.Flatten();
#endif

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
        unsigned char *keys = ReadAndFilterKeys(key_files[i].c_str(), 
                                                dim, min_feature_scale,
                                                0, num_keys);

        printf("[VocabBuildDB] Adding vector %d (%d keys)\n", 
               start_id + i, num_keys);
        tree.AddImageToDatabase(start_id + i, num_keys, keys);

        if (num_keys > 0) 
            delete [] keys;
    }

    printf("[VocabBuildDB] Pushed %lu features\n", count);
    fflush(stdout);

    if (use_tfidf)
        tree.ComputeTFIDFWeights(num_db_images);

    if (normalize) 
        tree.NormalizeDatabase(start_id, num_db_images);

    printf("[VocabBuildDB] Writing tree...\n");
    tree.Write(tree_out);

    // char filename[256];
    // sprintf(filename, "vectors_%03d.txt", start_id);
    // tree.WriteDatabaseVectors(filename, start_id, num_db_images);

    return 0;
}
