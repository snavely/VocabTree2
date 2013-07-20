/* VocabMatchScript_desc.cpp */
/* Build a database from a vocab tree and score a few images */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <string>

#include "VocabTree.h"
#include "keys2.h"

#include "defines.h"
#include "qsort.h"

#ifdef COLORED_NODES
#include "image.h"
#include "jpegcvt.h"
#include "matrix.h"
#include "util.h"
#endif


unsigned char *ReadDescriptorFile(const char *desc_file, unsigned int dim, 
                                  int &num_keys_out)
{
    FILE *f = fopen(desc_file, "rb");
    if (f == NULL) {
        printf("[ReadDescriptorFile] Error opening file %s for reading\n",
               desc_file);
        return NULL;
    }

    unsigned int num_points = 0;
    int nRead = fread(&num_points, sizeof(unsigned int), 1, f);
    assert(nRead == 1);

    if (num_points == 0) {
        num_keys_out = 0;
        return NULL;
    }
    
    unsigned char *desc = new unsigned char[num_points * dim];
    fread(desc, sizeof(unsigned char), num_points * dim, f);    

    fclose(f);

    num_keys_out = num_points;

    return desc;
}

int BasifyFilename(const char *filename, char *base)
{
    int len = strlen(filename);
    
    int i;
    for (i = len - 1; i >= 0 && filename[i] != '/'; i--) { }

    if (i == -1)
	strcpy(base, filename);
    else
	strcpy(base, filename + i + 1);
    
    base[strlen(base) - 4] = 0;    

    return 0;
}

int main(int argc, char **argv) 
{
    const int dim = 128;

    if (argc != 6 && argc != 7 && argc != 8 && argc != 9 && argc != 10 && 
        argc != 11) {
        printf("Usage: %s <tree.in> <db.in> <query.in> <num_nbrs> "
               "<match-script.out> [leaves_only] [distance_type] [normalize] "
               "[min_feature_scale] [max_keys]\n",
               argv[0]);
        return 1;
    }
    
    char *tree_in = argv[1];
    char *db_in = argv[2];
    char *query_in = argv[3];
    int num_nbrs = atoi(argv[4]);
    char *matches_out = argv[5];
    bool leaves_only = false;
    bool normalize = true;
    double min_feature_scale = 0.0;
    DistanceType distance_type = DistanceMin;
    int max_keys = 0;

    if (argc >= 7) {
        if (atoi(argv[6]) != 0)
            leaves_only = true;
    }

    if (argc >= 8)
        distance_type = (DistanceType) atoi(argv[7]);

    if (argc >= 9)
        if (atoi(argv[8]) == 0)
            normalize = false;

    if (argc >= 10) {
        min_feature_scale = atof(argv[9]);
    }

    if (argc >= 11) {
        max_keys = atoi(argv[10]);
    }

    if (leaves_only) {
        printf("[VocabMatch] Scoring with leaves only\n");
    } else {
        printf("[VocabMatch] Scoring with all nodes\n");
    }

    printf("[VocabMatch] Using tree %s\n", tree_in);
    printf("[VocabMatch] min_feature_scale = %0.3f\n", min_feature_scale);
    printf("[VocabMatch] max_keys = %d\n", max_keys);

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

    /* Read the tree */
    printf("[VocabMatch] Reading tree...\n");
    fflush(stdout);

    clock_t start = clock();
    VocabTree tree;
    tree.Read(tree_in);
    clock_t end = clock();
    printf("[VocabMatch] Read tree in %0.3fs\n", 
           (double) (end - start) / CLOCKS_PER_SEC);

#if 1
    tree.Flatten();
#endif

    tree.SetDistanceType(distance_type);

    if (leaves_only) {
        tree.SetInteriorNodeWeight(atoi(argv[6]) - 1, 0.0);
        // #define CONSTANT_WEIGHTS
#ifdef CONSTANT_WEIGHTS
        tree.SetConstantLeafWeights();
#endif
    }
    
    /* Read the database keyfiles */
    FILE *f = fopen(db_in, "r");
    
    std::vector<std::string> db_files;
    char buf[256];
    while (fgets(buf, 256, f)) {
        /* Remove trailing newline */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        db_files.push_back(std::string(buf));
    }

    fclose(f);

    /* Read the query keyfiles */
    f = fopen(query_in, "r");
    
    std::vector<std::string> query_files;
    std::vector<int> query_indices;
    while (fgets(buf, 256, f)) {
        /* Remove trailing newline */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        char keyfile[256];
        int index;
        sscanf(buf, "%d %s", &index, keyfile);

        query_files.push_back(std::string(keyfile));
        query_indices.push_back(index);
    }

    fclose(f);

    /* Populate the database */
    printf("[VocabMatch] Populating database...\n");
    fflush(stdout);

    int num_db_images = db_files.size();
    int num_query_images = query_files.size();

    /* Finalize the database vectors */
    // tree.ComputeDatabaseVectors(/*magnitudes*/);
    // delete [] magnitudes;

    /* Now score each query keyfile */
    printf("[VocabMatch] Scoring query images...\n");
    fflush(stdout);

    float *scores = new float[num_db_images];
    double *scores_d = new double[num_db_images];
    int *perm = new int[num_db_images];

    FILE *f_match = fopen(matches_out, "w");
    if (f_match == NULL) {
        printf("[VocabMatch] Error opening file %s for writing\n",
               matches_out);
        return 1;
    }

    for (int i = 0; i < num_query_images; i++) {
        int index_i = query_indices[i];

        start = clock();

        /* Clear scores */
        for (int j = 0; j < num_db_images; j++) 
            scores[j] = 0.0;

        unsigned char *keys;
        int num_keys;

        keys = ReadDescriptorFile(query_files[i].c_str(), dim, num_keys);

        tree.ScoreQueryKeys(num_keys, /*i,*/ true, keys, scores);

        end = clock();
        printf("[VocabMatch] Scored image %s (%d keys) in %0.3fs\n", 
               query_files[i].c_str(), num_keys,
               (double) (end - start) / CLOCKS_PER_SEC);

#if 0
        for (int j = 0; j < num_db_images; j++) {
            /* Normalize scores */
            if (magnitudes[j] > 0.0)
                scores[j] /= magnitudes[j];
            else 
                scores[j] = 0.0;
        }
#endif

        /* Find the top scores */
        for (int j = 0; j < num_db_images; j++) {
            scores_d[j] = (double) scores[j];
        }

        qsort_descending();
        qsort_perm(num_db_images, scores_d, perm);        
        // assert(is_sorted(num_db_images, scores_d));

        int top = MIN(num_nbrs+1, num_db_images);

        for (int j = 0; j < top; j++) {
            if (perm[j] == index_i)
                continue;
            fprintf(f_match, "%d %d %0.5e\n", index_i, perm[j], scores_d[j]);
            fflush(f_match);
        }
        
        fflush(stdout);

        delete [] keys;
    }

    fclose(f_match);

    delete [] scores;
    delete [] scores_d;
    delete [] perm;

    return 0;
}
