VocabTree2, by Noah Snavely (snavely@cs.cornell.edu)
----------------------------------------------------

To build the VocabTree2 library, just do "make" from the top-level
directory.

This code is distributed under a FreeBSD license.  See the source code
for details.

Unfortunately, I can't assist much with problems with the code.

If you find this code useful for your project, please cite the
following paper:

  Sameer Agarwal, Noah Snavely, Ian Simon, Steven M. Seitz and Richard
  Szeliski.  "Building Rome in a Day."  In Proc. ICCV 2009.

How to run
----------

This code works with SIFT keys in the format produced by David Lowe's
sift binary.  There are three tools provided:

 - VocabLearn: build a vocabulary tree from a set of key files using k-means
 - VocabBuildDB: build an image database from a set of key files and a tree
 - VocabMatch: match an image to a database and retrieve a set of top matches

Example usages:

  # VocabLearn  
  # Usage: VocabLearn list.in depth branching_factor restarts tree.out   
  #  - list.in contains a list of key files, one per line, with each key   
  #      file in Lowe's format.  
  #  - depth -- depth of tree. 0 indicates a flat tree.  
  #  - branching_factor -- number of children each non-leaf node.  
  #  - restarts -- number of trials in each run of k-means.  
  #  - tree.out -- name of output tree.  
  #   
  # Example:   
  # Learn a flat vocabulary tree with 500K visual words using the SIFT keys in list.txt   
  # Store the results in tree.500K.out   
  > ./VocabLearn/VocabLearn list.txt 0 500000 1 tree.500K.out   
  
  # VocabBuildDB  
  # Usage: VocabBuildDB list.in tree.in db.out [use_tfidf:1] [normalize:1] [start_id:0] [distance_type:1]  
  #  
  # Example:  
  > ./VocabBuildDB/VocabBuildDB list.txt tree.500K.out vocab.db  
  
  # VocabMatch  
  # Usage: VocabMatch db.in list.in query.in num_nbrs matches.out [distance_type:1] [normalize:1]   
  #   
  # Example:  
  > ./VocabMatch/VocabMatch vocab.db list.txt query.txt 2 matches.txt  
   
  # The query file is in the same format as the list file, having one SIFT   
  # key file per line, corresponding to the images to query for matching   
  # to the vocabulary database.  
  #  
  # The value num_nbrs is the number of top matches to retrieve for each  
  # query image.  

  # The output matches file has three columns:  
  #  - the index of the query image to match  
  #  - the index of a well-matching image from the database  
  #  - the matching score between these images.   
  #  
  # For example, if there are 10 images in the database, 3 query images,   
  # and num_nbrs is 2, this file may look as follows:  
  #    
  # 0 9  0.8004  
  # 0 7  0.3138  
  # 1 3  0.8017  
  # 1 8  0.3250  
  # 2 10 0.7933  
  # 2 6  0.3145  
