// Code-2 (Parallel Bubble Sort and Merge Sort)

/*
 * THIS CODE HAS BEEN TESTED AND IS FULLY OPERATIONAL.
 *
 * Problem Statement:
 *  Write a program to implement Parallel Bubble Sort and Merge sort using OpenMP.
 *  Use existing algorithms and measure the performance of sequential and parallel algorithms.
 *
 * Code from HighPerformanceComputing (SPPU - Final Year - Computer Engineering - Content)
 * repository on KSKA Git: https://git.kska.io/sppu-be-comp-content/HighPerformanceComputing
 **/

/*
 * EXECUTION INSTRUCTIONS (Debian-based distributions):
 *
 * i) Install g++ with OpenMP support:
 *   sudo apt update
 *   sudo apt install g++
 *
 * ii) Compile:
 *   g++ -fopenmp Code-2.cpp -o Code-2
 *
 * iii) Execute:
 *   ./Code-2
 **/

// BEGINNING OF CODE
#include <iostream>
#include <vector>
#include <cstdlib>
#include <omp.h>

using namespace std;

void printArray(const vector<int>& arr) {
    for (int num : arr)
        cout << num << " ";
    cout << endl;
}

// Bubble Sort

// Sequential bubble sort.
// Sorts the array using bubble sort by repeatedly swapping adjacent elements.
void sequentialBubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}

