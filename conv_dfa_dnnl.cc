#include "device.h"

#define HEIGHT 224
#define WIDTH 224
#define CHANNEL 3
#define OUTPUT_CHANNELS 64
#define INPUT_CHANNELS 3
#define KERNEL_SIZE 3
#define ONE_BYTE 8
#define OFFSET 2
#define PADDING 0
#define STRIDE 1

// util function to write to dnnl memory
inline void write_to_dnnl_memory(const void *handle, dnnl::memory &mem) {
  dnnl::engine eng = mem.get_engine();
  size_t size = mem.get_desc().get_size();

  if (eng.get_kind() == dnnl::engine::kind::cpu) {
    uint8_t *output = static_cast<uint8_t *>(mem.get_data_handle());
    if (output != nullptr) std::memcpy(output, handle, size);
  }
}
// util functions to read data from dnnl memory
inline void read_from_dnnl_memory(void *handle, const dnnl::memory &mem) {
  dnnl::engine eng = mem.get_engine();
  size_t size = mem.get_desc().get_size();

  if (eng.get_kind() == dnnl::engine::kind::cpu) {
    uint8_t *src = static_cast<uint8_t *>(mem.get_data_handle());
    if (src != nullptr) std::memcpy(handle, src, size);
  }
}
// function to print the shapes of operation and save the output
void print_and_save(memory &output_mem, std::string node_name,
                    std::string op_type, memory::dims &ip_dims,
                    memory::dims &op_dims) {
  std::cout << "\n\n===== Node Name: " << node_name << " =====";
  std::cout << "\n Node OpType: " << op_type;
  std::cout << "\n Input Shape : ";
  for (auto dim : ip_dims) {
    std::cout << dim << " ";
  }
  std::cout << "\n Output Shape : ";
  for (auto dim : op_dims) {
    std::cout << dim << " ";
  }
  save_to_npy(output_mem, op_dims, node_name);
  std::cout << std::endl;
}
// function to save the output from dnnl memory to npy
void save_to_npy(memory &output_mem, memory::dims &output_dims,
                 const std::string &filename) {
  try {
    std::vector<size_t> vec_dims(output_dims.begin(), output_dims.end());
    unsigned long total_size = 1;
    for (auto dim : output_dims) {
      total_size *= dim;
    }
    std::vector<float> final_output_data(total_size);
    read_from_dnnl_memory(final_output_data.data(), output_mem);

    std::string npy_filename = "./outputs/cpp_" + filename + ".npy";
    cnpy::npy_save(npy_filename, &final_output_data[0], vec_dims, "w");
  } catch (...) {
    std::cout << "Error while saving dnnl memory" << std::endl;
  }
}
// function to save the output from float array to npy
void save_to_npy(float *final_output_data, const std::string &filename,
                 memory::dims &output_dims) {
  try {
    std::string npy_filename = "./outputs/cpp_" + filename + ".npy";
    std::vector<size_t> vec_dims(output_dims.begin(), output_dims.end());
    cnpy::npy_save(npy_filename, final_output_data, vec_dims, "w");
  } catch (...) {
    std::cout << "Error while saving float array" << std::endl;
  }
}
// function to initialize the PE array
void initialize_pe_array_for_convolution(void *pe_arrays[], float *input, float* weight, memory::dims &input_dims, memory::dims &weight_dims) {
  // Initialize the PE with the input 
  float *weight_pe = static_cast<float *>(pe_arrays[0]);
  float *first_pe = static_cast<float *>(pe_arrays[1]);
  float *second_pe = static_cast<float *>(pe_arrays[2]);
  float *third_pe = static_cast<float *>(pe_arrays[3]);
  float *fourth_pe = static_cast<float *>(pe_arrays[4]);
  float *fifth_pe = static_cast<float *>(pe_arrays[5]);
  float *sixth_pe = static_cast<float *>(pe_arrays[6]);
  float *seventh_pe = static_cast<float *>(pe_arrays[7]);
  float *eighth_pe = static_cast<float *>(pe_arrays[8]);
  
  // Initialize weight PE
  for (int i = 0; i < OUTPUT_CHANNELS; i++) {
    for (int j = 0; j < INPUT_CHANNELS; j++) {
      for (int k = 0; k < KERNEL_SIZE; k++) {
        for (int l = 0; l < KERNEL_SIZE; l++) {
          
          weight_pe[i * INPUT_CHANNELS * KERNEL_SIZE * KERNEL_SIZE + j * KERNEL_SIZE * KERNEL_SIZE + k * KERNEL_SIZE + l] =
            weight[i * INPUT_CHANNELS * KERNEL_SIZE * KERNEL_SIZE + j * KERNEL_SIZE * KERNEL_SIZE + k * KERNEL_SIZE + l];
        }
      }
    }
  }

  int pe_counter=1;
  for (int i = 0; i < CHANNEL; i++) {
        for (int j = 0; j < HEIGHT / 8 + (OFFSET); j++) {
        for (int k = 0; k < WIDTH + (OFFSET); k++) {
            for(int l=0;l<8;l++){
                if(l==0){
                    int jj = j - (KERNEL_SIZE / 2);
                    int kk = k - (KERNEL_SIZE / 2);
                    
                    if (jj >= 0 && kk >= 0 && kk < WIDTH) {
                    reinterpret_cast<float*>(pe_arrays[pe_counter])[i * (HEIGHT / 8 + (OFFSET)) * (WIDTH + (OFFSET)) +
                            j * (WIDTH + (OFFSET)) + k] =
                        input[i * HEIGHT * WIDTH + jj * WIDTH + kk];
                    } else {
                    reinterpret_cast<float*>(pe_arrays[pe_counter])[i * (HEIGHT / 8 + (OFFSET)) * (WIDTH + (OFFSET)) +
                            j * (WIDTH + (OFFSET)) + k] = 0;
                    }
                }else{
                    
                    int jj = j + (l*(HEIGHT / 8)) - (KERNEL_SIZE / 2);
                    int kk = k - (KERNEL_SIZE / 2);
                    if (jj >= 0 && jj< HEIGHT&& kk >= 0 && kk < WIDTH) {
                    reinterpret_cast<float*>(pe_arrays[pe_counter + l])[i * ((HEIGHT / 8) + OFFSET) * (WIDTH + OFFSET) +
                                j * (WIDTH + OFFSET) + k] =
                        input[i * HEIGHT * WIDTH + jj * WIDTH + kk];
                    } else {
                    reinterpret_cast<float*>(pe_arrays[pe_counter + l])[i * ((HEIGHT / 8) + OFFSET) * (WIDTH + OFFSET) +
                                j * (WIDTH + OFFSET) + k] = 0;
                    }
                }
                
            }
        }
        }
    }
  
  std::cout << "Initialized PE's" << std::endl;
}

