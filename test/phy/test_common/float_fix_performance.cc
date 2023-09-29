/**********************************************************************
*
*
*  Copyright [2019 - 2023] [Intel Corporation]
* 
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  
*  You may obtain a copy of the License at
*  
*     http://www.apache.org/licenses/LICENSE-2.0 
*  
*  Unless required by applicable law or agreed to in writing, software 
*  distributed under the License is distributed on an "AS IS" BASIS, 
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and 
*  limitations under the License. 
*  
*  SPDX-License-Identifier: Apache-2.0 
*  
* 
*
**********************************************************************/

#include "common.hpp"

#include "float_int16_convert_agc.h"

enum class TestType {
    I2I = 0,
    I2F = 1,
    F2I = 2,
    F2I_TH = 3,
    I2I_FXP = 4
};

struct Int16_to_int16_data {
    int16_t* output;
    int16_t* input;
};

struct Int16_to_float_data {
    float* output;
    int16_t* input;
};

struct Float_to_int16_data {
    int16_t* output;
    float* input;
};

struct Float_to_int16_th_data {
    int16_t* output;
    float* input;
};

struct Int16_to_int16_fxp_data {
    int16_t* output;
    int16_t* input;
};

const std::string module_name = "float_fix_convert";

class FloatFixConvertPerf : public KernelTests {
protected:
    struct Int16_to_int16_data i2i{};
    struct Int16_to_float_data i2f{};
    struct Float_to_int16_data f2i{};
    struct Float_to_int16_th_data f2i_th{};
    struct Int16_to_int16_fxp_data i2i_fxp{};

    /* There are 4 functions to test, 4 types of tests are implemented:
       bblib_int16_to_int16_agc (TestType::I2I),
       bblib_int16_to_float_agc (TestType::I2F)
       bblib_float_to_int16_agc (TestType::F2I)
       bblib_float_to_int16_agc_threshold (TestType::F2I_TH) */
    TestType test_type;

    /* Common parameters */
    int32_t num_data;
    float gain;

    /* Parameter for bblib_float_to_int16_agc_threshold */
    int16_t threshold;

    void SetUp() override {
        init_test("float_fix_performance");

        /* Defined as the maximum size of all inputs/outputs in BYTE */
        const int buffer_len = 1024 * 1024 * 120;
        test_type = TestType(get_input_parameter<int>("test_type"));

        switch (test_type) {
            case TestType::I2I :
                i2i.input = generate_random_int_numbers<int16_t>((buffer_len / sizeof(int16_t)), 64, -32768, 32767);
                i2i.output = aligned_malloc<int16_t>((buffer_len / sizeof(int16_t)), 64);
                break;
            case TestType::I2F :
                i2f.input = generate_random_int_numbers<int16_t>((buffer_len / sizeof(int16_t)), 64, -32768, 32767);
                i2f.output = aligned_malloc<float>((buffer_len / sizeof(float)), 64);
                break;
            case TestType::F2I :
                f2i.input = generate_random_real_numbers<float>((buffer_len / sizeof(float)), 64, -100.0f, 100.0f);
                f2i.output = aligned_malloc<int16_t>((buffer_len / sizeof(int16_t)), 64);
                break;
            case TestType::F2I_TH :
                f2i_th.input = generate_random_real_numbers<float>((buffer_len / sizeof(float)), 64, -100.0f, 100.0f);
                f2i_th.output = aligned_malloc<int16_t>((buffer_len / sizeof(int16_t)), 64);
                threshold = get_input_parameter<int16_t>("threshold");
                break;
            case TestType::I2I_FXP :
                i2i_fxp.input = generate_random_int_numbers<int16_t>((buffer_len / sizeof(int16_t)), 64, -32768, 32767);
                i2i_fxp.output = aligned_malloc<int16_t>((buffer_len / sizeof(int16_t)), 64);
                break;

        }
        num_data = get_input_parameter<int32_t>("num_data");
        gain = get_input_parameter<float>("gain");
    }

