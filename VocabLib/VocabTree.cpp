/* VocabTree.cpp */
/* Build a vocabulary tree from a set of vectors */

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "VocabTree.h"
#include "defines.h"
#include "qsort.h"
#include "util.h"

/* Useful utility function for computing the squared distance between
 * two vectors a and b of length dim */
static unsigned long vec_diff_normsq(int dim, 
                                     unsigned char *a, 
                                     unsigned char *b)
{
    int i;
    unsigned long normsq = 0;

    for (i = 0; i < dim; i++) {
        int d = (int) a[i] - (int) b[i];
        normsq += d * d;
    }

    return normsq;
}

VocabTreeInteriorNode::~VocabTreeInteriorNode()
{
    if (m_children != NULL) 
        delete [] m_children;

    if (m_desc != NULL) 
        delete [] m_desc;
}

VocabTreeLeaf::~VocabTreeLeaf()
{
    if (m_desc != NULL)
        delete [] m_desc;
}

unsigned long VocabTreeInteriorNode::
    PushAndScoreFeature(unsigned char *v, 
                        unsigned int index, int bf, int dim, bool add)
{
#ifdef SKELETON_CODE
    /* *** TODO 5 ***
     *
     * You'll need to implement this part of the code.  This function
     * will need to take the input descriptor v and recursively push
     * it down the tree by finding the closest child descriptor at
     * each node.
     */

    return 0;
#else
    unsigned long min_dist = ULONG_MAX;
    int best_idx = 0;

    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            unsigned long dist = 
                vec_diff_normsq(dim, m_children[i]->m_desc, v);

            if (dist < min_dist) {
                min_dist = dist;
                best_idx = i;
            }
        }
    }    

    unsigned long r = 
        m_children[best_idx]->PushAndScoreFeature(v, index, bf, dim, add);

    return r;
#endif /* SKELETON_CODE */
}

unsigned long VocabTreeLeaf::PushAndScoreFeature(unsigned char *v, 
                                                 unsigned int index, 
                                                 int bf, int dim, 
                                                 bool add) 
{
    /* *** TODO 6 ***
     * 
     * Just kidding, we've implemented this function for you, but you
     * should still read this comment.  When a descriptor reaches a
     * leaf node, the temporary m_score member variable is
     * incremented, and the inverted file is optionally updated (if
     * add is true). 
     */

    /* Update the temporary score counter m_score */
    m_score += m_weight;

    if (add) {
        /* Update the inverted file */
        AddFeatureToInvertedFile(index, bf, dim);
    }
    
    return m_id;
}

int VocabTreeLeaf::AddFeatureToInvertedFile(unsigned int index, 
                                            int bf, int dim)
{
    /* Update the inverted file */
    int n = (int) m_image_list.size();

    if (n == 0) {
        m_image_list.push_back(ImageCount(index, (float) m_weight));
    } else {
        if (m_image_list[n-1].m_index == index) {
            m_image_list[n-1].m_count += m_weight;
        } else {
            m_image_list.
                push_back(ImageCount(index, (float) m_weight));
        }
    }

    return 0;
}

double VocabTreeInteriorNode::ComputeTFIDFWeights(int bf, double n)
{
    /* Compute TFIDF weights for all leaf nodes (visual words) */
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
             m_children[i]->ComputeTFIDFWeights(bf, n);
        }
    }

    return 0;
}

double VocabTreeLeaf::ComputeTFIDFWeights(int bf, double n)
{
    int len = (int) m_image_list.size();

#ifdef SKELETON_CODE
    /* *** TODO 6 (real) ***
     * 
     * This part of the code sets the TFIDF weights of this leaf node
     * (i.e., this visual word), according to how many documents this
     * word is visible in.  Using the definition of TFIDF given in
     * class, you will set the m_weight member variable to the IDF
     * part of the weight (instead of setting to 1 as below).  This
     * function will be called after the database is populated, so the
     * information you need to compute the IDF weight can be derived
     * from the inverted file m_image_list.
     */

    m_weight = 1.0;

    /* END TODO */
#else
	
    if (len > 0)
        m_weight = log((double) n / (double) len);
    else
        m_weight = 0.0;
	
#endif /* SKELETON_CODE */

    /* We'll pre-apply weights to the count values (TF scores) in the
     * inverted file.  We took care of this for you. */
    for (int i = 0; i < len; i++) {
        m_image_list[i].m_count *= m_weight;
    }

    return 0;
}

