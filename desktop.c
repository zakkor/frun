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

        if (!found_entry_block) {
            continue;
        }
        if (line[0] == '[') {
            /* Once we find the start of a new block, stop */
            break;
        }

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
 * Reads all the .desktop files from the given directories returning an array of entries
 */
int read_desktop_files_from_dirs(const char **dirnames, int dir_count, struct desktop_entry **entries)
{
    /* TODO: dynamic */
    *entries = malloc(sizeof(struct desktop_entry) * 400); 
    int entry_count = 0;

    for (int i = 0; i < dir_count; i++) {
        DIR *dp = opendir(dirnames[i]);

        if (dp) {
            struct dirent *dir;
            while ((dir = readdir(dp)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0 ||
                    strcmp(dir->d_name, "..") == 0) {
                    /* TODO: check if extension is .desktop */
                    continue;
                }

                char full_path[256] = "";
                snprintf(full_path, sizeof(full_path), "%s/%s", dirnames[i], dir->d_name);

                (*entries)[entry_count] = read_desktop_file(full_path); 
                entry_count++;
            }

            closedir(dp);
        }
    }

    return entry_count;
}
