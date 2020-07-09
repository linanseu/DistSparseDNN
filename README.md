# DistSparseDNN (Distributed Sparse Deep Neural Network Inference)
### Install
    make
### Uninstall
    make clean
### Run
    bin/./radixnet -m <#input instances #input features>
                   -n <#neurons>
                   -l <#layers>
                   -c <#categories>
                   <input path[text|binary]>
                   <DNN   path[text|binary]>
                   -p <parallelism type>

### Examples
    Run MNIST on Radixnet Sparse DNN (http://graphchallenge.mit.edu/data-sets)

    mpirun -np 4 bin/./radixnet -m 60000 1024 -n 1024 -l 120 -c 0 data/radixnet/bin/MNIST data/radixnet/bin/DNN -p 0

    Other ported datasets 

    Generate the sparse DNN using scripts/sparse_dnn_generator.py

    MNIST, fashion MNIST, CIFAR-10, CIFAR-100, IMDB
   
