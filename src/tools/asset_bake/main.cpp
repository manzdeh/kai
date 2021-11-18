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

int main(int argc, char **argv) {
    if(argc != 2) {
        // TODO: Print out usage message
        return -1;
    }

    std::vector<std::string> asset_names;

    FILE *f = fopen(argv[1], "r");
    if(f) {
        static char buffer[9999];

        while(fgets(buffer, sizeof(buffer), f) != nullptr) {
            buffer[strcspn(buffer, "\r\n")] = '\0';
            asset_names.push_back(buffer);
        }

        fclose(f);
    } else {
        // TODO: Print out file not found error
        return -1;
    }

    AssetTableHeader header;
    header.count = asset_names.size();

    std::vector<ptrdiff_t> offsets(asset_names.size());

    return 0;
}
