#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"

void print_tag(const char *id, const char *data, int size) 
{
    if (size <= 1 || !data) return;

    const char *label = strcmp(id, "TIT2") == 0 ? "Title" :
                        strcmp(id, "TPE1") == 0 ? "Artist" :
                        strcmp(id, "TALB") == 0 ? "Album" :
                        strcmp(id, "TYER") == 0 ? "Year" :
                        strcmp(id, "TCON") == 0 ? "Genre" :
                        strcmp(id, "COMM") == 0 ? "Comment" : NULL;

    if (label) 
    {
        if (strcmp(id, "COMM") == 0) 
        {
            int offset = 4; // skip encoding + lang + short desc
            while (offset < size && data[offset] != '\0') offset++;
            offset++;
            printf("%-8s: %.*s\n", label, size - offset, data + offset);
        } 
        else 
        {
            printf("%-8s: %.*s\n", label, size - 1, data + 1); // skip encoding byte
        }
    }
}

int read_id3v2_tag(const char *filename) 
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) 
    {
        perror("Error opening file");
        return 1;
    }

    char header[10];
    fread(header, 1, 10, fp);
    if (strncmp(header, "ID3", 3) != 0 || header[3] != 3) 
    {
        printf("Invalid ID3v2.3 tag\n");
        fclose(fp);
        return 1;
    }

    int tag_size = ((header[6] & 0x7F) << 21) |
                   ((header[7] & 0x7F) << 14) |
                   ((header[8] & 0x7F) << 7) |
                   (header[9] & 0x7F);

    printf("       MP3 Tag Reader and Editor for ID3v2\n");
    printf("-------------------------------------------------\n");

    long start = ftell(fp);
    while ((ftell(fp) - start) < tag_size) 
    {
        char id[5] = {0};
        if (fread(id, 1, 4, fp) != 4 || id[0] == '\0') break;

        unsigned char size_bytes[4];
        fread(size_bytes, 1, 4, fp);
        int size = (size_bytes[0] << 24) | (size_bytes[1] << 16) |
                   (size_bytes[2] << 8) | size_bytes[3];

        fseek(fp, 2, SEEK_CUR); // skip flags

        if (size <= 0) break;

        char *data = malloc(size);
        if (!data || fread(data, 1, size, fp) != size) 
        {
            free(data);
            break;
        }

        print_tag(id, data, size);
        free(data);
    }

    printf("-------------------------------------------------\n");
    printf("--------- Displayed Details Successfully --------\n");
    fclose(fp);
    return 0;
}