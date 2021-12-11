#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // sleep()関数を使う

void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp)
{
    int MAX_LEN = height * width;
    int init_x[MAX_LEN];
    int init_y[MAX_LEN];
    int n = 0;  // 初期値の個数
    if (fp != NULL) {
        // read the data.
        char buffer[20];
        while (fgets(buffer, 20, fp) != NULL) {
            if (buffer[0] != '#') {
                char *end1, *end2;
                init_x[n] = strtol(buffer, &end1, 10);
                init_y[n] = strtol(end1, &end2, 10);
                ++n;
            }
        }
    } else {
        srand((unsigned int)time(NULL));  // 現在時刻の情報で初期化
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                double r = (double)rand() / RAND_MAX;
                if (r < 0.10) {  // 初期化の際の確率は0.10
                    init_x[n] = j;
                    init_y[n] = i;
                    ++n;
                }
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        cell[init_y[i]][init_x[i]] = 1;
    }
}

// Life1.0.6形式で出力
void my_print_cells(FILE* fp, const int height, const int width, int cell[height][width])
{
    fprintf(fp, "#Life 1.06\n");
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int state = cell[i][j];
            if (state == 1) {
                fprintf(fp, "%d %d\n", j, i);
            }
        }
    }
}

int my_count_adjacent_cells(int h, int w, const int height, const int width, int cell[height][width])
{
    int count = 0;
    if (h > 0 && w > 0) {
        count += cell[h - 1][w - 1];
    }
    if (h > 0) {
        count += cell[h - 1][w];
    }
    if (h > 0 && w < width - 1) {
        count += cell[h - 1][w + 1];
    }
    if (w > 0) {
        count += cell[h][w - 1];
    }
    if (w < width - 1) {
        count += cell[h][w + 1];
    }
    if (h < height - 1 && w > 0) {
        count += cell[h + 1][w - 1];
    }
    if (h < height - 1) {
        count += cell[h + 1][w];
    }
    if (h < height - 1 && w < width - 1) {
        count += cell[h + 1][w + 1];
    }
    return count;
}

void my_update_cells(const int height, const int width, int cell[height][width])
{
    int next_cell[height][width];
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int count = my_count_adjacent_cells(i, j, height, width, cell);
            if (cell[i][j]) {
                if (count == 2 || count == 3) {
                    next_cell[i][j] = 1;
                } else {
                    next_cell[i][j] = 0;
                }
            } else {
                if (count == 3) {
                    next_cell[i][j] = 1;
                } else {
                    next_cell[i][j] = 0;
                }
            }
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            cell[i][j] = next_cell[i][j];
        }
    }
}

int main(int argc, char** argv)
{
    const int height = 40;
    const int width = 70;

    int cell[height][width];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cell[y][x] = 0;
        }
    }

    /* ファイルを引数にとるか、ない場合はデフォルトの初期値を使う */
    if (argc > 2) {
        fprintf(stderr, "usage: %s [filename for init]\n", argv[0]);
        return EXIT_FAILURE;
    } else if (argc == 2) {
        FILE* lgfile;
        if ((lgfile = fopen(argv[1], "r")) != NULL) {
            my_init_cells(height, width, cell, lgfile);  // ファイルによる初期化
        } else {
            fprintf(stderr, "cannot open file %s\n", argv[1]);
            return EXIT_FAILURE;
        }
        fclose(lgfile);
    } else {
        my_init_cells(height, width, cell, NULL);  // デフォルトの初期値を使う
    }

    /* 世代を進める*/
    for (int gen = 1; gen < 10000; gen++) {
        my_update_cells(height, width, cell);  // セルを更新
        if (gen % 100 == 0) {
            FILE* outputfile;  // 出力ストリーム
            char filename[30];
            sprintf(filename, "./output/gen%04d.lif", gen);
            outputfile = fopen(filename, "w");  // ファイルを書き込み用にオープン(開く)
            if (outputfile == NULL) {
                printf("cannot open %s\n", filename);
                return EXIT_FAILURE;
            }
            my_print_cells(outputfile, height, width, cell);  // 表示する
            fclose(outputfile);                               // ファイルをクローズ(閉じる)
        }
    }

    return EXIT_SUCCESS;
}
