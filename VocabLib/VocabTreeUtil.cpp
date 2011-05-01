/* VocabTreeUtil.cpp */
/* Utility functions for vocabulary tree */

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "VocabTree.h"

unsigned long VocabTreeInteriorNode::CountNodes(int bf) const
{
    unsigned long num_nodes = 0;
    
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL)
            num_nodes += m_children[i]->CountNodes(bf);
    }
    
    return num_nodes + 1;
}

unsigned long VocabTreeInteriorNode::CountLeaves(int bf) const
{
    unsigned long num_leaves = 0;
    
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL)
            num_leaves += m_children[i]->CountLeaves(bf);
    }
    
    return num_leaves;
}

double VocabTreeInteriorNode::CountFeatures(int bf)
{
    double num_features = 0.0;

    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            num_features += m_children[i]->CountFeatures(bf);
        }
    }

    return num_features;
}

int VocabTreeInteriorNode::ClearScores(int bf)
{    
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->ClearScores(bf);   
        }   
    }

    return 0;   
}

int VocabTreeInteriorNode::ClearDatabase(int bf)
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->ClearDatabase(bf);
        }
    }

    return 0;    
}

int VocabTreeInteriorNode::SetConstantLeafWeights(int bf) 
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL)
            m_children[i]->SetConstantLeafWeights(bf);
    }

    return 0;    
}

unsigned long VocabTreeLeaf::CountNodes(int bf) const
{
    return 1;
}

unsigned long VocabTreeLeaf::CountLeaves(int bf) const
{
    return 1;
}

double VocabTreeLeaf::CountFeatures(int bf)
{
    double num_features = 0;

    int len = (int) m_image_list.size();
    for (int i = 0; i < len; i++) {
        num_features += m_image_list[i].m_count;
    }

    return num_features;
}

int VocabTreeLeaf::ClearScores(int bf)
{
    m_score = 0.0;
    return 0;
}

int VocabTreeLeaf::ClearDatabase(int bf)
{
    m_image_list.clear();
    return 0;    
}

int VocabTreeLeaf::SetInteriorNodeWeight(int bf, float weight)
{
    return 0;
}

int VocabTreeLeaf::SetInteriorNodeWeight(int bf, int dist_from_leaves,
                                                 float weight)
{
    return 1;
}

int VocabTreeLeaf::SetConstantLeafWeights(int bf) 
{
    m_weight = 1.0;
    return 0;
}

int VocabTreeInteriorNode::PrintWeights(int depth_curr, int bf) const
{
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            m_children[i]->PrintWeights(depth_curr + 1, bf);
        }
    }

    return 0;
}

unsigned long VocabTreeInteriorNode::ComputeIDs(int bf, unsigned long id)
{
    m_id = id;

    unsigned long next_id = id + 1;
    for (int i = 0; i < bf; i++) {
        if (m_children[i] != NULL) {
            next_id = m_children[i]->ComputeIDs(bf, next_id);
        }
    }

    return next_id;
}

unsigned long VocabTreeLeaf::ComputeIDs(int bf, unsigned long id)
{
    m_id = id;
    return id + 1;
}

int VocabTreeLeaf::PrintWeights(int depth_curr, int bf) const
{
    for (int i = 0; i < depth_curr; i++) {
        printf(" ");
    }
    
    printf("%0.3f\n", m_weight);

    return 0;
}

unsigned long VocabTree::CountNodes() const
{
    if (m_root == NULL) {
        return 0;
    } else {
        return m_root->CountNodes(m_branch_factor);
    }
}

unsigned long VocabTree::CountLeaves() const
{
    if (m_root == NULL) {
        return 0;
    } else {
        return m_root->CountLeaves(m_branch_factor);
    }
}

int VocabTree::PrintWeights() 
{
    if (m_root != NULL) {
        m_root->PrintWeights(0, m_branch_factor);
    }
    
    return 0;
}

int VocabTree::ClearDatabase()
{
    if (m_root != NULL) {
        m_root->ClearDatabase(m_branch_factor);
    }
    
    return 0;
}

int VocabTree::SetInteriorNodeWeight(float weight)
{
    if (m_root != NULL) {
        m_root->SetInteriorNodeWeight(m_branch_factor, weight);
    }
    
    return 0;
}

int VocabTree::SetInteriorNodeWeight(int dist_from_leaves, float weight)
{
    if (m_root != NULL) {
        m_root->SetInteriorNodeWeight(m_branch_factor, 
                                      dist_from_leaves,weight);
    }
    
    return 0;
}

int VocabTree::SetConstantLeafWeights() 
{
    if (m_root != NULL) {
        m_root->SetConstantLeafWeights(m_branch_factor);
    }

    return 0;
}

int VocabTree::SetDistanceType(DistanceType type)
{
    m_distance_type = type;
    return 0;
}
