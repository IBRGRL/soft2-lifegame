#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // sleep()関数を使う

int is_num(const char val)
{
    const char nums[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    for (int i = 0; i < 10; ++i) {
        if (val == nums[i]) {
            return 1;
        }
    }
    return 0;
}

void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp)
{
    int MAX_LEN = height * width;
    int init_x[MAX_LEN];
    int init_y[MAX_LEN];
    int n = 0;  // 初期値の個数(RLE以外で使用)
    // 以下4つはRLEで使用
    int current_x = 0;
    int current_y = 0;
    int num_flag = 1;  // 読み取る文字が数字であることを期待。もし数字ではなかったらラン長が1ということに対応する。
    int run_length = 0;
    if (fp != NULL) {
        // read the data.
        char buffer[80];
        int is_rle = 0;
        int height2 = 0;
        int width2 = 0;
        while (fgets(buffer, 80, fp) != NULL) {
            if (buffer[0] != '#') {
                if (buffer[0] == 'x') {
                    is_rle = 1;
                    char *end1, *end2;
                    width2 = strtol(buffer, &end1, 10);  // 実は使っていない
                    height2 = strtol(end1, &end2, 10);   // 実は使っていない
                } else if (is_rle) {
                    for (int index = 0; buffer[index] != '\0'; ++index) {
                        if (buffer[index] == '$') {
                            printf("change line\n");
                            current_y += 1;
                            current_x = 0;
                        } else if (buffer[index] == '!') {
                            printf("end init\n");
                            break;
                        } else if (num_flag) {
                            num_flag = 0;
                            int i = 0;
                            while (is_num(buffer[index + i])) {
                                ++i;
                            }
                            // whileを抜けたあと、index番目から(index + i - 1)番目までは数字であることがわかっている
                            if (i == 0) {
                                run_length = 1;
                            } else {
                                char run_length_str[i];
                                for (int j = 0; j < i; ++j) {
                                    run_length_str[j] = buffer[index + j];
                                    printf("%c", run_length_str[j]);
                                }
                                printf("\n");
                                char* end;
                                run_length = strtol(run_length_str, &end, 10);
                            }
                            index += i - 1;
                        } else {
                            if (is_num(buffer[index])) {
                                fprintf(stderr, "error!! 実装見直せ！！\n");
                            } else {
                                num_flag = 1;
                                if (buffer[index] == 'o') {
                                    printf("current_x = %d, o, run_length = %d\n", current_x, run_length);
                                    for (int i = 0; i < run_length; ++i) {
                                        cell[current_y][current_x + i] = 1;
                                    }
                                    current_x += run_length;
                                } else if (buffer[index] == 'b') {
                                    printf("current_x = %d, b, run_length = %d\n", current_x, run_length);
                                    current_x += run_length;
                                }
                                run_length = 0;
                            }
                        }
                    }
                } else {
                    char *end1, *end2;
                    init_x[n] = strtol(buffer, &end1, 10);
                    init_y[n] = strtol(end1, &end2, 10);
                    ++n;
                }
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

void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width])
{
    int count = 0;
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
                ++count;
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
    fprintf(fp, "generation = %d, ratio = %f\n", gen, (float)count / (height * width));
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
