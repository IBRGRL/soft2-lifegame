#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // sleep()関数を使う

#define MAX_LEN 100

void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp)
{
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
        init_x[0] = 30;
        init_y[0] = 20;
        init_x[1] = 30;
        init_y[1] = 22;
        init_x[2] = 31;
        init_y[2] = 22;
        init_x[3] = 31;
        init_y[3] = 23;
        init_x[4] = 32;
        init_y[4] = 20;
        n = 5;
    }

    for (int i = 0; i < n; ++i) {
        cell[init_y[i]][init_x[i]] = 1;
    }
}

void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width])
{
    fprintf(fp, "generation = %d\n", gen);
    fprintf(fp, "+");
    for (int j = 0; j < width; ++j) {
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");
    for (int i = 0; i < height; ++i) {
        fprintf(fp, "|");
        fprintf(fp, "\e[31m");  // それ以降の文字出力を赤色に変更するシーケンス
        for (int j = 0; j < width; ++j) {
            int state = cell[i][j];
            if (state == 1) {
                fprintf(fp, "#");
            } else {
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "\e[0m");  // 元に戻すシーケンス
        fprintf(fp, "|\n");
    }
    fprintf(fp, "+");
    for (int j = 0; j < width; ++j) {
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");
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
    FILE* fp = stdout;
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

    my_print_cells(fp, 0, height, width, cell);  // 表示する
    sleep(1);                                    // 1秒休止

    /* 世代を進める*/
    for (int gen = 1;; gen++) {
        my_update_cells(height, width, cell);          // セルを更新
        my_print_cells(fp, gen, height, width, cell);  // 表示する
        sleep(1);                                      //1秒休止する
        fprintf(fp, "\e[%dA", height + 3);             //height+3 の分、カーソルを上に戻す(壁2、表示部1)
    }

    return EXIT_SUCCESS;
}
