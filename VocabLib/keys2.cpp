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

/* keys2.cpp */
/* Class for SIFT keypoints */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
//#include <unistd.h>
//#include <sys/mman.h>
#include <sys/stat.h>

#include <zlib.h>

#include "keys2.h"

int GetNumberOfKeysNormal(FILE *fp)
{
    int num, len;

    if (fscanf(fp, "%d %d", &num, &len) != 2) {
	printf("Invalid keypoint file.\n");
	return 0;
    }

    return num;
}

int GetNumberOfKeysGzip(gzFile fp)
{
    int num, len;

    char header[256];
    gzgets(fp, header, 256);

    if (sscanf(header, "%d %d", &num, &len) != 2) {
	printf("Invalid keypoint file.\n");
	return 0;
    }

    return num;
}

/* Returns the number of keys in a file */
int GetNumberOfKeys(const char *filename)
{
    FILE *file;

    file = fopen (filename, "r");
    if (! file) {
        /* Try to file a gzipped keyfile */
        char buf[1024];
        sprintf(buf, "%s.gz", filename);
        gzFile gzf = gzopen(buf, "rb");

        if (gzf == NULL) {
            printf("Could not open file: %s\n", filename);
            return 0;
        } else {
            int n = GetNumberOfKeysGzip(gzf);
            gzclose(gzf);
            return n;
        }
    }
    
    int n = GetNumberOfKeysNormal(file);
    fclose(file);
    return n;
}

/* This reads a keypoint file from a given filename and returns the list
 * of keypoints. */
int ReadKeyFile(const char *filename, short int **keys, keypt_t **info)
{
    FILE *file;

    file = fopen (filename, "r");
    if (! file) {
        /* Try to file a gzipped keyfile */
        char buf[1024];
        sprintf(buf, "%s.gz", filename);
        gzFile gzf = gzopen(buf, "rb");

        if (gzf == NULL) {
            printf("Could not open file: %s\n", filename);
            return 0;
        } else {
            int n = ReadKeysGzip(gzf, keys, info);
            gzclose(gzf);
            return n;
        }
    }
    
    int n = ReadKeys(file, keys, info);
    fclose(file);
    return n;

    // return ReadKeysMMAP(file);
}

#if 0
/* Read keys using MMAP to speed things up */
std::vector<Keypoint *> ReadKeysMMAP(FILE *fp) 
{    
    int i, j, num, len, val, n;

    std::vector<Keypoint *> kps;

    struct stat sb;

    /* Stat the file */
    if (fstat(fileno(fp), &sb) < 0) {
	printf("[ReadKeysMMAP] Error: could not stat file\n");
	return kps;
    }

    char *file = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, 
			      fileno(fp), 0);

    char *file_start = file;

    if (sscanf(file, "%d %d%n", &num, &len, &n) != 2) {
	printf("[ReadKeysMMAP] Invalid keypoint file beginning.");
	return kps;
    }

    file += n;

    if (len != 128) {
	printf("[ReadKeysMMAP] Keypoint descriptor length invalid "
	       "(should be 128).");
	return kps;
    }

    for (i = 0; i < num; i++) {
	/* Allocate memory for the keypoint. */
	unsigned char *d = new unsigned char[len];
	float x, y, scale, ori;

	if (sscanf(file, "%f %f %f %f%n", &y, &x, &scale, &ori, &n) != 4) {
	    printf("[ReadKeysMMAP] Invalid keypoint file format.");
	    return kps;
	}

	file += n;

	for (j = 0; j < len; j++) {
	    if (sscanf(file, "%d%n", &val, &n) != 1 || val < 0 || val > 255) {
		printf("[ReadKeysMMAP] Invalid keypoint file value.");
		return kps;
	    }
	    d[j] = (unsigned char) val;
	    file += n;
	}

	kps.push_back(new Keypoint(x, y, scale, ori, d));
    }

    /* Unmap */
    if (munmap(file_start, sb.st_size) < 0) {
	printf("[ReadKeysMMAP] Error: could not unmap memory\n");
	return kps;
    }

    return kps;    
}
#endif

/* Read keypoints from the given file pointer and return the list of
 * keypoints.  The file format starts with 2 integers giving the total
 * number of keypoints and the size of descriptor vector for each
 * keypoint (currently assumed to be 128). Then each keypoint is
 * specified by 4 floating point numbers giving subpixel row and
 * column location, scale, and orientation (in radians from -PI to
 * PI).  Then the descriptor vector for each keypoint is given as a
 * list of integers in range [0,255]. */