// Common function for convolution
memory dnnl_conv(engine &eng, stream &eng_stream, memory::dims &src_dims,
                 memory::dims &weights_dims, memory::dims &bias_dims,
                 memory::dims &output_dims, int st, int pad,
                 float *weights_data, float *bias_data, float *src_data) {
  auto src_md =
      memory::desc(src_dims, memory::data_type::f32, memory::format_tag::nchw);
  auto weights_md = memory::desc(weights_dims, memory::data_type::f32,
                                 memory::format_tag::oihw);
  auto bias_md =
      memory::desc(bias_dims, memory::data_type::f32, memory::format_tag::x);
  auto output_md = memory::desc(output_dims, memory::data_type::f32,
                                memory::format_tag::nchw);

  // Create memory objects
  auto src_mem = memory(src_md, eng);
  auto weights_mem = memory(weights_md, eng);
  auto bias_mem = memory(bias_md, eng);
  auto conv_output_mem = memory(output_md, eng);

  // Write data to memory objects
  write_to_dnnl_memory(weights_data, weights_mem);
  write_to_dnnl_memory(bias_data, bias_mem);
  write_to_dnnl_memory(src_data, src_mem);

  // Create convolution primitive
  auto conv_pd = convolution_forward::primitive_desc(
      eng, prop_kind::forward_inference, algorithm::convolution_direct, src_md,
      weights_md, bias_md, output_md, {st, st}, {pad, pad}, {pad, pad});

  auto conv_prim = convolution_forward(conv_pd);

  // Execute convolution primitive
  conv_prim.execute(eng_stream, {{DNNL_ARG_SRC, src_mem},
                                 {DNNL_ARG_WEIGHTS, weights_mem},
                                 {DNNL_ARG_BIAS, bias_mem},
                                 {DNNL_ARG_DST, conv_output_mem}});
  eng_stream.wait();
  return conv_output_mem;
}
//util function associated with threads
void perform_convolution(dnnl::engine& eng, dnnl::stream& eng_stream,  memory::dims& input_dims,
                          memory::dims& weights_dims,  memory::dims& bias_dims,
                          memory::dims& output_dims, int stride, int padding,
                         float* weight_pe, float* bias, float* input_pe, float* output_pe, const std::string conv_name, int pe_number) {
    auto conv_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                 output_dims, STRIDE, PADDING, weight_pe, bias, input_pe);

    conv_output_data[pe_number-1] = output_pe;

    read_from_dnnl_memory(output_pe, conv_output);
    print_and_save(conv_output, conv_name + std::to_string(pe_number), "conv", input_dims, output_dims);
}

