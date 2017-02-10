#include "fslc_stdlib.h"
#include "fslc_malloc.h"
#include "fslc_assert.h"

void initialize_bins(struct bin_t *bins)
{
    size_t i;
    size_t chunk_size = 0;
    size_t chunk_increment = CHUNK_MIN_SIZE;

    fslc_assert(bins != NULL);

    for (i = 0; i < 64; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^1
    for (; i < 96; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^2
    for (; i < 112; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^3
    for (; i < 120; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^4
    for (; i < 124; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^5
    for (; i < 126; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }

    chunk_increment *= 8; // 8^6
    for (; i < 128; ++i)
    {
        chunk_size += chunk_increment;
        bins[i].size = chunk_size;
        bins[i].top = NULL;
        bins[i].bottom = NULL;
    }
}

int find_bin_gte(struct bin_t *bins, size_t target)
{
    fslc_assert(target > 0);
    fslc_assert(bins != NULL);

    return find_bin_int(bins, target - 1);
}

int find_bin_lte(struct bin_t *bins, size_t target)
{
    fslc_assert(bins != NULL);

    return find_bin_int(bins, target) - 1;
}

int find_bin_int(struct bin_t *bins, size_t target)
{
    // based on code @ http://stackoverflow.com/questions/6553970/find-the-first-element-in-an-array-that-is-greater-than-the-target

    int low = 0;
    int high = MALLOC_BIN_COUNT;
    while (low != high)
    {
        int mid = (low + high) >> 1;
        if (bins[mid].size <= target)
        {
            /* This index, and everything below it, must not be the first element
             * greater than what we're looking for because this element is no greater
             * than the element.
             */
            low = mid + 1;
        }
        else
        {
           /* This element is at least as large as the element, so anything after it can't
            * be the first element that's at least as large.
            */
            high = mid;
        }
    }
    /* Now, low and high both point to the element in question. */
    return low;
}

void store_chunk(struct bin_t *bin, struct free_header_t *chunk)
{
    fslc_assert(bin != NULL);
    fslc_assert(chunk != NULL);

    chunk->down = NULL;
    chunk->up = bin->bottom;

    if (bin->bottom)
    {
        fslc_assert(bin->top != NULL);

        bin->bottom->down = chunk;
    }
    else
    {
        fslc_assert(bin->top == NULL);

        bin->top = chunk;
    }

    bin->bottom = chunk;
}

struct free_header_t *pop_chunk(struct bin_t *bin)
{
    fslc_assert(bin != NULL);

    struct free_header_t *chunk = bin->top;

    if (chunk == NULL)
        return NULL;

    bin->top = chunk->down;

    if (bin->top)
    {
        fslc_assert(bin->top->up  != NULL);

        bin->top->up = NULL;
    }
    else
    {
        fslc_assert(bin->bottom != NULL);

        bin->bottom = NULL;
    }

    return chunk;
}

void update_chunk(struct free_header_t *chunk, size_t size, size_t bin_index, unsigned char flags)
{
    fslc_assert(chunk != NULL);
    fslc_assert(flags <= CHUNK_FLAGS_MASK);
    fslc_assert((size & CHUNK_FLAGS_MASK) == 0);
    fslc_assert(size <= CHUNK_SIZE_MASK);
    fslc_assert(size >= CHUNK_MIN_SIZE);
    fslc_assert(bin_index < MALLOC_BIN_COUNT);

    chunk->size_x[0] = (bin_index << CHUNK_BIN_IDX_SHIFT) | size | flags;

    struct chunk_footer_t *footer = ADD_ADDR(struct chunk_footer_t *, chunk, size - sizeof(struct chunk_footer_t));
    footer->size_x = chunk->size_x[0];
}