int VocabTreeInteriorNode::FillQueryVector(float *q, int bf, 
                                           double mag_inv)
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->FillQueryVector(q, bf, mag_inv);
        }
    }

    return 0;
}

int VocabTreeLeaf::FillQueryVector(float *q, int bf, double mag_inv)
{
    q[m_id] = m_score * mag_inv;
    return 0;
}

int VocabTreeInteriorNode::ScoreQuery(float *q, int bf, DistanceType dtype, 
                                      float *scores)
{
#ifdef SKELETON_CODE
    /* *** TODO 7 ***
     * 
     * You'll need to fill in this function.  This function, and the
     * version for VocabTreeLeaf, will compute the similarity between
     * the query histogram (stored in q), and the database images,
     * stored in the inverted file.  This function is very simple, and
     * should take less than 12 lines (not counting comments) 
     */

    return 0;
#else
    /* Pass the scores to the children for updating */
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->ScoreQuery(q, bf, dtype, scores);
        }
    }

    return 0;
#endif /* SKELETON_CODE */
}

int VocabTreeLeaf::ScoreQuery(float *q, int bf, DistanceType dtype, 
                              float *scores)
{
#ifdef SKELETON_CODE
    /* *** TODO 8 ***
     * 
     * You'll need to fill in this function.  This function will look
     * at the inverted file at this leaf node (visual word), stored in
     * m_image_list, and will update the similarities in scores based
     * on the value of q.  A few helpful tips:
     * 
     *  - The value of the member variable m_id gives the index into
     *    the histogram q for this visual word.
     *  - There is a simple case in which you can skip any computation
     *    (i.e., when scores will not be updated
     *
     * There are two similarity functions between visual word scores
     * we require you to implement: 
     *
     *  - dtype == DistanceDot: compute the dot product of the query
     *    with the database vectors
     *  - dtype == DistanceMin: compute the sum of the element-wise
     *    min of the query vector with the database vectors.  
     *
     * This function should take less than 25 lines to write,
     * including comments (except this big one) and white space.
     *
     */

    return 0;
#else
    /* Early exit */
    if (q[m_id] == 0.0) return 0;

    int n = (int) m_image_list.size();
    
    for (int i = 0; i < n; i++) {
        int img = m_image_list[i].m_index;

        switch (dtype) {
            case DistanceDot:
                scores[img] += q[m_id] * m_image_list[i].m_count;
                break;
            case DistanceMin:
                scores[img] += MIN(q[m_id], m_image_list[i].m_count);
                break;
        }
    }

    return 0;
#endif /* SKELETON_CODE */
}

double ComputeMagnitude(DistanceType dtype, double dim)
{
    switch (dtype) {
    case DistanceDot:
        return dim * dim;
    case DistanceMin:
        return dim;
    default:
        printf("[ComputeMagnitude] No case value found!\n");
        return 0.0;
    }
}

double VocabTreeInteriorNode::
    ComputeDatabaseVectorMagnitude(int bf, DistanceType dtype) 
{
    double mag = 0.0;

    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            mag += m_children[i]->ComputeDatabaseVectorMagnitude(bf, dtype);
        }
    }

    return mag;
}

double VocabTreeLeaf::
    ComputeDatabaseVectorMagnitude(int bf, DistanceType dtype) 
{
    double dim = m_score;
    double mag = ComputeMagnitude(dtype, dim);

    return mag;
}

int VocabTreeInteriorNode::
    ComputeDatabaseMagnitudes(int bf, DistanceType dtype, 
                              std::vector<float> &mags) 
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->ComputeDatabaseMagnitudes(bf, dtype, mags);
        }
    }

    return 0;
}

