#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct rucksack {
    char *first, *second;
} rucksack;

u_int64_t char_bitset(const char *s) {
    u_int64_t set = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        set |= 1ul << (s[i] - 'A');
    }
    return set;
}

char *bitset_char(u_int64_t set) {
    size_t count = 0;
    for (size_t i = 0; i < 8 * sizeof(u_int64_t); ++i) {
        if (set & 1ul << i) {
            ++count;
        }
    }
    char *s = calloc(count + 1, sizeof(char));
    if (!s) {
        return NULL;
    }
    size_t current = 0;
    for (size_t i = 0; i < 8 * sizeof(u_int64_t); ++i) {
        if (set & (1ul << i)) {
            s[current++] = 'A' + (char)i;
        }
    }
    s[count] = '\0';
    return s;
}

int priority(char c) {
    if (c < 'A' || c > 'z') {
        return 0;
    }

    if (c < 'Z' + 1) {
        return c - 'A' + 27;
    }

    return c - 'a' + 1;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1) {
        return usage(argv[0]);
    }

    const char *input = argv[1];
    FILE *fptr = strequ(input, "-") ? stdin : fopen(input, "r");
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    size_t rucksacks_length = 1;
    rucksack *rucksacks = calloc(rucksacks_length, sizeof(rucksack));
    if (!rucksacks) {
        fprintf(stderr, "could not allocate %ld bytes: %s\n", rucksacks_length * sizeof(rucksack), strerror(errno));
        return EXIT_FAILURE;
    }

    size_t current = 0;
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        if (!(current + 1 < rucksacks_length)) {
            rucksacks_length *= 2;
            rucksacks = realloc(rucksacks, rucksacks_length * sizeof(rucksack));
            if (!rucksacks) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", rucksacks_length * sizeof(rucksack),
                        strerror(errno));
                return EXIT_FAILURE;
            }
        }

        size_t line_length = strlen(line) - 1;
        rucksack sack = {calloc(line_length / 2 + 1, sizeof(char)), calloc(line_length / 2 + 1, sizeof(char))};
        if (!sack.first || !sack.second) {
            fprintf(stderr, "could not allocate %ld bytes: %s\n", line_length / 2 * sizeof(char), strerror(errno));
            return EXIT_FAILURE;
        }
        strncat(sack.first, line, line_length / 2);
        strncat(sack.second, line + line_length / 2, line_length / 2);
        rucksacks[current++] = sack;
    }
    rucksacks_length = current;

    {
        printf("--- Part One ---\n");
        printf(
            "Find the item type that appears in both compartments of each rucksack. What is the sum of the priorities "
            "of those item types?\n");

        int priority_sum = 0;
        for (size_t i = 0; i < rucksacks_length; ++i) {
            char *s = bitset_char(char_bitset(rucksacks[i].first) & char_bitset(rucksacks[i].second));
            if (!s) {
                fprintf(stderr, "could not convert bitset to char: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
            priority_sum += priority(s[0]);
            free(s);
        }

        printf("The sum of the priorities is %d\n", priority_sum);
    }

    {
        printf("--- Part Two ---\n");
        printf("Find the item type that corresponds to the badges of each three-Elf group. What is the sum of the "
               "priorities of those item types?\n");

        int priority_sum = 0;
        char *buffer[3] = {NULL, NULL, NULL};
        for (size_t i = 0; i < rucksacks_length; ++i) {
            buffer[i % 3] = calloc(strlen(rucksacks[i].first) + strlen(rucksacks[i].second) + 1, sizeof(char));
            if (!buffer[i % 3]) {
                fprintf(stderr, "could not allocate %ld bytes: %s\n",
                        (strlen(rucksacks[i].first) + strlen(rucksacks[i].second)) * sizeof(char), strerror(errno));
                return EXIT_FAILURE;
            }
            strcat(buffer[i % 3], rucksacks[i].first);
            strcat(buffer[i % 3], rucksacks[i].second);

            if (i % 3 == 2) {
                u_int64_t set = 0xffffffffffffffff;
                for (size_t j = 0; j < 3; ++j) {
                    set &= char_bitset(buffer[j]);
                    free(buffer[j]);
                }

                char *s = bitset_char(set);
                if (!s) {
                    fprintf(stderr, "could not convert bitset to char: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
                priority_sum += priority(s[0]);
                free(s);
            }
        }

        printf("The sum of the priorities is %d\n", priority_sum);
    }

    for (size_t i = 0; i < rucksacks_length; ++i) {
        free(rucksacks[i].first);
        free(rucksacks[i].second);
    }
    free(rucksacks);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