// Parallel bubble sort using odd-even transposition.
// Standard bubble sort cannot be parallelized directly: thread on index j
// and thread on index j+1 would both touch arr[j+1] simultaneously (data race).
// Odd-even transposition alternates between two phases each pass:
//   Phase 0 (even): compare pairs (0,1), (2,3), (4,5), ...
//   Phase 1 (odd):  compare pairs (1,2), (3,4), (5,6), ...
// Within each phase every pair is independent, so threads never share elements.
void parallelBubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n; i++) {
        // i % 2 selects even phase (0) or odd phase (1).
        // The starting index of the first pair in each phase matches i % 2.
        #pragma omp parallel for
        for (int j = i % 2; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}

// Merge Sort

// Merges two sorted halves arr[left..mid] and arr[mid+1..right] in place.
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void sequentialMergeSort(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sequentialMergeSort(arr, left, mid);
    sequentialMergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// Parallel merge sort using OpenMP tasks.
// "#pragma omp parallel sections" inside a recursive function would spawn a
// new thread team at every level of recursion, hundreds of thousands of teams
// for a large array, causing enormous overhead and likely a crash.
// Tasks are lighter: the runtime schedules them across an existing thread pool.
// The depth cutoff switches to sequential below a threshold to avoid spawning
// tasks so small that the overhead exceeds the work itself.
void parallelMergeSortHelper(vector<int>& arr, int left, int right, int depth) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;

    if (depth <= 0) {
        // Below the cutoff the subarray is small enough that sequential is faster.
        sequentialMergeSort(arr, left, mid);
        sequentialMergeSort(arr, mid + 1, right);
    } else {
        #pragma omp task
        parallelMergeSortHelper(arr, left, mid, depth - 1);

        #pragma omp task
        parallelMergeSortHelper(arr, mid + 1, right, depth - 1);

        // Wait for both tasks to finish before merging.
        #pragma omp taskwait
    }

    merge(arr, left, mid, right);
}

void parallelMergeSort(vector<int>& arr, int left, int right) {
    // The single directive creates one thread team for the entire sort.
    // All recursive tasks share this pool instead of creating new teams.
    #pragma omp parallel
    {
        // single ensures only one thread kicks off the root task;
        // the rest wait and pick up the child tasks as they are created.
        #pragma omp single
        parallelMergeSortHelper(arr, left, right, 4); // depth 4 → up to 16 parallel tasks
    }
}

// Main function

int main() {
    int n = 10000; // Adjust this to specify the number of elements.
    vector<int> arr(n);

    for (int i = 0; i < n; i++)
        arr[i] = rand() % 10000;

    double start, end;
    double time_seq_bubble, time_par_bubble;
    double time_seq_merge, time_par_merge;

    // --- Sequential Bubble Sort ---
    vector<int> seqArr = arr;
    start = omp_get_wtime();
    sequentialBubbleSort(seqArr);
    end = omp_get_wtime();
    time_seq_bubble = end - start;
    cout << "Sequential Bubble Sort time: " << time_seq_bubble << " seconds" << endl;

    // --- Parallel Bubble Sort ---
    vector<int> parArr = arr;
    start = omp_get_wtime();
    parallelBubbleSort(parArr);
    end = omp_get_wtime();
    time_par_bubble = end - start;
    cout << "Parallel Bubble Sort time: " << time_par_bubble << " seconds" << endl;

    cout << "Bubble Sort Speedup (Sequential / Parallel) = " << (time_seq_bubble / time_par_bubble) << "x" << endl;

    // --- Sequential Merge Sort ---
    seqArr = arr;
    start = omp_get_wtime();
    sequentialMergeSort(seqArr, 0, n - 1);
    end = omp_get_wtime();
    time_seq_merge = end - start;
    cout << "\nSequential Merge Sort time: " << time_seq_merge << " seconds" << endl;

    // --- Parallel Merge Sort ---
    parArr = arr;
    start = omp_get_wtime();
    parallelMergeSort(parArr, 0, n - 1);
    end = omp_get_wtime();
    time_par_merge = end - start;
    cout << "Parallel Merge Sort time: " << time_par_merge << " seconds" << endl;

    cout << "Merge Sort Speedup (Sequential / Parallel) = " << (time_seq_merge / time_par_merge) << "x" << endl;

    return 0;
}
// END OF CODE

/*
EXAMPLE OUTPUT (when n=10000):

$ ./Code-2 
Sequential Bubble Sort time: 0.955394 seconds
Parallel Bubble Sort time: 0.282093 seconds
Bubble Sort Speedup (Sequential / Parallel) = 3.38681x

Sequential Merge Sort time: 0.0116294 seconds
Parallel Merge Sort time: 0.00282529 seconds
Merge Sort Speedup (Sequential / Parallel) = 4.11618x
*/







// /*
// Write a program to implement Parallel Bubble Sort and Merge sort using OpenMP. Use
// existing algorithms and measure the performance of sequential and parallel algorithms.
// */

// #include <iostream>
// #include <vector>
// #include <omp.h>
// #include <algorithm>
// #include <ctime>

// using namespace std;

// // --- Bubble Sort ---
// void sequentialBubbleSort(vector<int> &arr) {
//     int n = arr.size();

//     for (int i=0 ; i<n ; i++) {
//         for (int j=0 ; j<n-i-1 ; j++) {
//             if (arr[j] > arr[j+1]) swap(arr[j], arr[j + 1]);
//         }
//     }

// }


// void parBubbleSort(vector<int>& arr) {
//     int n = arr.size();
//     for (int i = 0; i < n; i++) {
//         // Odd phase
//         #pragma omp parallel for
//         for (int j = 1; j < n; j += 2) {
//             if (arr[j - 1] > arr[j]) swap(arr[j - 1], arr[j]);
//         }
//         // Even phase
//         #pragma omp parallel for
//         for (int j = 2; j < n; j += 2) {
//             if (arr[j - 1] > arr[j]) swap(arr[j - 1], arr[j]);
//         }
//     }
// }


// void parallelBubbleSort(vector<int> &arr) {
//     int n = arr.size();

//     for (int i=0 ; i<n ; i++) {

//         #pragma omp parallel for
//         for (int j=1 ; j<n-i-1 ; j+=2) {
//             if (arr[j] < arr[j-1]) swap(arr[j], arr[j-1]);
//         }

//         #pragma omp parallel for
//         for (int j=2 ; j<n-i-1 ; j+=2) {
//             if (arr[j] < arr[j-1]) swap(arr[j], arr[j-1]);
//         }
//     }

// }


// // --- Merge Sort ---
// void merge(vector<int> &arr, int l, int m, int h) {
//     int n1 = m - l + 1, n2 = h - m;
//     vector<int> L(n1), R(n2);

//     for (int x=0 ; x<n1 ; x++) L[x] = arr[l+x];
//     for (int x=0 ; x<n2 ; x++) R[x] = arr[m+1+x];

//     int i=0, j=0, k=l;
//     while (i<n1 && j<n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++]; 
//     while (i<n1) arr[k++] = L[i++];
//     while (j<n2) arr[k++] = R[j++];
// }


// void seqMergeSort(vector<int>& arr, int l, int r) {
//     if (l < r) {
//         int m = l + (r - l) / 2;
//         seqMergeSort(arr, l, m);
//         seqMergeSort(arr, m + 1, r);
//         merge(arr, l, m, r);
//     }
// }


// void parallelMergeSort(vector<int> &arr, int l, int h) {
//     if (l < h) {
//         // int m = l + (r - l) / 2;
//         // // Limit recursion depth for efficiency
//         // if (r - l < 1000) {
//         //     seqMergeSort(arr, l, r);
//         // } else {
//         int m = l + (h-l)/2;
//         #pragma omp task
//         parallelMergeSort(arr, l, m);
//         #pragma omp task
//         parallelMergeSort(arr, m+1, h);
//         #pragma omp taskwait
//         merge(arr, l, m, h);
//     }
// }


// int main() {
//     int N = 10000; 
//     vector<int> data(N);
//     generate(data.begin(), data.end(), rand);

//     cout << "--- Performance for N = " << N << " ---" << endl;

//     // Bubble Sort Performance
//     vector<int> b1 = data, b2 = data;
//     double s = omp_get_wtime();
//     sequentialBubbleSort(b1);
//     cout << "Sequential Bubble Sort: " << omp_get_wtime() - s << "s" << endl;

//     s = omp_get_wtime();
//     parallelBubbleSort(b2);
//     cout << "Parallel Bubble Sort: " << omp_get_wtime() - s << "s" << endl;

//     // Merge Sort Performance
//     vector<int> m1 = data, m2 = data;
//     s = omp_get_wtime();
//     seqMergeSort(m1, 0, N - 1);
//     cout << "Sequential Merge Sort: " << omp_get_wtime() - s << "s" << endl;

//     s = omp_get_wtime();
//     #pragma omp parallel
//     {
//         #pragma omp single
//         {
//             parallelMergeSort(m2, 0, N - 1);
//         }
//     }
    
//     cout << "Parallel Merge Sort: " << omp_get_wtime() - s << "s" << endl;


//     // --- PARALLEL BUBBLE SORT (ODD-EVEN) COMPLEXITY ---
//     // Theoretical Span: O(n), representing the n alternating phases.
//     // Practically, following the Work-Span model (Brent's Theorem), 
//     // the complexity is O( (n^2) / P + n ).
//     // Note: By Amdahl's Law, the synchronization barriers between phases are 
//     // the bottleneck. Parallelization reduces the O(n^2) work but the O(n) 
//     // dependency chain of swaps remains.
//     // (P = processors, n = number of elements)


//     // --- PARALLEL MERGE SORT COMPLEXITY ---
//     // Theoretical Span: O(log^2 n) for standard parallel merge, or O(log n) 
//     // with a parallel merge step.
//     // Practically, following the Work-Span model, the complexity is O( (n log n) / P + log n ).
//     // Note: By Amdahl's Law, the final merge step and task creation overhead 
//     // are the sequential bottlenecks. Performance scales well on large datasets 
//     // until the task granularity becomes smaller than the thread management cost.
//     // (P = processors, n = number of elements)


//     return 0;
// }