int ReadKeys(FILE *fp, short int **keys, keypt_t **info)
{
    int i, num, len;

    std::vector<Keypoint *> kps;

    if (fscanf(fp, "%d %d", &num, &len) != 2) {
	printf("Invalid keypoint file\n");
	return 0;
    }

    if (len != 128) {
	printf("Keypoint descriptor length invalid (should be 128).");
	return 0;
    }

    *keys = new short int[128 * num];

    if (info != NULL) 
        *info = new keypt_t[num];

    short int *p = *keys;
    for (i = 0; i < num; i++) {
	/* Allocate memory for the keypoint. */
	// short int *d = new short int[128];
	float x, y, scale, ori;

	if (fscanf(fp, "%f %f %f %f\n", &y, &x, &scale, &ori) != 4) {
	    printf("Invalid keypoint file format.");
	    return 0;
	}

        if (info != NULL) {
            (*info)[i].x = x;
            (*info)[i].y = y;
            (*info)[i].scale = scale;
            (*info)[i].orient = ori;
        }
        
	char buf[1024];
	for (int line = 0; line < 7; line++) {
	    fgets(buf, 1024, fp);

	    if (line < 6) {
		sscanf(buf, 
		       "%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu "
		       "%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu", 
		       p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8, p+9, 
		       p+10, p+11, p+12, p+13, p+14, 
		       p+15, p+16, p+17, p+18, p+19);

		p += 20;
	    } else {
		sscanf(buf, 
		       "%hu %hu %hu %hu %hu %hu %hu %hu",
		       p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);
		p += 8;
	    }
	}
    }

    return num; // kps;
}

int WriteBinaryKeyFile(const char *filename, int num_keys, 
                       const short int *keys, const keypt_t *info)
{
    FILE *f = fopen(filename, "wb");
    
    if (f == NULL) {
        printf("[WriteBinaryKeyFile] Error opening file %s for writing\n",
               filename);

        return 0;
    }

    fwrite(&num_keys, sizeof(int), 1, f);
    fwrite(info, sizeof(keypt_t), num_keys, f);
    fwrite(keys, sizeof(short int), num_keys * 128, f);

    fclose(f);

    return num_keys;
}

int ReadKeysGzip(gzFile fp, short int **keys, keypt_t **info)
{
    int i, num, len;

    std::vector<Keypoint *> kps;
    char header[256];
    gzgets(fp, header, 256);

    if (sscanf(header, "%d %d", &num, &len) != 2) {
	printf("Invalid keypoint file.\n");
	return 0;
    }

    if (len != 128) {
	printf("Keypoint descriptor length invalid (should be 128).");
	return 0;
    }

    *keys = new short int[128 * num];

    if (info != NULL) 
        *info = new keypt_t[num];

    short int *p = *keys;
    for (i = 0; i < num; i++) {
	/* Allocate memory for the keypoint. */
	// short int *d = new short int[128];
	float x, y, scale, ori;
        char buf[1024];
        gzgets(fp, buf, 1024);

	if (sscanf(buf, "%f %f %f %f\n", &y, &x, &scale, &ori) != 4) {
	    printf("Invalid keypoint file format.");
	    return 0;
	}

        if (info != NULL) {
            (*info)[i].x = x;
            (*info)[i].y = y;
            (*info)[i].scale = scale;
            (*info)[i].orient = ori;
        }
        
	for (int line = 0; line < 7; line++) {
	    gzgets(fp, buf, 1024);

	    if (line < 6) {
		sscanf(buf, 
		       "%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu "
		       "%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu", 
		       p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8, p+9, 
		       p+10, p+11, p+12, p+13, p+14, 
		       p+15, p+16, p+17, p+18, p+19);

		p += 20;
	    } else {
		sscanf(buf, 
		       "%hu %hu %hu %hu %hu %hu %hu %hu",
		       p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);
		p += 8;
	    }
	}
    }

    return num; // kps;
}

std::vector<KeypointMatch> 
  RemoveDuplicateMatches(const std::vector<KeypointMatch> &matches)
{
    int num_matches = (int) matches.size();
    std::vector<KeypointMatch> matches_new;

    int num_matches_new = 0;
    for (int i = 0; i < num_matches; i++) {
	int target = matches[i].m_idx2;
	
	bool duplicate = false;
	for (int j = 0; j < num_matches_new; j++) {
	    if (matches_new[j].m_idx2 == target) {
		duplicate = true;
		break;
	    }
	}

	if (!duplicate)
	    matches_new.push_back(matches[i]);

        num_matches_new = (int) matches_new.size();
    }

    return matches_new;
}
