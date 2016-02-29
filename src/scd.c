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

#include "garble.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <msgpack.h>
#include <malloc.h>
#include <sys/stat.h>

static long
fsize(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) == 0)
		return st.st_size;
	return -1;
}

int
garble_to_file(GarbledCircuit *gc, char *fname)
{
    msgpack_sbuffer *buffer;
    msgpack_packer *pk;
	FILE *f;

    if ((f = fopen(fname, "wb")) == NULL) {
        perror("fopen");
		return -1;
	}
	buffer = msgpack_sbuffer_new();
	pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);
	msgpack_sbuffer_clear(buffer);

	msgpack_pack_array(pk, 4 + 4 * gc->q + 2 * gc->nFixedWires + gc->m);
	msgpack_pack_int(pk, gc->n);
	msgpack_pack_int(pk, gc->m);
	msgpack_pack_int(pk, gc->q);
    msgpack_pack_int(pk, gc->nFixedWires);
	for (int i = 0; i < gc->q; ++i) {
		msgpack_pack_int(pk, gc->gates[i].input0);
        msgpack_pack_int(pk, gc->gates[i].input1);
        msgpack_pack_int(pk, gc->gates[i].output);
        msgpack_pack_int(pk, gc->gates[i].type);
	}
	for (int i = 0; i < gc->nFixedWires; ++i) {
		msgpack_pack_int(pk, gc->fixedWires[i].type);
        msgpack_pack_int(pk, gc->fixedWires[i].idx);
	}
	for (int i = 0; i < gc->m; ++i) {
		msgpack_pack_int(pk, gc->outputs[i]);
	}
	fwrite(buffer->data, sizeof(char), buffer->size, f);
    fclose(f);
    msgpack_packer_free(pk);
    msgpack_sbuffer_free(buffer);
	return 0;
}

int
garble_from_file(GarbledCircuit *gc, char *fname)
{
    msgpack_sbuffer buffer;
    msgpack_unpacked msg;
    msgpack_object *p;
    FILE *f = NULL;
    int res = -1;

	msgpack_sbuffer_init(&buffer);

	if ((buffer.size = fsize(fname)) == -1)
        goto cleanup;
	if ((f = fopen(fname, "rb")) == NULL) {
        perror("fopen");
        goto cleanup;
	}

	if ((buffer.data = malloc(buffer.size)) == NULL)
        goto cleanup;
	(void) fread(buffer.data, sizeof(char), buffer.size, f);

	msgpack_unpacked_init(&msg);
	msgpack_unpack_next(&msg, buffer.data, buffer.size, NULL);
	p = msg.data.via.array.ptr;
	gc->n = (*p++).via.i64;
	gc->m = (*p++).via.i64;
	gc->q = (*p++).via.i64;
    gc->nFixedWires = (*p++).via.i64;
    gc->r = gc->n + gc->q + gc->nFixedWires;

    gc->gates = calloc(gc->q, sizeof(Gate));
    gc->garbledTable = calloc(gc->q, sizeof(GarbledTable));
    gc->wires = calloc(gc->r, sizeof(Wire));
    gc->outputs = calloc(gc->m, sizeof(int));
    gc->fixedWires = calloc(gc->nFixedWires, sizeof(FixedWire));
	if (gc->outputs == NULL || gc->gates == NULL || gc->garbledTable == NULL
        || gc->wires == NULL || gc->fixedWires == NULL) {
		return -1;
	}
	for (int i = 0; i < gc->q; ++i) {
		gc->gates[i].input0 = (*p++).via.i64;
        gc->gates[i].input1 = (*p++).via.i64;
        gc->gates[i].output = (*p++).via.i64;
        gc->gates[i].type = (*p++).via.i64;
	}
    for (int i = 0; i < gc->nFixedWires; ++i) {
        gc->fixedWires[i].type = (*p++).via.i64;
        gc->fixedWires[i].idx = (*p++).via.i64;
    }
	for (int i = 0; i < gc->m; ++i) {
		gc->outputs[i] = (*p++).via.i64;
	}
    msgpack_unpacked_destroy(&msg);

    res = 0;
cleanup:
    if (f)
        fclose(f);
    msgpack_sbuffer_destroy(&buffer);
	return res;
}