memory convolution_dfa(engine &eng, stream &eng_stream,
                       memory::dims &input_dims, memory::dims &weights_dims,
                       memory::dims &bias_dims, memory::dims &output_dims,
                       void *pe_arrays[], int st, int pad) {
    std::string conv_name = "conv_output";
    // get weights and inputs from PE
    float *weight_pe = static_cast<float *>(pe_arrays[0]);
    float *first_pe = static_cast<float *>(pe_arrays[1]);
    float *second_pe = static_cast<float *>(pe_arrays[2]);
    float *third_pe = static_cast<float *>(pe_arrays[3]);
    float *fourth_pe = static_cast<float *>(pe_arrays[4]);
    float *fifth_pe = static_cast<float *>(pe_arrays[5]);
    float *sixth_pe = static_cast<float *>(pe_arrays[6]);
    float *seventh_pe = static_cast<float *>(pe_arrays[7]);
    float *eighth_pe = static_cast<float *>(pe_arrays[8]);
    // use PE to store the output from convolution
    float *ninth_pe = static_cast<float *>(pe_arrays[9]);
    float *tenth_pe = static_cast<float *>(pe_arrays[10]);
    float *eleventh_pe = static_cast<float *>(pe_arrays[11]);
    float *twelfth_pe = static_cast<float *>(pe_arrays[12]);
    float *thirteenth_pe = static_cast<float *>(pe_arrays[13]);
    float *fourteenth_pe = static_cast<float *>(pe_arrays[14]);
    float *fifteenth_pe = static_cast<float *>(pe_arrays[15]);
    float *sixteenth_pe = static_cast<float *>(pe_arrays[16]);
    
    // set bias to zero
    float *bias = (float *)std::malloc(bias_dims[0] * sizeof(float));
    std::memset(bias, 0, bias_dims[0] * sizeof(float));

    auto start = std::chrono::high_resolution_clock::now();
    std::thread threads[8];

    threads[0]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream),  std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims), STRIDE, PADDING, weight_pe, bias, first_pe, ninth_pe, conv_name, 1);
    
    threads[1]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream),  std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, second_pe, tenth_pe, conv_name, 2);

    threads[2]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream), std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, third_pe, eleventh_pe, conv_name, 3);

    threads[3]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream), std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, fourth_pe, twelfth_pe, conv_name, 4);

    threads[4]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream),  std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, fifth_pe, thirteenth_pe, conv_name, 5);

    threads[5]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream),  std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, sixth_pe, fourteenth_pe, conv_name, 6);

    threads[6]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream), std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, seventh_pe, fifteenth_pe, conv_name, 7);

    threads[7]=std::thread(perform_convolution, std::ref(eng), std::ref(eng_stream), std::ref(input_dims), std::ref(weights_dims), 
    std::ref(bias_dims), std::ref(output_dims),  STRIDE, PADDING, weight_pe, bias, eighth_pe, sixteenth_pe, conv_name, 8);

    // Join all threads
    for (int i=0;i<8;i++) {
        threads[i].join();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    start = std::chrono::high_resolution_clock::now(); 
    /* Perfom convolution for all the 8 split parts */
    // Perform and process each convolution output
    auto conv_first_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                      output_dims, STRIDE, PADDING, weight_pe, bias, first_pe);
    conv_output_data[0] = ninth_pe;
    read_from_dnnl_memory(conv_output_data[0], conv_first_output);
    print_and_save(conv_first_output, conv_name + "1", "conv", input_dims, output_dims);

    // Second convolution output
    auto conv_second_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                        output_dims, STRIDE, PADDING, weight_pe, bias, second_pe);
    conv_output_data[1] = tenth_pe;
    read_from_dnnl_memory(conv_output_data[1], conv_second_output);
    print_and_save(conv_second_output, conv_name + "2", "conv", input_dims, output_dims);

    // Third convolution output
    auto conv_third_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                      output_dims, STRIDE, PADDING, weight_pe, bias, third_pe);
    conv_output_data[2] = eleventh_pe;
    read_from_dnnl_memory(conv_output_data[2], conv_third_output);
    print_and_save(conv_third_output, conv_name + "3", "conv", input_dims, output_dims);

    // Fourth convolution output
    auto conv_fourth_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                        output_dims, STRIDE, PADDING, weight_pe, bias, fourth_pe);
    conv_output_data[3] = twelfth_pe;
    read_from_dnnl_memory(conv_output_data[3], conv_fourth_output);
    print_and_save(conv_fourth_output, conv_name + "4", "conv", input_dims, output_dims);

    // Fifth convolution output
    auto conv_fifth_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                      output_dims, STRIDE, PADDING, weight_pe, bias, fifth_pe);
    conv_output_data[4] = thirteenth_pe;
    read_from_dnnl_memory(conv_output_data[4], conv_fifth_output);
    print_and_save(conv_fifth_output, conv_name + "5", "conv", input_dims, output_dims);

    // Sixth convolution output
    auto conv_sixth_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                      output_dims, STRIDE, PADDING, weight_pe, bias, sixth_pe);
    conv_output_data[5] = fourteenth_pe;
    read_from_dnnl_memory(conv_output_data[5], conv_sixth_output);
    print_and_save(conv_sixth_output, conv_name + "6", "conv", input_dims, output_dims);

    // Seventh convolution output
    auto conv_seventh_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                        output_dims, STRIDE, PADDING, weight_pe, bias, seventh_pe);
    conv_output_data[6] = fifteenth_pe;
    read_from_dnnl_memory(conv_output_data[6], conv_seventh_output);
    print_and_save(conv_seventh_output, conv_name + "7", "conv", input_dims, output_dims);

    // Eighth convolution output
    auto conv_eighth_output = dnnl_conv(eng, eng_stream, input_dims, weights_dims, bias_dims,
                                        output_dims, STRIDE, PADDING, weight_pe, bias, eighth_pe);
    conv_output_data[7] = sixteenth_pe;
    read_from_dnnl_memory(conv_output_data[7], conv_eighth_output);
    print_and_save(conv_eighth_output, conv_name + "8", "conv", input_dims, output_dims);
      
    stop = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    //  Merge the outputs into a single array
    float *merged_pe =
          (float *)std::malloc(HEIGHT * OUTPUT_CHANNELS * WIDTH * sizeof(float));
      for (int i = 0; i < OUTPUT_CHANNELS; i++) {
        for (int j = 0; j < (HEIGHT / 8); j++) {
            for (int k = 0; k < WIDTH; k++) {
                // merge the outputs from the 8 convolutions
                merged_pe[i * HEIGHT * WIDTH + j * WIDTH + k] =
                    conv_output_data[0][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + HEIGHT / 8) * WIDTH + k] =
                    conv_output_data[1][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (2 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[2][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (3 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[3][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (4 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[4][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (5 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[5][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (6 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[6][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
                merged_pe[i * HEIGHT * WIDTH + (j + (7 * (HEIGHT / 8))) * WIDTH + k] =
                    conv_output_data[7][i * ((HEIGHT / 8) * WIDTH) + j * WIDTH + k];
            }
        }
}
  std::cout << "With Threads took " << duration1.count() << " microseconds to execute." << std::endl;
  std::cout << "Without Threads took " << duration2.count() << " microseconds to execute." << std::endl;
  std::cout << "Merged output to npy files" << std::endl;
  memory::dims merged_output_dims = {1, OUTPUT_CHANNELS, HEIGHT, WIDTH};
  auto merged_conv_md = memory::desc(merged_output_dims, memory::data_type::f32,
                                     memory::format_tag::nchw);
  auto merged_conv_output = memory(merged_conv_md, eng, merged_pe);
  write_to_dnnl_memory(merged_pe, merged_conv_output);
  print_and_save(merged_conv_output, conv_name + "_merged", "merge",
                 merged_output_dims, merged_output_dims);

  return merged_conv_output;
}
void run_convolution(engine &eng, stream &eng_stream, void *pe_arrays[]) {
  // declare input and weight
  int input_size = HEIGHT * WIDTH * INPUT_CHANNELS;
  int weight_size =
      OUTPUT_CHANNELS * INPUT_CHANNELS * KERNEL_SIZE * KERNEL_SIZE;
  float *input = (float *)std::malloc(input_size * sizeof(float));
  float *weight = (float *)std::malloc(weight_size * sizeof(float));
  for (int i = 0; i < input_size; i++) {
    input[i] = i + 1;
  }
  for (int i = 0; i < weight_size; i++) {
    weight[i] = i + 1;
  }
  // define the input, weight, bias and output shapes
  memory::dims input_dims;
  memory::dims weights_dims;
  memory::dims bias_dims;
  memory::dims output_dims;
  input_dims = {1, INPUT_CHANNELS, ((HEIGHT / 8) + OFFSET), (WIDTH + OFFSET)};
  weights_dims = {OUTPUT_CHANNELS, INPUT_CHANNELS, KERNEL_SIZE, KERNEL_SIZE};
  bias_dims = {OUTPUT_CHANNELS};
  output_dims = {1, OUTPUT_CHANNELS, (HEIGHT / 8), WIDTH};

  initialize_pe_array_for_convolution(pe_arrays, input, weight, input_dims,
                                      weights_dims);

  auto conv_output1 = convolution_dfa(eng, eng_stream, input_dims, weights_dims,
                                      bias_dims, output_dims, pe_arrays, 1, 0);
}

int main() {
  const int total_pe = (PE_ROWS * PE_COLUMNS);
  void *pe_arrays[total_pe];

  for (int i = 0; i < total_pe; i++) {
    pe_arrays[i] = std::malloc(
        (OUTPUT_CHANNELS  * HEIGHT/8 * WIDTH) * sizeof(float));
  }

  // Engine creation
  engine eng(engine::kind::cpu, 0);
  stream eng_stream(eng);
  
  run_convolution(eng, eng_stream, pe_arrays);
  std::cout << "Dumped outputs to output/ folder" << std::endl;
  
  return 0;
}