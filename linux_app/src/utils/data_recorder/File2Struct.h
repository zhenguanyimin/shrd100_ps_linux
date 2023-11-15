#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

template<typename T>
int file2struct(const char * filename, T &t)
{
    FILE *fp = fopen(filename, "rb");
    if(!fp)
    {
        std::cout << "can not open file:" << filename << std::endl;
        return -1;
    }

    size_t struct_len = sizeof(t);
    size_t real_len = fread((char *)(&t), 1, struct_len, fp);
    if (real_len != struct_len) return -2;

    fclose(fp);

    return 0;
}

template<typename T>
int struct2file(const char * filename, T &t)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        std::cout << "can not open file:" << filename << std::endl;
        return -1;
    }

    size_t struct_len = sizeof(t);
    size_t real_len = fwrite((char *)(&t), 1, struct_len, fp);
    if (real_len != struct_len) return -2;

    fclose(fp);
    return 0;
}

int file2array(const char * filename, float  buff[], int len);

int array2file(const char * filename, int16_t  buff[], int len);
