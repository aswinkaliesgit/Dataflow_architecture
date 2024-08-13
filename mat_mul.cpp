#include "device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include<thread>
#include<mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
using namespace std;

#define MATRIX_ROWS_1 500
#define MATRIX_COLUMNS_1 500
#define MATRIX_ROWS_2 500
#define MATRIX_COLUMNS_2 500
#define ONE_BYTE 8

std::mutex m_;
int pe_index=0;
// Helper function to verify the results
class ThreadPool {
public:
    // Constructor to create a thread pool with given
    // number of threads
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        // Creating worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    function<void()> task;
                    // The reason for putting the below code
                    // here is to unlock the queue before
                    // executing the task so that other
                    // threads can perform enqueue tasks
                    {
                        // Locking the queue so that data
                        // can be shared safely
                        unique_lock<mutex> lock(queue_mutex_);

                        // Waiting until there is a task to
                        // execute or the pool is stopped
                        cv_.wait(lock, [this] {
                            return !tasks_.empty() || stop_;
                        });

                        // Exit the thread in case the pool
                        // is stopped and there are no tasks
                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        // Get the next task from the queue
                        task = move(tasks_.front());
                        tasks_.pop();
                        lock.unlock();
                    }

                    task();
                    
                }
            });
        }
    }

    // Destructor to stop the thread pool
    ~ThreadPool() {
        {
            // Lock the queue to update the stop flag safely
            unique_lock<mutex> lock(queue_mutex_);
            stop_ = true;
        }

        // Notify all threads
        cv_.notify_all();

        // Joining all worker threads to ensure they have
        // completed their tasks
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool
    void enqueue(function<void()> task) {
        {
            unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(move(task));
            lock.unlock();
        }
        cv_.notify_one();
    }

private:
    // Vector to store worker threads
    vector<thread> threads_;

    // Queue of tasks
    queue<function<void()> > tasks_;

    // Mutex to synchronize access to shared data
    mutex queue_mutex_;

    // Condition variable to signal changes in the state of
    // the tasks queue
    condition_variable cv_;

    // Flag to indicate whether the thread pool should stop
    // or not
    bool stop_ = false;
};

void verify_results(float *c, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {   
        if(d[i]!=c[i]){
            std::cout<<"\n"<<c[i]<<" "<<d[i]<<"\n";
        }
        assert(d[i] == c[i]);
    }
}

// Helper function to initialize input data
void initialize_input_data(float *a, float *b, float *c, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_1; ++i)
    {
        a[i] = static_cast<float>(i + 1);
    }

    for (int i = 0; i < MATRIX_ROWS_2 * MATRIX_COLUMNS_2; ++i)
    {
        b[i] = static_cast<float>(i + 1);
    }

    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {
        c[i] = 0.0f;
    }

    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {
        d[i] = 0.0f;
    }
}

// Helper function to free allocated resources
void free_resources(void *pe_arrays[], int resource_required, float *a, float *b, float *c, float *d)
{
    for (int i = 0; i < resource_required; i++)
    {
        std::free(pe_arrays[i]);
    }

    std::free(a);
    std::free(b);
    std::free(c);
    std::free(d);
}

// Helper function to print resource utilization
void print_resource_utilization(int resource_required, float *c, float *d)
{
    std::cout << "*******DEVICE SPECS*******\n";
    std::cout << "TOTAL PE ROWS->" << PE_ROWS << "\n";
    std::cout << "TOTAL PE COLUMNS->" << PE_COLUMNS << "\n";
    std::cout << "SIZE PER PE->" << SIZE_PER_PE << " BYTES" << "\n";
    std::cout << "Total PE's required for calculation->" << (resource_required) << " PE's\n\n";

    verify_results(c, d);

    std::cout << "\n";
    std::cout << "TEST PASSED\n";
    std::cout << "RESULT VERIFIED SUCCESSFULLY\n";
}

// Helper function to calculate the resource required for computation
int calculate_resource() { return MATRIX_ROWS_1 * MATRIX_COLUMNS_2; }

// Helper function to do computation without dataflow
void compute_results_without_dataflow(float *a, float *b, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1; i++)
    {
        for (int j = 0; j < MATRIX_COLUMNS_2; j++)
        {
            for (int k = 0; k < MATRIX_COLUMNS_1; k++)
            {
                d[i * MATRIX_COLUMNS_2 + j] +=
                    a[i * MATRIX_COLUMNS_1 + k] * b[k * MATRIX_COLUMNS_2 + j];
            }
        }
    }
}

// Helper function to load input data into processing elements
void load_data_to_pe_arrays(float *a, float *b, void *pe_arrays[],
                            int resource_required)
{
    pe_index = 0;
    // The data is loaded in a way that a row from matrix_a and a column from
    // matrix_b will be in a single PE
    for (int i = 0; i < MATRIX_ROWS_1; i++)
    {
        for (int j = 0; j < MATRIX_COLUMNS_2; j++)
        {
            if (pe_index >= resource_required)
                break;

            float *curr_arr = static_cast<float *>(pe_arrays[pe_index]);

            for (int l = 0; l < MATRIX_COLUMNS_1; l++)
            {
                curr_arr[l] = a[i * MATRIX_COLUMNS_1 + l];
            }

            for (int l = 0; l < MATRIX_ROWS_2; l++)
            {
                curr_arr[MATRIX_COLUMNS_1 + l] = b[l * MATRIX_COLUMNS_2 + j];
            }

            pe_index++;
        }
    }
}
// Helper function to load data from processing elements
void load_data_from_pe_arrays(float *c, void *pe_arrays[], int resource_required, int index)
{
    int t = 0;
    // The data is loaded in a way that a row from matrix_a and a column from
    // matrix_b will be in a single PE
    for (int i = resource_required; i <= resource_required + index; i++)
    {
        float *curr_arr = static_cast<float *>(pe_arrays[i]);
        for (int j = 0; j < (SIZE_PER_PE / (sizeof(float))); j++)
        {
            if ((t * (SIZE_PER_PE / sizeof(float)) + j) < (MATRIX_ROWS_1 * MATRIX_COLUMNS_2))
            {
                c[t * (SIZE_PER_PE / sizeof(float)) + j] = curr_arr[j];
            }
        }
        t++;
    }
}

