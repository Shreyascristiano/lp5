// Code-3 (Min, Max, Sum and Average Operations)

/*
 * THIS CODE HAS BEEN TESTED AND IS FULLY OPERATIONAL.
 *
 * Problem Statement: Implement Min, Max, Sum and Average operations using Parallel Reduction.
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
 *   g++ -fopenmp Code-3.cpp -o Code-3
 *
 * iii) Execute:
 *   ./Code-3
 **/

// BEGINNING OF CODE
#include <iostream>
#include <vector>
#include <omp.h>
#include <cstdlib>

using namespace std;

int main() {
    // Uncomment to manually control thread count
    // omp_set_num_threads(4);

    // --- Input ---
    int n = 1000000;
    vector<int> nums(n);

    for (int i = 0; i < n; i++)
        nums[i] = rand() % 10000;

    cout << "Input: " << n << " random integers in the range [0, 9999]." << endl << endl;

    // long long prevents overflow: up to 1,000,000 * 9,999 ≈ 10 billion,
    // which exceeds the int limit of ~2.1 billion.
    long long sum_seq, sum_par;
    int min_seq, max_seq;
    int min_par, max_par;
    double avg_seq, avg_par;
    double start, end;

    // --- Sequential ---
    min_seq = max_seq = nums[0];
    sum_seq = 0;

    start = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        if (nums[i] < min_seq) min_seq = nums[i];
        if (nums[i] > max_seq) max_seq = nums[i];
        sum_seq += nums[i];
    }
    end = omp_get_wtime();

    // Computed after timing so both versions are measured fairly.
    avg_seq = (double)sum_seq / n;
    double time_seq = end - start;

    // --- Parallel ---
    min_par = max_par = nums[0];
    sum_par = 0;

    start = omp_get_wtime();
    // reduction(min/max/+) gives each thread its own private copy of the
    // variable, then combines them at the end, no critical sections needed.
    // Without reduction, threads would race to update the same variable.
    #pragma omp parallel for reduction(min: min_par) reduction(max: max_par) reduction(+: sum_par)
    for (int i = 0; i < n; i++) {
        if (nums[i] < min_par) min_par = nums[i];
        if (nums[i] > max_par) max_par = nums[i];
        sum_par += nums[i];
    }
    end = omp_get_wtime();

    avg_par = (double)sum_par / n;
    double time_par = end - start;

    // --- Output ---
    cout << "--- Sequential Computation ---" << endl;
    cout << "Minimum  : " << min_seq << endl;
    cout << "Maximum  : " << max_seq << endl;
    cout << "Sum      : " << sum_seq << endl;
    cout << "Average  : " << avg_seq << endl;
    cout << "Time     : " << time_seq << " seconds" << endl;

    cout << "\n--- Parallel Computation ---" << endl;
    cout << "Minimum  : " << min_par << endl;
    cout << "Maximum  : " << max_par << endl;
    cout << "Sum      : " << sum_par << endl;
    cout << "Average  : " << avg_par << endl;
    cout << "Time     : " << time_par << " seconds" << endl;

    cout << "\nSpeedup (Sequential / Parallel) = " << (time_seq / time_par) << "x" << endl;

    return 0;
}
// END OF CODE

/*
EXAMPLE OUTPUT:

$ ./Code-3 
Input: 1000000 random integers in the range [0, 9999].

--- Sequential Computation ---
Minimum  : 0
Maximum  : 9999
Sum      : 5000491283
Average  : 5000.49
Time     : 0.0205385 seconds

--- Parallel Computation ---
Minimum  : 0
Maximum  : 9999
Sum      : 5000491283
Average  : 5000.49
Time     : 0.0135714 seconds

Speedup (Sequential / Parallel) = 1.51336x
*/






// #include <iostream>
// #include <vector>
// #include <omp.h>
// #include <algorithm>
// #include <climits>

// using namespace std;

// void parallelReduction(const vector<int>& arr) {
//     int n = arr.size();
    
//     int min_val = INT_MAX;
//     int max_val = INT_MIN;
//     long long sum = 0;

//     // OpenMP handles the splitting, local computation, and merging automatically
//     #pragma omp parallel for reduction(min:min_val) reduction(max:max_val) reduction(+:sum)
//     for (int i = 0; i < n; i++) {
//         if (arr[i] < min_val) min_val = arr[i];
//         if (arr[i] > max_val) max_val = arr[i];
//         sum += arr[i];
//     }

//     double average = static_cast<double>(sum) / n;

//     cout << "--- Parallel Reduction Results ---" << endl;
//     cout << "Minimum: " << min_val << endl;
//     cout << "Maximum: " << max_val << endl;
//     cout << "Sum:     " << sum << endl;
//     cout << "Average: " << average << endl;
// }

// int main() {
//     int N = 1000000; // 1 Million elements
//     vector<int> data(N);
    
//     for (int i = 0; i < N; i++) {
//         data[i] = i + 1; // Array of 1 to 1,000,000
//     }

//     double start = omp_get_wtime();
//     parallelReduction(data);
//     double end = omp_get_wtime();

//     cout << "Reduction Time: " << (end - start) << " seconds" << endl;

//     return 0;
// }

// // --- PARALLEL REDUCTION COMPLEXITY ---
// // Theoretical Span: O(log n), representing the height of the reduction tree.
// // Practically, following the Work-Span model, the complexity is O( n / P + log n ).
// // Note: According to Amdahl's Law, the sequential fraction (s) consists of the 
// // final steps of combining local thread results into the global total. 
// // Because log(n) is very small, reduction typically shows the highest parallel 
// // efficiency and speedup of all parallel patterns.
// // (P = processors, n = number of elements)