int VocabTreeLeaf::
    ComputeDatabaseMagnitudes(int bf, DistanceType dtype, 
                              std::vector<float> &mags) 
{
    int len = (int) m_image_list.size();
    for (int i = 0; i < len; i++) {
        unsigned int index = m_image_list[i].m_index;
        double dim = m_image_list[i].m_count;
        assert(index < mags.size());
        mags[index] += ComputeMagnitude(dtype, dim);
    }

    return 0;
}

int VocabTreeInteriorNode::NormalizeDatabase(int bf, std::vector<float> &mags)
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->NormalizeDatabase(bf, mags);
        }
    }

    return 0;
}

int VocabTreeLeaf::NormalizeDatabase(int bf, std::vector<float> &mags)
{
    int len = (int) m_image_list.size();
    for (int i = 0; i < len; i++) {
        unsigned int index = m_image_list[i].m_index;
        assert(index < mags.size());
        m_image_list[i].m_count /= mags[index];
    }

    return 0;
}


/* Implementations of driver functions */
int VocabTree::PushAndScoreFeature(unsigned char *v, 
                                   unsigned int index, bool add)
{
    qsort_descending();
    m_root->PushAndScoreFeature(v, index, m_branch_factor, m_dim, add);
    
    return 0;
}

int VocabTree::ComputeTFIDFWeights()
{
    if (m_root != NULL) {
        double n = m_root->CountFeatures(m_branch_factor);
        printf("[VocabTree::ComputeTFIDFWeights] Found %lf features\n", n);
        m_root->ComputeTFIDFWeights(m_branch_factor, n);
    }

    return 0;
}

int VocabTree::NormalizeDatabase(int num_db_images)
{
    std::vector<float> mags;
    mags.resize(num_db_images);
    m_root->ComputeDatabaseMagnitudes(m_branch_factor, m_distance_type, mags);

    // for (int i = 0; i < num_db_images; i++) {
    //     printf("[NormalizeDatabase] Vector %d has magnitude %0.3f\n",
    //            i, mags[i]);
    // }

    return m_root->NormalizeDatabase(m_branch_factor, mags);
}


double VocabTree::AddImageToDatabase(int index, int n, unsigned char *v)
{
    m_root->ClearScores(m_branch_factor);
    unsigned long off = 0;

    // printf("[AddImageToDatabase] Adding image with %d features...\n", n);
    // fflush(stdout);

    for (int i = 0; i < n; i++) {
        m_root->PushAndScoreFeature(v + off, index, m_branch_factor, m_dim);

        off += m_dim;
        fflush(stdout);
    }

    double mag = m_root->ComputeDatabaseVectorMagnitude(m_branch_factor,
                                                        m_distance_type);

    m_database_images++;

    switch (m_distance_type) {
    case DistanceDot:
        return sqrt(mag);
    case DistanceMin:
        return mag;
    default:
        printf("[VocabTree::AddImageToDatabase] No case value found!\n");
        return 0.0;
    }
}

int VocabTree::ScoreQueryKeys(int n, bool normalize, unsigned char *v, 
                              float *scores)
{
    qsort_descending();

    /* Compute the query vector */
    m_root->ClearScores(m_branch_factor);
    unsigned long off = 0;
    for (int i = 0; i < n; i++) {
        m_root->PushAndScoreFeature(v + off, 0, 
                                    m_branch_factor, m_dim, false);

        off += m_dim;
    }

    double mag = m_root->ComputeDatabaseVectorMagnitude(m_branch_factor, 
                                                        m_distance_type);

    if (m_distance_type == DistanceDot)
        mag = sqrt(mag);

    /* Now, compute the normalized vector */
    int num_nodes = m_num_nodes;
    float *q = new float[num_nodes];

    if (normalize)
        m_root->FillQueryVector(q, m_branch_factor, 1.0 / mag);
    else
        m_root->FillQueryVector(q, m_branch_factor, 1.0);

    m_root->ScoreQuery(q, m_branch_factor, m_distance_type, scores);

    delete [] q;

    return 0;
}
