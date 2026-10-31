/*name-yogeshwary
date-17/7/25
project on mp3 tag reader*/
#include <stdio.h>
#include <string.h>
#include "reader.h"
#include "writer.h"

void show_help() 
{
    printf("\nUsage:\n");
    printf("  ./a.out -v <file>        View tags\n");
    printf("  ./a.out -e -t/-a/-A/-y/-m/-c <text> <file>  Edit tags\n");
    printf("  ./a.out --help            Show help\n");
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        show_help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) 
    {
        show_help();
    } 
    else if (strcmp(argv[1], "-v") == 0 && argc == 3) 
    {
        read_id3v2_tag(argv[2]);
    } 
    else if (strcmp(argv[1], "-e") == 0) 
    {
        edit_id3v2_tag(argc, argv);
    } 
    else 
    {
        printf("Invalid input.\n");
        show_help();
    }

    return 0;
}