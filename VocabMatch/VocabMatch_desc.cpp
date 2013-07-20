/* 
 * Copyright 2011-2012 Noah Snavely, Cornell University
 * (snavely@cs.cornell.edu).  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NOAH SNAVELY ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NOAH SNAVELY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * The views and conclusions contained in the software and
 * documentation are those of the authors and should not be
 * interpreted as representing official policies, either expressed or
 * implied, of Cornell University.
 *
 */

/* VocabMatch_desc.cpp */
/* Read a database stored as a vocab tree and score a set of query
 * images (stored as descriptor files) */

#include <assert.h>
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

#if 0
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

int main(int argc, char **argv) 
{
    const int dim = 128;

    if (argc != 6 && argc != 7 && argc != 8) {
        printf("Usage: %s <tree.in> <db.in> <query.in> <num_nbrs> "
               "<matches.out> [distance_type:1] [normalize:1]\n", argv[0]);
        return 1;
    }
    
    char *tree_in = argv[1];
    char *db_in = argv[2];
    char *query_in = argv[3];
    int num_nbrs = atoi(argv[4]);
    char *matches_out = argv[5];
    DistanceType distance_type = DistanceMin;
    bool normalize = true;

#if 0    
    if (argc >= 7)
        output_html = argv[6];
#endif

    if (argc >= 7)
        distance_type = (DistanceType) atoi(argv[6]);

    if (argc >= 8)
        normalize = (atoi(argv[7]) != 0);

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

    printf("[VocabMatch] Read %d database images\n", num_db_images);

    /* Now score each query keyfile */
    printf("[VocabMatch] Scoring %d query images...\n", num_query_images);
    fflush(stdout);

#if 0
    FILE *f_html = fopen(output_html, "w");
    PrintHTMLHeader(f_html, num_nbrs);
#endif

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
        start = clock();

        /* Clear scores */
        for (int j = 0; j < num_db_images; j++) 
            scores[j] = 0.0;

        int num_keys = 0;
        unsigned char *keys = ReadDescriptorFile(query_files[i].c_str(), 
                                                 dim, num_keys);

        clock_t start_score = clock();
        double mag = tree.ScoreQueryKeys(num_keys, normalize, keys, scores);
        clock_t end_score = end = clock();

        printf("[VocabMatch] Scored image %s in %0.3fs "
               "( %0.3fs total, num_keys = %d, mag = %0.3f )\n", 
               query_files[i].c_str(), 
               (double) (end_score - start_score) / CLOCKS_PER_SEC,
               (double) (end - start) / CLOCKS_PER_SEC, num_keys, mag);

        /* Find the top scores */
        for (int j = 0; j < num_db_images; j++) {
            scores_d[j] = (double) scores[j];
        }

        qsort_descending();
        qsort_perm(num_db_images, scores_d, perm);        

        int top = MIN(num_nbrs, num_db_images);

        for (int j = 0; j < top; j++) {
            // if (perm[j] == index_i)
            //     continue;
            
            fprintf(f_match, "%d %d %0.4f\n", i, perm[j], scores_d[j]);
            //fprintf(f_match, "%d %d %0.4f\n", i, perm[j], mag - scores_d[j]);
        }
        
        fflush(f_match);
        fflush(stdout);

#if 0
        PrintHTMLRow(f_html, query_files[i], scores_d, 
                     perm, num_nbrs, db_files);
#endif

        delete [] keys;
    }

    fclose(f_match);

#if 0
    PrintHTMLFooter(f_html);
    fclose(f_html);
#endif

    delete [] scores;
    delete [] scores_d;
    delete [] perm;

    return 0;
}
