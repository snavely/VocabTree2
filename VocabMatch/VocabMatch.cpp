/* VocabMatch.cpp */
/* Build a database from a vocab tree and score a few images */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctime>

#include <string>

#include "VocabTree.h"
#include "keys2.h"

#include "defines.h"
#include "qsort.h"

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

int BasifyFilename(const char *filename, char *base)
{
    strcpy(base, filename);
    base[strlen(base) - 4] = 0;    

    return 0;
}

/* Functions for outputting a webpage synopsis of the results */
void PrintHTMLHeader(FILE *f, int num_nns) 
{
    fprintf(f, "<html>\n"
            "<header>\n"
            "<title>Vocabulary tree results</title>\n"
            "</header>\n"
            "<body>\n"
            "<h1>Vocabulary tree results</h1>\n"
            "<hr>\n\n");

    fprintf(f, "<table border=2 align=center>\n<tr>\n<th>Query image</th>\n");
    for (int i = 0; i < num_nns; i++) {
        fprintf(f, "<th>Match %d</th>\n", i+1);
    }
    fprintf(f, "</tr>\n");
}

void PrintHTMLRow(FILE *f, const std::string &query, 
                  double *scores, int *perm, int num_nns,
                  const std::vector<std::string> &db_images)
{
    char q_base[512], q_thumb[512];
    BasifyFilename(query.c_str(), q_base);
    sprintf(q_thumb, "%s.thumb.jpg", q_base);

    fprintf(f, "<tr align=center>\n<td><img src=\"%s\"</td>\n", q_thumb);

    for (int i = 0; i < num_nns; i++) {
        char d_base[512], d_thumb[512];
        BasifyFilename(db_images[perm[i]].c_str(), d_base);
        sprintf(d_thumb, "%s.thumb.jpg", d_base);

        fprintf(f, "<td><img src=\"%s\"</td>\n", d_thumb);
    }
            
    fprintf(f, "</tr>\n<tr align=right>\n");

    fprintf(f, "<td></td>\n");
    for (int i = 0; i < num_nns; i++) 
        fprintf(f, "<td>%0.5f</td>\n", scores[i]);

    fprintf(f, "</tr>\n");
}

void PrintHTMLFooter(FILE *f) 
{
    fprintf(f, "</tr>\n"
            "</table>\n"
            "<hr>\n"
            "</body>\n"
            "</html>\n");
}

int main(int argc, char **argv) 
{
    const int dim = 128;

    if (argc != 6 && argc != 7 && argc != 8) {
        printf("Usage: %s <tree.in> <db.in> <query.in> <num_nbrs> "
               "<matches.out> [output.html] [distance_type]\n", argv[0]);
        return 1;
    }
    
    char *tree_in = argv[1];
    char *db_in = argv[2];
    char *query_in = argv[3];
    int num_nbrs = atoi(argv[4]);
    char *matches_out = argv[5];
    DistanceType distance_type = DistanceMin;
    char *output_html = "results.html";
    
    if (argc >= 7)
        output_html = argv[6];

    if (argc >= 8)
        distance_type = (DistanceType) atoi(argv[7]);

    printf("[VocabMatch] Using tree %s\n", tree_in);

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
    tree.SetInteriorNodeWeight(0, 0.0);
    
    /* Read the database keyfiles */
    FILE *f = fopen(db_in, "r");
    
    std::vector<std::string> db_files;
    std::vector<int> db_landmarks;
    char buf[256];
    while (fgets(buf, 256, f)) {
        /* Remove trailing newline */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        char filename[256];
        int landmark;
        sscanf(buf, "%s %d", filename, &landmark);
        db_files.push_back(std::string(filename));
        db_landmarks.push_back(landmark);
    }

    fclose(f);

    /* Read the query keyfiles */
    f = fopen(query_in, "r");
    
    std::vector<std::string> query_files;
    while (fgets(buf, 256, f)) {
        /* Remove trailing newline */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        char keyfile[256];
        sscanf(buf, "%s", keyfile);

        query_files.push_back(std::string(keyfile));
    }

    fclose(f);

    int num_db_images = db_files.size();
    int num_query_images = query_files.size();

    /* Now score each query keyfile */
    printf("[VocabMatch] Scoring query images...\n");
    fflush(stdout);

    FILE *f_html = fopen(output_html, "w");
    PrintHTMLHeader(f_html, num_nbrs);

    float *scores = new float[num_db_images];
    double *scores_d = new double[num_db_images];
    int *perm = new int[num_db_images];

    int max_ld = 0;
    for (int i = 0; i < num_db_images; i++) {
        if (db_landmarks[i] > max_ld){
            max_ld = db_landmarks[i];
        }
    }

    FILE *f_match = fopen(matches_out, "w");
    if (f_match == NULL) {
        printf("[VocabMatch] Error opening file %s for writing\n",
               matches_out);
        return 1;
    }

    for (int i = 0; i < num_query_images; i++) {
        start = clock();

        /* Clear scores */
        for (int j = 0; j < num_db_images; j++) 
            scores[j] = 0.0;

        unsigned char *keys;
        int num_keys;

        keys = ReadKeys(query_files[i].c_str(), dim, num_keys);

        clock_t start_score = clock();
        tree.ScoreQueryKeys(num_keys, true, keys, scores);
        clock_t end_score = end = clock();

        printf("[VocabMatch] Scored image %s in %0.3fs "
               "( %0.3fs total, num_keys = %d )\n", 
               query_files[i].c_str(), 
               (double) (end_score - start_score) / CLOCKS_PER_SEC,
               (double) (end - start) / CLOCKS_PER_SEC, num_keys);

        /* Find the top scores */
        for (int j = 0; j < num_db_images; j++) {
            scores_d[j] = (double) scores[j];
        }

        qsort_descending();
        qsort_perm(num_db_images, scores_d, perm);        

#ifdef SKELETON_CODE
        /* *** TODO 9 ***
         *
         * You'll need to implement this part.  The scores_d array now
         * contains the similarities of the top matching database
         * image to the query image, and perm contains the
         * corresponding database image indices.  You'll need to use
         * k-nearest neighbors to compute the best landmark.  The
         * landmark ID for each database image is stored in the vector
         * db_landmarks.  When you're done, max_landmark should
         * contain your answer, and max_votes the support that it has.
         * If you'd like to try a more sophisticated method than raw
         * knn, please feel free.
         */

        int max_landmark = 0, max_votes = 0;

        /* END TODO */
#else
        int max_landmark = 0, max_votes = 0;
        int top = MIN(num_nbrs, num_db_images);

        int *votes = new int[max_ld+1];

        for (int j = 0; j < max_ld+1; j++){
            votes[j] = 0;
        }

        for (int j = 0; j < top; j++) {
            votes[db_landmarks[perm[j]]]++;
        }
        
        max_votes = 0;
        max_landmark = -1;
        for (int j = 0; j < max_ld+1; j++) {
            if (votes[j] > max_votes) {
                max_votes = votes[j];
                max_landmark = j;
            }
        }
#endif /* SKELETON_CODE */

        /* Print the matching information to a file */
        fprintf(f_match, "%d %d %d\n", i, max_landmark, max_votes);
        fflush(f_match);
        fflush(stdout);

        PrintHTMLRow(f_html, query_files[i], scores_d, 
                     perm, num_nbrs, db_files);

        delete [] keys;
    }

    fclose(f_match);

    PrintHTMLFooter(f_html);
    fclose(f_html);

    delete [] scores;
    delete [] scores_d;
    delete [] perm;

    return 0;
}
