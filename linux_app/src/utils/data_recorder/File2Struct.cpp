#include "File2Struct.h"


int file2array(const char * filename, float  buff[], int len)
{
    std::ifstream fin;
    fin.open(filename, std::ios_base::in | std::ios_base::binary);
    if (!fin.is_open())
    {
        std::cout << "can not open file:" << filename << std::endl;
        return -1;
    }

    int read_count = 0;
    bool low_bit = true;
    char ch = 0;
    int16_t result = 0;
    while (!fin.eof())
    {
        fin.get(ch);

        if (fin.fail())
        {
            std::cout << "\nFile reading error" << std::endl;
            return -1;
        }

        if(!low_bit)
        {
            result += ch << 8;
            std::cout << "result: " << result << std::endl;
            float float_result = (float) result / 10;
            buff[read_count] = float_result;
            result = 0;
            read_count ++;

            if(read_count >= len) break;

            low_bit = true;
        }
        else
        {
            result += (unsigned char)ch ;
            low_bit = false;
        }

    }
    fin.close();

    if(read_count != len) return -2;

    return 0;

}


int array2file(const char * filename, int16_t  buff[], int len)
{

    char ch='a';
    std::ofstream OpenFile(filename,std::ios::binary);

    if (OpenFile.fail())
    {
        std::cout<<"打开文件错误!"<<std::endl;
        return -1;
    }

    char * begin = (char *)buff;
    for (int i=0;i<len*2;i++)
    {
        OpenFile.put(*begin);
        begin++;
    }

    OpenFile.close();

    return 0;
}


#if 0
/* 单独测试代码 */

//#define HEX_OUT(x)  std::cout  << std::hex << x;
#define HEX_OUT(x)  printf("0x%x\t",x);
typedef struct {
    char sn[64];
    short compensate_data[2560];
    short extra_compensate_data[128];
}CompensateData;

void showCompensateData(CompensateData & cd)
{
    std::cout << "SN :"  << cd.sn << std::endl;
    std::cout << "hex :";
    for(int i=0;i < 64; i++)
    {
        if(i % 16 == 0)  std::cout  << std::endl;
        HEX_OUT(cd.sn[i]);
    }


    std::cout << std::endl << "CompensateData begin :"  << std::endl;
    for(int i=0;i < 2560; i++)
    {
        if(i % 16 == 0)  std::cout  << std::endl;
        std::cout  << cd.compensate_data[i]  << "\t";
    }

    std::cout << std::endl << "ExtraCompensateData begin :"  << std::endl;
    for(int i = 0; i < 128; i++)
    {
        if(i % 16 == 0)  std::cout  << std::endl;
        std::cout  << cd.extra_compensate_data[i] << "\t";
    }
    std::cout << std::endl;
}

int main(int argc, char * argv[])
{
    int16_t arr[16] = {306,3500, 2508,4012, 1276, 718,33, 24, 168, 910, 418, 681, 231, 123,681, 682};
    array2file("./a.data", arr, 16);
    float arr1[16];
    file2array("./a.data", arr1, 16);
    for(int i = 0; i < 16; i++)
    {
        std::cout << "org:" <<arr[i] << ", dest" << arr1[i] << " " << std::endl;
    }

    CompensateData cd;
    int ret = file2struct("./b.bin", cd);
    if(ret)
    {
        std::cout << "cannot open file :./b.bin, return : " << ret << std::endl;
        return 0;
    }
    showCompensateData(cd);
    return 0;
}
#endif