// Helper function to perform multiplication with threads
void thread_multiply(void *pe_arrays[], int no_of_pe_to_store_output, int i, int resource_required){
    float *curr_arr = static_cast<float *>(pe_arrays[i]);
    
    float temp = 0;
    for (int j = 0; j < MATRIX_COLUMNS_1; j++)
    {
        temp += curr_arr[j] * curr_arr[MATRIX_COLUMNS_1 + j];
    }
    m_.lock();
    pe_index = i / (SIZE_PER_PE / sizeof(float));
    
    int local_index = i % (SIZE_PER_PE / sizeof(float));

    if (pe_index < no_of_pe_to_store_output)
    {
        float *result_arr =
            static_cast<float *>(pe_arrays[resource_required + pe_index]);
        result_arr[local_index] = temp;
    }
    m_.unlock();
    
}
// Helper function to perform matrix multiplication
int pe_mat_mul_without_threads(void *pe_arrays[], int resource_required, int no_of_pe_to_store_output)
{
    pe_index = 0;
    for (int i = 0; i < resource_required; i++)
    {
        float *curr_arr = static_cast<float *>(pe_arrays[i]);

        float temp = 0;
        for (int j = 0; j < MATRIX_COLUMNS_1; j++)
        {
            temp += curr_arr[j] * curr_arr[MATRIX_COLUMNS_1 + j];
        }

        pe_index = i / (SIZE_PER_PE / sizeof(float));
        int local_index = i % (SIZE_PER_PE / sizeof(float));

        if (pe_index < no_of_pe_to_store_output)
        {
            float *result_arr =
                static_cast<float *>(pe_arrays[resource_required + pe_index]);
            result_arr[local_index] = temp;
        }
    }
    return pe_index;
}

// Helper function to perform matrix multiplication with threadpool
int pe_mat_mul_with_threadpool(void *pe_arrays[], int resource_required, int no_of_pe_to_store_output)
{
    pe_index = 0;
    ThreadPool* pool=new ThreadPool(4);
    auto startTime = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < resource_required; i++){
         pool->enqueue([pe_arrays,no_of_pe_to_store_output,i,resource_required] {
            thread_multiply(pe_arrays,no_of_pe_to_store_output,i, resource_required);
        });
        
    }
    delete pool;
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
    std::cout << "Time taken with dfa and threadpool: " << duration.count() <<" "<<"micro seconds"<< std::endl<<std::endl;
    return pe_index;
}

//main function
int main()
{
    // Allocating memory to the Hardware and slicing it to make many PEs
    const int total_pe = (PE_ROWS * PE_COLUMNS);
    
    void *pe_arrays[total_pe];
    for (int i = 0; i < total_pe; i++)
    {
        pe_arrays[i] = std::malloc(SIZE_PER_PE * ONE_BYTE);
    }

    float *a = static_cast<float *>(
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_1 * sizeof(float)));
    float *b = static_cast<float *>(
        std::malloc(MATRIX_ROWS_2 * MATRIX_COLUMNS_2 * sizeof(float)));
    float *c = static_cast<float *>(
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_2 * sizeof(float)));
    float *d = static_cast<float *>(
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_2 * sizeof(float)));

    initialize_input_data(a, b, c, d);

    // matrix multiplication without dataflow of a and b and storing the result in d
    auto startTime = std::chrono::high_resolution_clock::now();
    compute_results_without_dataflow(a, b, d);
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
    std::cout << "Time taken without dfa and threads: " << duration.count() <<" "<<"micro seconds"<< std::endl;
    int resource_required = calculate_resource();

    load_data_to_pe_arrays(a, b, pe_arrays, resource_required);

    int resource_to_store_output = MATRIX_ROWS_1 * MATRIX_COLUMNS_2;
    int no_of_pe_to_store_output = static_cast<int>(
        std::ceil(static_cast<float>(resource_to_store_output) / (SIZE_PER_PE / sizeof(float))));

    // Computing the matrix multiplication and storing the result in the the new
    // pe locations and copying the result to the c array

    startTime = std::chrono::high_resolution_clock::now();
    int pe_index = pe_mat_mul_without_threads(pe_arrays, resource_required, no_of_pe_to_store_output);
    stopTime = std::chrono::high_resolution_clock::now();

    duration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
    std::cout << "Time taken with dfa and without threads: " << duration.count() <<" "<<"micro seconds"<< std::endl;

    pe_index=pe_mat_mul_with_threadpool(pe_arrays, resource_required, no_of_pe_to_store_output);

    // function to store c the result_arr to c array
    load_data_from_pe_arrays(c, pe_arrays, resource_required, pe_index);

    // Printing the reources and verify results
    print_resource_utilization(resource_required, c, d);

    // Free allocated resources
    free_resources(pe_arrays, resource_required, a, b, c, d);

    return 0;
}