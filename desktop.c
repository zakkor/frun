#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "desktop.h"

struct desktop_entry
read_desktop_file(const char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    const char* TARGET_BLOCK_NAME = "Desktop Entry";
    int found_entry_block = 0;

    struct desktop_entry de;
    de.name = NULL;
    de.generic_name = NULL;
    de.exec = NULL;

    /* 
     * First we must find the line containing [Desktop Entry]
     * which signals the start of the relevant block
     */
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[0] == '[') {
            /* Found opening block delimiter */

            char block_name[50] = "";
            for (int i = 1; i < strlen(line); i++) {
                if (line[i] == ']') {
                    break;
                }

                block_name[i-1] = line[i];
            }

            if (strcmp(block_name, TARGET_BLOCK_NAME) == 0) {
                /* Found relevant block, now we can start 
                 * gathering key,val pairs */
                found_entry_block = 1;
                continue;
            }
        }

        /* Keep going only if we're in the right block */
        if (!found_entry_block)
            continue;

        /* Once we find the start of a new block, stop */
        if (line[0] == '[')
            break;

        /* TODO: dynamically resize these */
        char key[9000] = "", val[9000] = "";
        int looking_for_key = 1;
        int idx = 0;

        for (int i = 0; i < strlen(line); i++) {
            char curr = line[i];

            if (curr == '\n') {
                break;
            }

            if (curr == '=') {
                looking_for_key = 0;
                idx = 0;
                continue;
            }

            if (looking_for_key) {
                key[idx] = curr;
            } else {
                val[idx] = curr;
            }

            idx++;
        }

        /* Fill our struct with the values we found */
        if (strcmp(key, "Name") == 0) {
            de.name = strdup(val);
        } else if (strcmp(key, "GenericName") == 0) {
            de.generic_name = strdup(val);
        } else if (strcmp(key, "Exec") == 0) {
            de.exec = strdup(val);
        }
    }

    fclose(fp);
    free(line);

    return de;
}


void free_desktop_entry(struct desktop_entry *de)
{
    free(de->name);
    free(de->generic_name);
    free(de->exec);
}


/* 
 * Reads all the .desktop files from the given directories populating an array of entries,
 * and returns the number of entries
 */
int read_desktop_files_from_dirs(const char **dirnames, int dir_count, struct desktop_entry **entries)
{
    const unsigned int INITIAL_ALLOCATION_SIZE = 100;

    unsigned int allocated_size = INITIAL_ALLOCATION_SIZE;
    *entries = malloc(allocated_size * sizeof(struct desktop_entry)); 
    if (! *entries) {
        exit(1);
    }

    unsigned int entry_count = 0;
    for (int i = 0; i < dir_count; i++) {
        DIR *dp = opendir(dirnames[i]);

        if (dp) {
            struct dirent *dir;
            while ((dir = readdir(dp)) != NULL) {
                char *filename = dir->d_name;

                if (strcmp(filename, ".") == 0 ||
                    strcmp(filename, "..") == 0) {
                    continue;
                }

                /* Check if extension is 'desktop' */
                char *ext = strrchr(filename, '.');
                if (!ext) {
                    continue;
                }
                ext++;
                if (strcmp(ext, "desktop") != 0) {
                    continue;
                }

                char full_path[256] = "";
                snprintf(full_path, sizeof(full_path), "%s/%s", dirnames[i], dir->d_name);

                if (entry_count+1 > allocated_size) {
                    allocated_size *= 2;
                    *entries = realloc(*entries, allocated_size * sizeof(**entries));
                }

                (*entries)[entry_count] = read_desktop_file(full_path); 
                entry_count++;
            }

            closedir(dp);
        }
    }

    /* Shrink down to actual size */
    *entries = realloc(*entries, entry_count * sizeof(**entries));

    return entry_count;
}

void free_all_desktop_entries(struct desktop_entry **entries, int entry_count)
{
    for (int i = 0; i < entry_count; i++) {
        free_desktop_entry(&(*entries)[i]);
    }
    free(*entries);
}
