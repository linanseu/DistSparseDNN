/*
 * spops.cpp: Sparse Matrix operations implementation
 * Sparse Matrix - Sparse Matrix (SpMM)
 * (c) Mohammad Hasanzadeh Mofrad, 2019
 * (e) m.hasanzadeh.mofrad@gmail.com
 */
 
#ifndef SPOPS_H
#define SPOPS_H

#include "env.hpp"
#include "spmat.hpp"

template<typename Weight>
inline std::tuple<uint64_t, uint32_t, uint32_t> spmm_sym(std::shared_ptr<struct Compressed_Format<Weight>> A,
                                                         std::shared_ptr<struct Compressed_Format<Weight>> B,
                                                         std::vector<Weight> s,
                                                         int32_t tid) {
    double start_time = 0;
    if(!tid) {
        start_time = Env::tic();                                                                    
    }
    
    uint64_t nnzmax = 0;
    uint32_t nrows = 0;
    uint32_t ncols = 0; 
    if((A->compression_type == COMPRESSED_FORMAT::_CSC_) and 
       (B->compression_type == COMPRESSED_FORMAT::_CSC_)) {
        const std::shared_ptr<struct CSC<Weight>> A_CSC = std::static_pointer_cast<struct CSC<Weight>>(A);
        const uint64_t A_nnz   = A_CSC->nnz;
        const uint32_t A_nrows = A_CSC->nrows;
        const uint32_t A_ncols = A_CSC->ncols;
        const uint32_t* A_IA   = A_CSC->IA_blk->ptr;
        const uint32_t* A_JA   = A_CSC->JA_blk->ptr;
        const Weight*    A_A   = A_CSC->A_blk->ptr;
        
        const std::shared_ptr<struct CSC<Weight>> B_CSC = std::static_pointer_cast<struct CSC<Weight>>(B);
        const uint64_t B_nnz   = B_CSC->nnz;
        const uint32_t B_nrows = B_CSC->nrows;
        const uint32_t B_ncols = B_CSC->ncols;
        const uint32_t* B_IA   = B_CSC->IA_blk->ptr;
        const uint32_t* B_JA   = B_CSC->JA_blk->ptr;
        const Weight*    B_A   = B_CSC->A_blk->ptr;

        if(A_ncols != B_nrows) {
            Logging::print(Logging::LOG_LEVEL::ERROR, "SpMM dimensions do not agree A[%d %d] B[%d %d]\n", A_nrows, A_ncols, B_nrows, B_ncols);
            std::exit(Env::finalize()); 
        }
        nrows = A_nrows;
        ncols = B_ncols;
        
        
        //A_CSC->walk(tid);
        //std::exit(0);
        
        uint32_t start_col = Env::start_col[tid];
        uint32_t end_col = Env::end_col[tid];
        uint32_t displacement_nnz = Env::displacement_nnz[tid];
        //uint32_t j
        for(uint32_t j = 0; j < B_ncols; j++) {
            //if(tid == 0)
            //printf("%d/%d: %d %d %d\n", j, B_ncols, B_JA[j], B_JA[j+1], B_JA[j+1] - B_JA[j]);
            //if(!tid)
                //printf("\n%d: ", j);
            for(uint32_t k = B_JA[j]; k < B_JA[j+1]; k++) {
                uint32_t l = B_IA[k];
                //l +=  1 + l/((1028 - Env::nthreads) / Env::nthreads);
                //uint32_t m = (l == start_col) ? displacement_nnz : 0;
                //m = 0;; //42619280
                //if(!tid)
                    //printf("  k=%d Bk=%d l=%d AJl=%d\n", k, B_IA[k], l, A_JA[l]);
                //    printf("%d ", l);
                for(uint32_t n = A_JA[l]; n < A_JA[l+1]; n++) {
                    s[A_IA[n]] = 1;
                }
            }
            for(uint32_t i = 0; i < A_nrows; i++) {
                if(s[i]){
                    nnzmax++;
                    s[i] = 0;
                }
            }
        }
    }
    else {
        Logging::print(Logging::LOG_LEVEL::ERROR, "SpMM not implemented.\n");
        std::exit(Env::finalize()); 
    }
    
    if(!tid) {
        Env::spmm_sym_time += Env::toc(start_time);
    }
    if(!tid) {
        printf("%d %d %d\n", 0, Env::start_col[0], Env::end_col[0]);
        printf("%d %d %d\n", 1, Env::start_col[1], Env::end_col[1]);
        printf("%d %d %d\n", 2, Env::start_col[2], Env::end_col[2]);
    }
    
    //#pragma omp barrier
    //std::exit(0);
    return std::make_tuple(nnzmax, nrows, ncols);
}

