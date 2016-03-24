#include "garble.h"
#include "garble/block.h"
#include "circuits.h"

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
build(garble_circuit *gc, garble_type_e type)
{
    garble_context ctxt;

    int addKeyInputs[256];
    int addKeyOutputs[128];
    int subBytesOutputs[128];
    int shiftRowsOutputs[128];
    int mixColumnOutputs[128];

    garble_new(gc, n, m, type);
    garble_start_building(gc, &ctxt);

    countToN(addKeyInputs, 256);

    for (int round = 0; round < roundLimit; ++round) {

        AddRoundKey(gc, &ctxt, addKeyInputs, addKeyOutputs);

        for (int i = 0; i < 16; ++i) {
            SubBytes(gc, &ctxt, addKeyOutputs + 8 * i, subBytesOutputs + 8 * i);
        }

        ShiftRows(subBytesOutputs, shiftRowsOutputs);

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
    garble_finish_building(gc, &ctxt, mixColumnOutputs);
}

int
main(int argc, char *argv[])
{
    garble_circuit gc;

    block *inputLabels = garble_allocate_blocks(2 * n);
    block *extractedLabels = garble_allocate_blocks(n);
    block *outputMap = garble_allocate_blocks(2 * m);
    bool *inputs = calloc(n, sizeof(bool));
    block seed;
    int times = 100;
    garble_type_e type;

    unsigned char hash[SHA_DIGEST_LENGTH];

    if (argc == 2) {
        type = atoi(argv[1]);
    }  else {
        type = GARBLE_TYPE_STANDARD;
    }

    printf("Type: ");
    switch (type) {
    case GARBLE_TYPE_STANDARD:
        printf("Standard\n");
        break;
    case GARBLE_TYPE_HALFGATES:
        printf("Half-gates\n");
        break;
    case GARBLE_TYPE_PRIVACY_FREE:
        printf("Privacy free\n");
        break;
    }

    build(&gc, type);

    seed = garble_seed(NULL);
    garble_garble(&gc, NULL, outputMap);
    memcpy(inputLabels, gc.wires, 2 * gc.n * sizeof(block));
    garble_hash(&gc, hash);

    {
        block *computedOutputMap = garble_allocate_blocks(m);
        bool *outputVals = calloc(m, sizeof(bool));
        bool *outputVals2 = calloc(m, sizeof(bool));
        for (int i = 0; i < n; ++i) {
            inputs[i] = rand() % 2;
        }
        garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
        garble_eval(&gc, extractedLabels, computedOutputMap, outputVals);
        assert(garble_map_outputs(outputMap, computedOutputMap, outputVals2, m) == GARBLE_OK);
        for (uint64_t i = 0; i < gc.m; ++i) {
            assert(outputVals[i] == outputVals2[i]);
        }
        {
            garble_circuit gc2;

            (void) garble_seed(&seed);
            build(&gc2, type);
            garble_garble(&gc2, NULL, NULL);
            assert(garble_check(&gc2, hash) == GARBLE_OK);
            garble_delete(&gc2);
        }
        free(computedOutputMap);
        free(outputVals);
        free(outputVals2);
    }
    {
        mytime_t start, end;
        double garblingTime, evalTime;
        mytime_t *timeGarble = calloc(times, sizeof(mytime_t));
        mytime_t *timeEval = calloc(times, sizeof(mytime_t));
        double *timeGarbleMedians = calloc(times, sizeof(double));
        double *timeEvalMedians = calloc(times, sizeof(double));
        bool *outputs = calloc(m, sizeof(bool));

        for (int j = 0; j < times; j++) {
            for (int i = 0; i < times; i++) {
                start = current_time_cycles();
                {
                    (void) garble_garble(&gc, inputLabels, outputMap);
                }
                end = current_time_cycles();
                timeGarble[i] = end - start;

                start = current_time_cycles();
                {
                    garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
                    garble_eval(&gc, extractedLabels, NULL, outputs);
                }
                end = current_time_cycles();
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
        free(outputs);
    }

    {
        unsigned long long start, end;
        bool *outputs = calloc(m, sizeof(bool));

        start = current_time_ns();
        for (int i = 0; i < 1000; ++i) {
            (void) garble_garble(&gc, inputLabels, outputMap);
            garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
            garble_eval(&gc, extractedLabels, NULL, outputs);
        }
        end = current_time_ns();
        printf("%llu\n", (end - start) / 1000);

        free(outputs);
    }

    garble_delete(&gc);
    free(inputs);
    free(extractedLabels);
    free(outputMap);
    free(inputLabels);
    return 0;
}
