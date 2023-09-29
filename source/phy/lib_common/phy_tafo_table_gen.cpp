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

/*
 * @file   phy_tafo_table_gen.cpp
 * @brief  This file will generate ta/fo tables.
*/
#include <string.h>
#include "bblib_common.hpp"
#include "phy_tafo_table_gen.h"

#ifndef PI
#define PI ((float) 3.14159265358979323846)
#endif

void bblib_init_common_time_offset_tables(const struct bblib_ta_request *request, struct bblib_ta_response *response)
{
    uint32_t n, a, b;
    double tempTO, pi = PI;
    int16_t *pRealN, *pTemp;
    int16_t nTimeOffset;
    uint32_t nSeq;
    uint16_t nFft = request->n_fft_size;
    int32_t nCP = request->n_cp;
    uint32_t nSC = request->n_fullband_sc;

    pRealN = (int16_t *)malloc(nSC * sizeof(int16_t));
    pTemp = (int16_t *)malloc(nFft * 2 * sizeof(int16_t));

    if (NULL == pRealN || NULL == pTemp)
    {
        printf("\ninit_common_time_offset_tables: \n");
        printf("Not able to allocate pRealN or pTemp, size = %ld\n\n", nSC * sizeof(int16_t));
        exit(-1);
    }

    memset((void *)pRealN, 0, nSC * sizeof(int16_t));
    memset((void *)pTemp, 0, nFft * 2 * sizeof(int16_t));

    // calculate phase shift real and imagine part in 16S15 FXP
    for (n = 0; n < nFft; n++)
    {
        tempTO = (2 * pi * n) / nFft;
        pTemp[2 * n]     = (int16_t)(round(cos(tempTO) * 32767.0));
        pTemp[2 * n + 1] = (int16_t)(round(sin(tempTO) * 32767.0));
    }

    for (n = 0; n < nSC; n++)
    {
        if (n < (nSC / 2))
        {
            pRealN[n] = n + (nFft - nSC / 2);
        }
        else if ((n >= (nSC / 2)))
        {
            pRealN[n] = n - nSC / 2;
        }
    }

    nSeq = 0;
    for (nTimeOffset = -nCP; nTimeOffset <= nCP; nTimeOffset++)
    {
        for (n = 0; n < nSC; n++)
        {
            a = pRealN[n];
            b = (a*nTimeOffset) % nFft;
            response->pCeTaFftShiftScCp[2 * nSeq]     = pTemp[2 * b];
            response->pCeTaFftShiftScCp[2 * nSeq + 1] = pTemp[2 * b + 1];
            nSeq = nSeq + 1;
        }
    }

    free(pRealN);
    free(pTemp);

}



void bblib_init_common_frequency_compensation_tables(const struct bblib_fo_request *request, struct bblib_fo_response *response)
{
    
    int16_t nFft = request->n_fft_size;
    int16_t n, i;
    double tempTO = 0, pi = PI;
    
    int16_t cosine, sine;
    double factor = 2 * pi / nFft;

    // calculate phase shift real and imagine part in 16S15 FXP
    for (n = 0; n < nFft; n++)
    {
        cosine = (int16_t)(round(cos(tempTO) * 32767.0));
        sine = (int16_t)(round(sin(tempTO) * 32767.0));
        for (i = 0; i < 16; i++)
        {
            response->pFoCompScCp[2 * i + 32 * n] = cosine;
            response->pFoCompScCp[2 * i + 1 + 32 * n] = sine;
        }
        tempTO += factor;
    }
}

