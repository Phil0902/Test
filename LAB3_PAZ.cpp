#include <iostream>
#include <vector>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

using namespace std;

const long MAX = 500000000;  
const int RANGE = 5;         
const int NUM_THREADS = 8;   

vector<int> num(MAX);        
atomic<unsigned long long int> sum(0); 
atomic<int> h1(0), h2(0), h3(0), h4(0), h5(0);  
atomic<unsigned long long int> histogramsum(0); 
mutex print_mtx;  


const long original_runtime_ms = 10243; 

void rand_num(int start, int end) {
    for (int i = start; i < end; i++) {
        num[i] = rand() % RANGE + 1;
    }
}// fdfsdfsdf

void total(int start, int end) {
    unsigned long long int local_sum = 0;
    for (int i = start; i < end; i++) {
        local_sum += num[i];
    }
    sum.fetch_add(local_sum, std::memory_order_relaxed);
}

void hist(int start, int end) {
    int local_h1 = 0, local_h2 = 0, local_h3 = 0, local_h4 = 0, local_h5 = 0;
    for (int i = start; i < end; i++) {
        switch (num[i]) {
            case 1: local_h1++; break;
            case 2: local_h2++; break;
            case 3: local_h3++; break;
            case 4: local_h4++; break;
            case 5: local_h5++; break;
        }
    }
    h1.fetch_add(local_h1, std::memory_order_relaxed);
    h2.fetch_add(local_h2, std::memory_order_relaxed);
    h3.fetch_add(local_h3, std::memory_order_relaxed);
    h4.fetch_add(local_h4, std::memory_order_relaxed);
    h5.fetch_add(local_h5, std::memory_order_relaxed);
    histogramsum.store(h1.load() + (h2.load() * 2) + (h3.load() * 3) + (h4.load() * 4) + (h5.load() * 5), std::memory_order_relaxed);
}

int main() {
    auto start = chrono::high_resolution_clock::now();  

    vector<thread> threads;
    int chunk_size = MAX / NUM_THREADS;


    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(thread(rand_num, i * chunk_size, (i + 1) * chunk_size));
    }
    for (auto& t : threads) t.join(); 

    threads.clear();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(thread(total, i * chunk_size, (i + 1) * chunk_size));
        threads.push_back(thread(hist, i * chunk_size, (i + 1) * chunk_size));
    }
    for (auto& t : threads) t.join(); 

    auto end = chrono::high_resolution_clock::now();  
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    long threaded_runtime_ms = duration.count();  


    double enhancement_percentage = ((double)(original_runtime_ms - threaded_runtime_ms) / original_runtime_ms) * 100;

  
    {
        lock_guard<mutex> guard(print_mtx);
        cout << "Histogram Result is : " << endl;
        cout << "1  ----> " << h1.load() << " SUM : " << h1.load() << endl;
        cout << "2  ----> " << h2.load() << " SUM : " << h2.load() * 2 << endl;
        cout << "3  ----> " << h3.load() << " SUM : " << h3.load() * 3 << endl;
        cout << "4  ----> " << h4.load() << " SUM : " << h4.load() * 4 << endl;
        cout << "5  ----> " << h5.load() << " SUM : " << h5.load() * 5 << endl;
        cout << "The sum of the vector is : " << sum.load() << " and the sum of the histogram is " << histogramsum.load() << endl;
        cout << "TOTAL RUNNING TIME IS : " << threaded_runtime_ms << " milliseconds...." << endl;
        cout << "Performance Enhancement: " << enhancement_percentage << "%" << endl;
    }

    return 0;
}


