#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <string>
#include <vector>


extern int pcre_test(std::vector<std::string> regs, const char* src, ssize_t len);
extern int hs_test(std::vector<std::string> regs, const char* src, ssize_t len);

const char patterns[][128] = {
    "Twain",
    "(?i)Twain",
    "[a-z]shing",
    "Huck[a-zA-Z]+|Saw[a-zA-Z]+",
    "\\b\\w+nn\\b",
    "[a-q][^u-z]{13}x",
    "Tom|Sawyer|Huckleberry|Finn",
    "(?i)Tom|Sawyer|Huckleberry|Finn",
    ".{0,2}(Tom|Sawyer|Huckleberry|Finn)",
    ".{2,4}(Tom|Sawyer|Huckleberry|Finn)",
    "Tom.{10,25}river|river.{10,25}Tom",
    "[a-zA-Z]+ing",
    "\\s[a-zA-Z]{0,12}ing\\s",
    "([A-Za-z]awyer|[A-Za-z]inn)\\s",
    "[\"'][^\"']{0,30}[?!\\.][\"']",
    "\u221E|\u2713",
    "\\p{Sm}", // any mathematical symbol
};


struct Mmap
{
    int fd;
    char* data;
    size_t length;
};


int load_file(const char* filename, Mmap* mp)
{
    ssize_t file_size = -1;
    struct stat statbuff;

    if (stat(filename, &statbuff) < 0) {
        return -1;
    } else {
        file_size = statbuff.st_size;
    }

    mp->fd = open(filename, O_RDONLY);
    if (mp->fd < 0) {
        free(mp);
        return -1;
    }
    mp->data = (char*) malloc(file_size);
    mp->length = file_size;
    read(mp->fd, mp->data, file_size);
    close(mp->fd);
    return 0;
}

int load_reg_file(std::vector<std::string>& regs)
{
    Mmap fm;
    int ret = 0;
    ret = load_file("data/reg.txt", &fm);
    if (ret < 0) {
        return -1;
    }

    FILE* fp = fopen("data/reg_check.txt", "w");
    if (fp == nullptr) {
        return -1;
    }

    char buff[2048];
    for (size_t j = 0, one_line_cnt = 0; j < fm.length; j++) {
        if (fm.data[j] != '\n') {
            one_line_cnt++;
            continue;
        }
        memset(buff, 0, sizeof(buff));
        memcpy(buff, &fm.data[j - one_line_cnt], one_line_cnt>2048?2048:one_line_cnt);
        fm.data[one_line_cnt] = 0;
        one_line_cnt = 0;

        regs.push_back(buff);
        fprintf(fp, "%lu:/%s/\n", regs.size(), buff);
    }

    fclose(fp);

    return 0;
}

int main()
{

    Mmap mmap;
    std::vector<std::string> regs;
    int ret = load_file("data/mtent12.txt", &mmap);
    if (ret < 0) {
        fprintf(stderr, "fail to load \n");
        return -1;
    }


    if (load_reg_file(regs) < 0) {
        for (int i = 0; i < sizeof(patterns) / 128; i++) {
            regs.push_back(std::string(patterns[i]));
        }
    }

    //pcre_test(regs, mmap.data, mmap.length);
    hs_test(regs, mmap.data, mmap.length);

    return 0;
}

