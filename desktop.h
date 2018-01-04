/*
 * Utility functions for extracting necessary data from .desktop files.
 */

struct desktop_entry {
    char *name;
    char *generic_name;
    char *exec;
};

struct desktop_entry
read_desktop_file(const char *filename);

void free_desktop_entry(struct desktop_entry *de);


int read_desktop_files_from_dirs(const char **dirnames, int dir_count, struct desktop_entry **entries);
