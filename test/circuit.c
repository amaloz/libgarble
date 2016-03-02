#include "garble.h"
#include "circuits.h"
#include "gates.h"
#include "utils.h"

#include <assert.h>

static void
build_GF4MULCircuit(garble_circuit *gc, garble_type_e type)
{
    garble_context ctxt;

    int inputs[4];
    int outputs[2];

    countToN(inputs, 4);
    garble_new(gc, 4, 2, 7, 11, type);
    garble_start_building(gc, &ctxt);
    GF4MULCircuit(gc, &ctxt, inputs, outputs);
    garble_finish_building(gc, outputs);
}

static void
build_GF16MULCircuit(garble_circuit *gc, garble_type_e type)
{
    garble_context ctxt;

    int inputs[4];
    int outputs[4];

    countToN(inputs, 4);
    garble_new(gc, 4, 4, 100, 100, type);
    garble_start_building(gc, &ctxt);
    GF16MULCircuit(gc, &ctxt, inputs, outputs);
    garble_finish_building(gc, outputs);
}

static void
build_GF256InvCircuit(garble_circuit *gc, garble_type_e type)
{
    garble_context ctxt;

    int inputs[8];
    int outputs[8];

    countToN(inputs, 8);
    garble_new(gc, 8, 8, 1000, 1000, type);
    garble_start_building(gc, &ctxt);
    GF256InvCircuit(gc, &ctxt, inputs, outputs);
    garble_finish_building(gc, outputs);
}

static void
test_garbled_circuit(garble_circuit *gc)
{
    block *inputLabels = garble_allocate_blocks(2 * gc->n);
    block *outputLabels = garble_allocate_blocks(2 * gc->m);
    block *extractedLabels = garble_allocate_blocks(gc->n);
    block *computedOutputLabels = garble_allocate_blocks(gc->n);
    bool *inputs = calloc(gc->n, sizeof(bool));
    bool *outputs = calloc(gc->n, sizeof(bool));

    (void) garble_seed(NULL);
    garble_garble(gc, NULL, outputLabels);
    printf("Input:  ");
    for (uint64_t i = 0; i < gc->n; ++i) {
        inputLabels[2 * i] = gc->wires[i].label0;
        inputLabels[2 * i + 1] = gc->wires[i].label1;
        inputs[i] = rand() % 2;
        printf("%d", inputs[i]);
    }
    printf("\n");
    garble_extract_labels(extractedLabels, inputLabels, inputs, gc->n);
    garble_eval(gc, extractedLabels, computedOutputLabels);
    assert(garble_map_outputs(outputLabels, computedOutputLabels, outputs, gc->m) == GARBLE_OK);
    printf("Output: ");
    for (uint64_t i = 0; i < gc->m; ++i) {
        printf("%d", outputs[i]);
    }
    printf("\n");

    free(inputLabels);
    free(outputLabels);
    free(extractedLabels);
    free(computedOutputLabels);
    free(inputs);
    free(outputs);
}

int
main(int argc, char *argv[])
{
    garble_circuit gc;
    garble_type_e type;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <type>\n", argv[0]);
        exit(1);
    }

    type = atoi(argv[1]);

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

    printf("***** GF4MULCircuit *****\n");
    build_GF4MULCircuit(&gc, type);
    test_garbled_circuit(&gc);
    garble_delete(&gc);

    printf("***** GF16MULCircuit *****\n");
    build_GF16MULCircuit(&gc, type);
    test_garbled_circuit(&gc);
    garble_delete(&gc);

    printf("***** GF256InvCircuit *****\n");
    build_GF256InvCircuit(&gc, type);
    test_garbled_circuit(&gc);
    garble_delete(&gc);

    return 0;
}
