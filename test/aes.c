/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <garble.h>
#include <circuits.h>

#include "utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <openssl/sha.h>

#define AES_CIRCUIT_FILE_NAME "./aesCircuit"

static const int roundLimit = 10;
static const int n = 128 * (10 + 1);
static const int m = 128;

static void
buildAESCircuit(GarbledCircuit *gc)
{
	GarblingContext ctxt;

    int q = 50000;
	int r = 50000;
    int addKeyInputs[256];
    int addKeyOutputs[128];
    int subBytesOutputs[128];
    int shiftRowsOutputs[128];
    int mixColumnOutputs[128];

	garble_new(gc, n, m, q, r);
	garble_start_building(gc, &ctxt);

	countToN(addKeyInputs, 256);

	for (int round = 0; round < roundLimit; round++) {

		AddRoundKey(gc, &ctxt, addKeyInputs, addKeyOutputs);

		for (int i = 0; i < 16; i++) {
			SubBytes(gc, &ctxt, addKeyOutputs + 8 * i, subBytesOutputs + 8 * i);
		}

		ShiftRows(gc, &ctxt, subBytesOutputs, shiftRowsOutputs);

		for (int i = 0; i < 4; i++) {
			if (round != roundLimit - 1)
				MixColumns(gc, &ctxt, shiftRowsOutputs + i * 32,
                           mixColumnOutputs + 32 * i);
		}
		for (int i = 0; i < 128; i++) {
			addKeyInputs[i] = mixColumnOutputs[i];
			addKeyInputs[i + 128] = (round + 2) * 128 + i;
		}
	}
	garble_finish_building(gc, mixColumnOutputs);
}

int
main(int argc, char *argv[])
{
	GarbledCircuit gc;

    block *inputLabels = garble_allocate_blocks(2 * n);
    block *extractedLabels = garble_allocate_blocks(n);
    block *outputMap = garble_allocate_blocks(2 * m);
    int *inputs = calloc(n, sizeof(int));
    block seed;
    int times;

    unsigned char hash[SHA_DIGEST_LENGTH];
    GarbleType type = GARBLE_TYPE_STANDARD;

    if (argc == 2) {
        times = atoi(argv[1]);
    } else {
        times = 100;
    }

    buildAESCircuit(&gc);
    garble_to_file(&gc, AES_CIRCUIT_FILE_NAME);
    garble_delete(&gc);
	garble_from_file(&gc, AES_CIRCUIT_FILE_NAME);

    seed = garble_seed(NULL);
    garble_create_input_labels(inputLabels, n, NULL);
    garble_garble(&gc, inputLabels, outputMap, type);
    garble_hash(&gc, hash, type);

    {
        block *extractedLabels = garble_allocate_blocks(n);
        block *computedOutputMap = garble_allocate_blocks(m);
        int *inputs = calloc(n, sizeof(int));
        int *outputVals = calloc(m, sizeof(int));
        for (int i = 0; i < n; ++i) {
            inputs[i] = rand() % 2;
        }
        garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
        garble_eval(&gc, extractedLabels, computedOutputMap, type);
        assert(garble_map_outputs(outputMap, computedOutputMap, outputVals, m) == GARBLE_OK);
        {
            GarbledCircuit gc2;

            (void) garble_seed(&seed);
            garble_create_input_labels(inputLabels, n, NULL);
            buildAESCircuit(&gc2);
            garble_garble(&gc2, inputLabels, NULL, type);
            assert(garble_check(&gc2, hash, type) == GARBLE_OK);
            garble_delete(&gc2);
        }
        free(extractedLabels);
        free(computedOutputMap);
        free(inputs);
        free(outputVals);
    }
    {
        mytime_t start, end;
        double garblingTime, evalTime;
        mytime_t *timeGarble = calloc(times, sizeof(mytime_t));
        mytime_t *timeEval = calloc(times, sizeof(mytime_t));
        double *timeGarbleMedians = calloc(times, sizeof(double));
        double *timeEvalMedians = calloc(times, sizeof(double));

        for (int j = 0; j < times; j++) {
            for (int i = 0; i < times; i++) {
                start = current_time();
                {
                    (void) garble_garble(&gc, inputLabels, outputMap, type);
                }
                end = current_time();
                timeGarble[i] = end - start;

                start = current_time();
                {
                    garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
                    garble_eval(&gc, extractedLabels, outputMap, type);
                }
                end = current_time();
                timeEval[i] = end - start;
            }
            timeGarbleMedians[j] = ((double) median(timeGarble, times)) / gc.q;
            timeEvalMedians[j] = ((double) median(timeEval, times)) / gc.q;
        }
        garblingTime = doubleMean(timeGarbleMedians, times);
        evalTime = doubleMean(timeEvalMedians, times);
        printf("%lf %lf\n", garblingTime, evalTime);

        free(timeGarble);
        free(timeEval);
        free(timeGarbleMedians);
        free(timeEvalMedians);
    }

    /* { */
    /*     mytime_t start, end; */
    /*     start = current_time(); */
    /*     for (int i = 0; i < 1000; ++i) { */
    /*         (void) garbleCircuit(&gc, inputLabels, outputMap, type); */
    /*     } */
    /*     end = current_time(); */
    /*     printf("%lu\n", (end - start) / 1000000); */

    /*     start = current_time(); */
    /*     for (int i = 0; i < 1000; ++i) { */
    /*         extractLabels(extractedLabels, inputLabels, inputs, gc.n); */
    /*         evaluate(&gc, extractedLabels, outputMap, type); */
    /*     } */
    /*     end = current_time(); */
    /*     printf("%lu\n", (end - start) / 1000000); */
    /* } */

    garble_delete(&gc);
    free(extractedLabels);
    free(outputMap);
    free(inputLabels);
	return 0;
}
