// Offline tool used to generate a table of all paths to the assets that are used in a game.
// The table is used during runtime to load assets from disk if they're not already loaded

#include <stddef.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <unordered_set>
#include <utility>

#include "../../asset/asset_table.h"
#include "../../core/includes/utils.h"

static void print_usage(void) {
    fprintf(stdout, "Asset bake usage:\n"
            "\tasset_bake <name of text file>\n");
}

int main(int argc, char **argv) {
    if(argc != 2) {
        print_usage();
        return -1;
    }

    std::vector<std::pair<std::string, Uint64>> assets;

    FILE *f = fopen(argv[1], "r");
    if(f) {
        static char buffer[9999];
        std::unordered_set<Uint64> ids; // This is only used to check for hashed string collisions

        while(fgets(buffer, sizeof(buffer), f) != nullptr) {
            buffer[strcspn(buffer, "\r\n")] = '\0';
            assets.push_back({buffer, kai::fnv1a64_str_hash(buffer)});

            if(!ids.insert(assets.back().second).second) {
                fprintf(stderr, "[ERROR] - Hash collision found for the asset name \"%s\"\n", buffer);
                return -1;
            }
        }

        fclose(f);
    } else {
        fprintf(stderr, "[ERROR] - File \"%s\" was not found or it couldn't be opened\n", argv[1]);
        return -1;
    }

    const size_t asset_count = assets.size();

    std::unordered_set<Uint64> set;
    Uint64 mod = 1ull;

    bool mapped_range;

    do {
        mapped_range = true;
        set.clear();

        for(const auto &asset : assets) {
            Uint64 index = (asset.second ^ mod) % asset_count;

            mapped_range &= set.insert(index).second;

            if(!mapped_range) {
                goto try_next;
            }
        }

        if(mapped_range) {
            break;
        }

try_next:
        mod++;
    } while(mod != 0);

    if(mapped_range) {
        const char *file_name = "asset_manifest.bin";
        FILE *manifest = fopen(file_name, "wb");

        if(manifest) {
            std::vector<ptrdiff_t> offsets;
            offsets.reserve(asset_count);
            const size_t start = sizeof(ptrdiff_t) * asset_count;

            for(Uint64 index : set) {
                ptrdiff_t offset = start;

                for(Uint64 i = 0; i < index; i++) {
                    offset += assets[i].first.size();
                }

                offsets.push_back(offset);
            }

            KAI_PUSH_DISABLE_COMPILER_WARNINGS(4815);
            AssetTableHeader header;
            header.count = asset_count;
            header.mod = mod;
            KAI_POP_COMPILER_WARNINGS;

            fwrite(&header, sizeof(AssetTableHeader), 1, manifest);

            for(ptrdiff_t offset : offsets) {
                fwrite(&offset, sizeof(offset), 1, manifest);
            }

            for(Uint64 index : set) {
                fwrite(assets[index].first.c_str(), sizeof(char), assets[index].first.size() + 1, manifest);
            }

            if(fclose(manifest) == 0) {
                fprintf(stdout, "Successfully saved asset lookup list to \"%s\"\n", file_name);
            }
        }

    } else {
        fprintf(stderr, "[ERROR] - Could not map all hashed asset names to the available slots\n");
        return -1;
    }

    return 0;
}