template<typename Weight>
inline void spmm(std::shared_ptr<struct Compressed_Format<Weight>> A,
                 std::shared_ptr<struct Compressed_Format<Weight>> B,
                 std::shared_ptr<struct Compressed_Format<Weight>> C,
                 std::vector<Weight> s,
                 std::vector<Weight> b,
                 int32_t tid) {

    double start_time = 0;
    if(!tid) {
        start_time = Env::tic();                                                                    
    }

    if((A->compression_type == COMPRESSED_FORMAT::_CSC_) and 
       (B->compression_type == COMPRESSED_FORMAT::_CSC_) and
       (C->compression_type == COMPRESSED_FORMAT::_CSC_)) {  
       
        const std::shared_ptr<struct CSC<Weight>> A_CSC = std::static_pointer_cast<struct CSC<Weight>>(A);
        const uint64_t A_nnz   = A_CSC->nnz;
        const uint32_t A_nrows = A_CSC->nrows;
        const uint32_t A_ncols = A_CSC->ncols;
        const uint32_t* A_IA   = A_CSC->IA_blk->ptr;
        const uint32_t* A_JA   = A_CSC->JA_blk->ptr;
        const Weight*    A_A   = A_CSC->A_blk->ptr;
        
        const std::shared_ptr<struct CSC<Weight>> B_CSC = std::static_pointer_cast<struct CSC<Weight>>(B);
        const uint64_t B_nnz   = B_CSC->nnz;
        const uint32_t B_nrows = B_CSC->nrows;
        const uint32_t B_ncols = B_CSC->ncols;
        const uint32_t* B_IA   = B_CSC->IA_blk->ptr;
        const uint32_t* B_JA   = B_CSC->JA_blk->ptr;
        const Weight*    B_A   = B_CSC->A_blk->ptr;
        
        const std::shared_ptr<struct CSC<Weight>> C_CSC = std::static_pointer_cast<struct CSC<Weight>>(C);
        const uint64_t C_nnz   = C_CSC->nnz;
        const uint32_t C_nrows = C_CSC->nrows;
        const uint32_t C_ncols = C_CSC->ncols;
        const uint32_t* C_IA   = C_CSC->IA_blk->ptr;
        uint32_t*       C_JA   = C_CSC->JA_blk->ptr;
        const Weight*    C_A   = C_CSC->A_blk->ptr;
        
        uint32_t start_col = Env::start_col[tid];
        uint32_t end_col = Env::end_col[tid];
        uint32_t displacement_nnz = Env::displacement_nnz[tid];
        
        if(A_ncols != B_nrows) {
            Logging::print(Logging::LOG_LEVEL::ERROR, "SpMM dimensions do not agree C[%d %d] != A[%d %d] B[%d %d]\n", C_nrows, C_ncols, A_nrows, A_ncols, B_nrows, B_ncols);
            std::exit(Env::finalize()); 
        }
        
        for(uint32_t j = 0; j < B_ncols; j++) {
            for(uint32_t k = B_JA[j]; k < B_JA[j+1]; k++) {
                uint32_t l = B_IA[k];
                //uint32_t m = (l == start_col) ? displacement_nnz : 0;
                //m  = 0;
                for(uint32_t n = A_JA[l]; n < A_JA[l+1]; n++) {
                    s[A_IA[n]] += (B_A[k] * A_A[n]);
                }
            }
            //j += (tid + 1);
            //if(tid)     
                //C_CSC->populate_spa(s, b, j-1, tid);
            //else
                C_CSC->populate_spa(s, b, j, tid);
        }
        #pragma omp barrier
        C_CSC->adjust(tid);
        C_CSC->walk(tid);
        //C_CSC->statistics(tid);
    }
    else {
        Logging::print(Logging::LOG_LEVEL::ERROR, "SpMM not implemented.\n");
        std::exit(Env::finalize()); 
    }
   
    if(!tid) {
        Env::spmm_time += Env::toc(start_time);
    }
}

template<typename Weight>
inline char validate_prediction(std::shared_ptr<struct Compressed_Format<Weight>> A,
                                      std::vector<uint32_t> trueCategories) {
    const std::shared_ptr<struct CSC<Weight>> A_CSC = std::static_pointer_cast<struct CSC<Weight>>(A);
    const uint64_t A_nnz   = A_CSC->nnz;
    const uint32_t A_nrows = A_CSC->nrows;
    const uint32_t A_ncols = A_CSC->ncols;
    const uint32_t* A_IA   = A_CSC->IA_blk->ptr;
    const uint32_t* A_JA   = A_CSC->JA_blk->ptr;
    const Weight*    A_A   = A_CSC->A_blk->ptr;
    
    
    std::vector<uint32_t> allCategories(A_nrows);
    for(int32_t t = 0; t < Env::nthreads; t++) {
        uint32_t start_col = Env::start_col[t];
        uint32_t end_col = Env::end_col[t];
        uint32_t displacement_nnz = Env::displacement_nnz[t];
        for(uint32_t j = start_col; j < end_col; j++) {
            uint32_t k = (j == start_col) ? displacement_nnz : 0;
            k = 0;
            for(uint32_t i = A_JA[j] + k ; i < A_JA[j+1]; i++) {
                allCategories[A_IA[i]] = 1;
            }
        }
    }
    
    char me = 1;
    uint32_t j = 0;
    for(uint32_t i = 0; i < A_nrows; i++) {
        if(trueCategories[i] != allCategories[i]) {
            me = 0;
            break;
        }
    }
    char all = 0;
    MPI_Allreduce(&me, &all, 1, MPI_CHAR, MPI_SUM, MPI_COMM_WORLD);

    return((all == Env::nranks));
}
#endif