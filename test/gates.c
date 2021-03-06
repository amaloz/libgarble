#include <garble.h>
#include "circuits.h"
#include "circuit_builder.h"
#include "utils.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

static void
build_circuit(garble_circuit *gc, int n, int nlayers, garble_type_e type,
              void (*f)(garble_circuit *, garble_context *, int, int, int))
{
    garble_context ctxt;
    int *inputs = calloc(n, sizeof(int));
    int *outputs = calloc(n, sizeof(int));

    garble_new(gc, n, n, type);
    builder_start_building(gc, &ctxt);
    builder_init_wires(inputs, n);
    for (int layer = 0; layer < nlayers; ++layer) {
        for (int i = 0; i < n; i += 2) {
            int wire = builder_next_wire(&ctxt);
            f(gc, &ctxt, inputs[i], inputs[i + 1], wire);
            outputs[i] = outputs[i + 1] = wire;
        }
        memcpy(inputs, outputs, n * sizeof(int));
    }
    builder_finish_building(gc, &ctxt, outputs);

    free(inputs);
    free(outputs);
}

static void
test_circuit(int n, int nlayers, garble_type_e type,
             void (*f)(garble_circuit *, garble_context *, int, int, int))
{
    garble_circuit gc;
    block *inputLabels = garble_allocate_blocks(2 * n);
    block *outputLabels = garble_allocate_blocks(2 * n);
    block *extractedLabels = garble_allocate_blocks(n);
    block *computedOutputLabels = garble_allocate_blocks(n);
    bool *inputs = calloc(n, sizeof(bool));
    bool *outputs = calloc(n, sizeof(bool));

    build_circuit(&gc, n, nlayers, type, f);

    (void) garble_seed(NULL);
    garble_garble(&gc, NULL, outputLabels);

    printf("Input:  ");
    for (uint64_t i = 0; i < gc.n; ++i) {
        inputLabels[2 * i] = gc.wires[2 * i];
        inputLabels[2 * i + 1] = gc.wires[2 * i + 1];
        inputs[i] = rand() % 2;
        printf("%d", inputs[i]);
    }
    printf("\n");
    garble_extract_labels(extractedLabels, inputLabels, inputs, n);
    garble_eval(&gc, extractedLabels, computedOutputLabels, NULL);
    assert(garble_map_outputs(outputLabels, computedOutputLabels, outputs, gc.m) == GARBLE_OK);
    printf("Output: ");
    for (uint64_t i = 0; i < gc.m; ++i) {
        printf("%d", outputs[i]);
    }
    printf("\n");

    garble_delete(&gc);

    free(inputLabels);
    free(outputLabels);
    free(extractedLabels);
    free(computedOutputLabels);
    free(inputs);
    free(outputs);
}

static void
measure_circuit(int n, int nlayers, int ntimes, garble_type_e type,
                void (*f)(garble_circuit *, garble_context *, int, int, int))
{
    garble_circuit gc;
    block *inputLabels = garble_allocate_blocks(2 * n);
    block *outputLabels = garble_allocate_blocks(2 * n);
    block *computedOutputLabels = garble_allocate_blocks(n);
    bool *outputs = calloc(n, sizeof(bool));

    build_circuit(&gc, n, nlayers, type, f);

    (void) garble_seed(NULL);
    garble_garble(&gc, NULL, outputLabels);

    mytime_t start, end;
    double garblingTime, evalTime;
    mytime_t *timeGarble = calloc(ntimes, sizeof(mytime_t));
    mytime_t *timeEval = calloc(ntimes, sizeof(mytime_t));
    double *timeGarbleMedians = calloc(ntimes, sizeof(double));
    double *timeEvalMedians = calloc(ntimes, sizeof(double));
    block *extractedLabels = garble_allocate_blocks(gc.n);
    block *outputMap = garble_allocate_blocks(gc.m);
    bool *inputs = calloc(gc.n, sizeof(bool));

    for (int j = 0; j < ntimes; j++) {
        printf(".");
        fflush(stdout);
        for (int i = 0; i < ntimes; i++) {
            start = current_time_cycles();
            {
                (void) garble_garble(&gc, inputLabels, outputLabels);
            }
            end = current_time_cycles();
                
            timeGarble[i] = end - start;

            for (uint64_t k = 0; k < gc.n; ++k) {
                inputs[k] = rand() % 2;
            }
                
            start = current_time_cycles();
            {
                garble_extract_labels(extractedLabels, inputLabels, inputs, gc.n);
                garble_eval(&gc, extractedLabels, outputMap, NULL);
            }
            end = current_time_cycles();
            timeEval[i] = end - start;
        }
        timeGarbleMedians[j] = ((double) median(timeGarble, ntimes)) / gc.q;
        timeEvalMedians[j] = ((double) median(timeEval, ntimes)) / gc.q;
    }
    printf("\n");
    garblingTime = doubleMean(timeGarbleMedians, ntimes);
    evalTime = doubleMean(timeEvalMedians, ntimes);
    printf("%lf %lf\n", garblingTime, evalTime);

    free(inputLabels);
    free(outputLabels);
    free(computedOutputLabels);
    free(outputs);
    free(extractedLabels);
    free(outputMap);
    free(inputs);
    free(timeGarble);
    free(timeEval);
    free(timeGarbleMedians);
    free(timeEvalMedians);

}

int
main(int argc, char *argv[])
{
    int ninputs, nlayers, ntimes;
    garble_type_e type;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <ninputs> <nlayers> <ntimes> <type>\n", argv[0]);
        exit(1);
    }

    ninputs = atoi(argv[1]);
    nlayers = atoi(argv[2]);
    ntimes = atoi(argv[3]);
    type = atoi(argv[4]);

    if (ninputs % 2 != 0) {
        fprintf(stderr, "Error: ninputs must be even\n");
        exit(1);
    }

    printf("Type: ");
    switch (type) {
    default:
        type = GARBLE_TYPE_STANDARD;
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

    printf("***** Testing AND *****\n");
    test_circuit(ninputs, nlayers, type, gate_AND);
    printf("***** Testing OR *****\n");
    test_circuit(ninputs, nlayers, type, gate_OR);
    printf("***** Testing XOR *****\n");
    test_circuit(ninputs, nlayers, type, gate_XOR);
    printf("***** Measuring AND *****\n");
    measure_circuit(ninputs, nlayers, ntimes, type, gate_AND);

    return 0;
}
