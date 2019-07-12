# SiSe (Similarity Search)

SiSe is bytewise approximate matching algorithm suitable for files of dissimilar sizes based on ssdeep (https://ssdeep-project.github.io/ssdeep/usage.html) that has been implemented as a C++ multi-threaded command-line application that uses features of the C++17 standard and the OpenMP library. 

SiSe implements a dual input command format: One adapted to the FRASH tests, and a proprietary one. Regarding the input format compatible with FRASH, SiSe can be used without modifiers with one or several files separated by spaces (e.g. sise file1 file2 file3). This indicates SiSe that it must treat those files as content data and generate their signatures individually. If modifier "-x" is used along with the name of a file, the command indicates the application that the file contains multiple signatures and that it must compare all signatures against the rest. Finally, if modifier "-r" is employed together with the path of a directory, SiSe will compute the signature of all the files contained in that folder.

In comparison, the proprietary input command format used by SiSe for creating a new signature is "sise -i file", which allows the following additional optional elements:

-i (mandatory): Input file name.

-o (optional): Output file name.

-b (optional): Block size (if not indicated, the block size is computed by the application as in ssdeep).

-d: Decremental value (i.e., the amount to be decremented to the block size in the trigger point computations. The default value is 1).

-t: Number of threads used by the application when computing the signatures (if not indicated, the value employed is decided by the application based on the size of the input file).

The command for comparing two files is "sise -c file1 file2". Those files can be both of them content files, hash files or a mixture of them (one content file and one hash file, the order is not important).

SiSe can be compiled on Linux and Windows platforms, though it has only been extensively tested on Linux (Ubuntu). As SiSe is implemented as a single C++ file and uses standard libraries, it can be compiled with the command "g++-8 -Ofast -std=c++17 sise.cpp -fopenmp -lstdc++fs -o sise".

* SiSe has been developed at CSIC-ITEFI (Spanish National Research Council) *
