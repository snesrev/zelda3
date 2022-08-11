#pragma once

typedef void SaveLoadFunc(void *ctx, void *data, size_t data_size);

#define SL(x) func(ctx, &x, sizeof(x))