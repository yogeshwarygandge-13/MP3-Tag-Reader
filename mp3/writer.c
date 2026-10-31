#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "writer.h"
#include "reader.h"

#define MAX_FRAMES 20

typedef struct 
{
    char id[5];
    int size;
    char *data;
} Frame;

int read_frames(FILE *fp, Frame *frames, int *count, int tag_size) 
{
    *count = 0;
    long start = ftell(fp);
    while ((ftell(fp) - start) < tag_size && *count < MAX_FRAMES) 
    {
        char id[5] = {0}; fread(id, 1, 4, fp);
        if (!id[0]) break;

        unsigned char sz[4]; fread(sz, 1, 4, fp);
        int size = (sz[0]<<24) | (sz[1]<<16) | (sz[2]<<8) | sz[3];
        fseek(fp, 2, SEEK_CUR); // skip flags

        char *data = malloc(size);
        if (!data || fread(data, 1, size, fp) != size) break;

        strcpy(frames[*count].id, id);
        frames[*count].size = size;
        frames[*count].data = data;
        (*count)++;
    }
    return 0;
}

void modify_frame(Frame *frames, int *count, const char *id, const char *text) 
{
    int size = 1 + strlen(text);
    int is_comm = strcmp(id, "COMM") == 0;
    if (is_comm) size += 4;

    char *new_data = malloc(size);
    new_data[0] = 0x00;
    int offset = 1;

    if (is_comm) 
    {
        memcpy(new_data + 1, "eng", 3);
        new_data[4] = '\0';
        offset = 5;
    }
    strcpy(new_data + offset, text);

    for (int i = 0; i < *count; i++) 
    {
        if (!strcmp(frames[i].id, id)) 
        {
            free(frames[i].data);
            frames[i].data = new_data;
            frames[i].size = size;
            return;
        }
    }

    // New frame
    strcpy(frames[*count].id, id);
    frames[*count].data = new_data;
    frames[*count].size = size;
    (*count)++;
}

void save_frames(FILE *fp, Frame *frames, int count) 
{
    fseek(fp, 10, SEEK_SET);
    for (int i = 0; i < count; i++) 
    {
        fwrite(frames[i].id, 1, 4, fp);
        unsigned char sz[4] = {
            (frames[i].size >> 24) & 0xFF,
            (frames[i].size >> 16) & 0xFF,
            (frames[i].size >> 8) & 0xFF,
            frames[i].size & 0xFF
        };
        fwrite(sz, 1, 4, fp);
        fwrite("\0\0", 1, 2, fp);
        fwrite(frames[i].data, 1, frames[i].size, fp);
    }
}

int edit_id3v2_tag(int argc, char *argv[]) 
{
    const char *file = argv[argc - 1];
    FILE *fp = fopen(file, "rb+");
    if (!fp) return perror("Open failed"), 1;

    char head[10]; fread(head, 1, 10, fp);
    if (strncmp(head, "ID3", 3) || head[3] != 3) 
    {
        printf("Invalid ID3v2.3 tag\n");
        fclose(fp); return 1;
    }

    int tag_size = ((head[6] & 0x7F) << 21) | ((head[7] & 0x7F) << 14) |
                   ((head[8] & 0x7F) << 7) | (head[9] & 0x7F);

    Frame frames[MAX_FRAMES]; int count = 0;
    read_frames(fp, frames, &count, tag_size);

    //printf("------------- Selected Edit Details--------------\n");
    for (int i = 2; i < argc - 1; i++) 
    {
        const char *id = !strcmp(argv[i], "-t") ? "TIT2" :
                         !strcmp(argv[i], "-a") ? "TPE1" :
                         !strcmp(argv[i], "-A") ? "TALB" :
                         !strcmp(argv[i], "-y") ? "TYER" :
                         !strcmp(argv[i], "-m") ? "TCON" :
                         !strcmp(argv[i], "-c") ? "COMM" : NULL;

        if (id && argv[i + 1]) 
        {
            printf("            Updated tag %s to: %s\n", id, argv[i + 1]);
            modify_frame(frames, &count, id, argv[i + 1]);
            i++;
        }
    }

    save_frames(fp, frames, count);

    for (int i = 0; i < count; i++) free(frames[i].data);
    fclose(fp);

    return read_id3v2_tag(file);
}