    void TearDown() override {
        switch (test_type) {
            case TestType::I2I :
                aligned_free(i2i.input);
                aligned_free(i2i.output);
                break;
            case TestType::I2F :
                aligned_free(i2f.input);
                aligned_free(i2f.output);
                break;
            case TestType::F2I :
                aligned_free(f2i.input);
                aligned_free(f2i.output);
                break;
            case TestType::F2I_TH :
                aligned_free(f2i_th.input);
                aligned_free(f2i_th.output);
                break;
            case TestType::I2I_FXP :
                aligned_free(i2i_fxp.input);
                aligned_free(i2i_fxp.output);
                break;

        }
    }
};

TEST_P(FloatFixConvertPerf, C_Perf)
{
    switch (test_type) {
        case TestType::I2I :
            performance("C", module_name, bblib_int16_to_int16_agc_c,
                        i2i.output, i2i.input, num_data, gain);
            break;
        case TestType::I2F :
            performance("C", module_name, bblib_int16_to_float_agc_c,
                        i2f.output, i2f.input, num_data, gain);
            break;
        case TestType::F2I :
            performance("C", module_name, bblib_float_to_int16_agc_c,
                        f2i.output, f2i.input, num_data, gain);
            break;
        case TestType::F2I_TH :
            performance("C", module_name, bblib_float_to_int16_agc_threshold_c,
                        f2i_th.output, f2i_th.input, num_data, gain, threshold);
            break;
        case TestType::I2I_FXP :
            performance("C", module_name, bblib_int16_to_int16_fxp_scale_c,
                        i2i_fxp.output, i2i_fxp.input, num_data, gain);
        }
}

#ifdef _BBLIB_AVX2_
TEST_P(FloatFixConvertPerf, AVX2_Perf)
{
    switch (test_type) {
        case TestType::I2I :
            performance("AVX2", module_name, bblib_int16_to_int16_agc_avx2,
                        i2i.output, i2i.input, num_data, gain);
            break;
        case TestType::I2F :
            performance("AVX2", module_name, bblib_int16_to_float_agc_avx2,
                        i2f.output, i2f.input, num_data, gain);
            break;
        case TestType::F2I :
            performance("AVX2", module_name, bblib_float_to_int16_agc_avx2,
                        f2i.output, f2i.input, num_data, gain);
            break;
        case TestType::F2I_TH :
            performance("AVX2", module_name, bblib_float_to_int16_agc_threshold_avx2,
                        f2i_th.output, f2i_th.input, num_data, gain, threshold);
            break;
        case TestType::I2I_FXP:
            break;
    }
}
#endif

#ifdef _BBLIB_AVX512_
TEST_P(FloatFixConvertPerf, AVX512_Perf)
{
    switch (test_type) {
        case TestType::I2I :
            performance("AVX512", module_name, bblib_int16_to_int16_agc_avx512,
                        i2i.output, i2i.input, num_data, gain);
            break;
        case TestType::I2F :
            performance("AVX512", module_name, bblib_int16_to_float_agc_avx512,
                        i2f.output, i2f.input, num_data, gain);
            break;
        case TestType::F2I :
            performance("AVX512", module_name, bblib_float_to_int16_agc_avx512,
                        f2i.output, f2i.input, num_data, gain);
            break;
        case TestType::F2I_TH :
            performance("AVX512", module_name, bblib_float_to_int16_agc_threshold_avx512,
                        f2i_th.output, f2i_th.input, num_data, gain, threshold);
            break;
        case TestType::I2I_FXP :
            performance("AVX512", module_name, bblib_int16_to_int16_fxp_scale_avx512,
                        i2i_fxp.output, i2i_fxp.input, num_data, gain);
            break;

        }
}
#endif

INSTANTIATE_TEST_CASE_P(UnitTest, FloatFixConvertPerf,
                        testing::ValuesIn(get_sequence(FloatFixConvertPerf::get_number_of_cases("float_fix_performance"))));