#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <ncurses.h>

#define MAX_WORDS 100
#define MAX_WORD_SIZE 20
#define MAX_ATTEMPTS 6

char **load_words(const char* filename, int* words_count);
char *random_word(const char** words, int count);
int play_game(const char* word, int wins);
void draw_game(WINDOW *game, const char *display, int attempts, const char* wrong_letters, int wins);
void draw_hangman(WINDOW *game, int attempts);

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);

    int words_count = 0;
    char **words = load_words("palavras.txt", &words_count);
    if (words == NULL || words_count < 1) {
        return EXIT_FAILURE;
    }

    int exitGame = 0;
    int wins = 0;
    while (!exitGame) {
        char* word = random_word((const char**)words, words_count);
        int result = play_game(word, wins);

        if (result == 1) {
            wins++;
        } else if (result == 0) {
            wins = 0;
        } else if (result == -1) {
            exitGame = 1;
        }
    }

    endwin();

    return EXIT_SUCCESS;
}

char** load_words(const char* filename, int* words_count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao ler arquivo palavras.txt\n");
        return NULL;
    }

    char **words = malloc(MAX_WORDS *  sizeof(char*));
    if (words == NULL) {
        printf("Erro ao armazenar dados na memória.\n");
        return NULL;
    }

    char buffer[MAX_WORD_SIZE];
    while (fgets(buffer, MAX_WORD_SIZE, file)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) {
            continue;
        }

        words[*words_count] = malloc(strlen(buffer) + 1);
        strcpy(words[*words_count], buffer);
        (*words_count)++;
    }
    fclose(file);

    return words;
}

char *random_word(const char** words, int count) {
    return (char*)words[rand() % count];
}

int play_game(const char* word, int wins) {
    int len = strlen(word);
    char *display = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        display[i] = '_';
    }

    int attemtps = 0;
    int corrects = 0;
    char wrong_letters[27] = "";

    WINDOW *game = newwin(20, 70, 2, 0);
    keypad(game, TRUE);

    while (attemtps < MAX_ATTEMPTS && corrects < len) {
        draw_game(game, display, attemtps, wrong_letters, wins);

        mvwprintw(game, 18, 2, "Digite uma letra: ");
        wrefresh(game);

        char guess = wgetch(game);
        guess = toupper(guess);
        if (guess < 'A' || guess > 'Z') {
            continue;
        }

        int alreadyGuessed = FALSE;
        for (int i = 0; i < strlen(display); i++) {
            if (guess == display[i]) {
                alreadyGuessed = TRUE;
                break;
            }
        }

        for (int i = 0; i < strlen(wrong_letters); i++) {
            if (guess == wrong_letters[i]) {
                alreadyGuessed = TRUE;
                break;
            }
        }

        if (alreadyGuessed)
            continue;

        int hit = FALSE;
        for (int i = 0; i < len; i++) {
            if (guess == word[i]) {
                display[i] = toupper(word[i]);
                hit = TRUE;
                corrects++;
            }
        }

        if (hit) {
            beep();
        } else {
            int wrong_len = strlen(wrong_letters);
            wrong_letters[wrong_len] = guess;
            wrong_letters[len + 1] = '\0';
            attemtps++;
        }
    }

    int result = 0;
    if (corrects == len) {
        werase(game);
        box(game, 0, 0);
        char *win_message = "Parabéns! Você venceu!";
        mvwprintw(game, 9, 20, "%s", win_message);
        beep();
        beep();
        wrefresh(game);
        result = 1;
        napms(2000);
    } else {
        char *lose_message = "Você perdeu!";
        mvwprintw(game, 9, 20, "%s", lose_message);
        wrefresh(game);

        mvwprintw(game, 16, 2, "(R) Reiniciar e jogar novamente.");
        mvwprintw(game, 17, 2, "(S) Sair do jogo");

        int ch;
        do {
            ch = wgetch(game);
            ch = toupper(ch);
        } while (ch != 'R' && ch != 'S');

        result = (ch == 'R') ? 0 : -1;
    }

    delwin(game);
    free(display);

    return result;
}

void draw_game(
    WINDOW *game,
    const char *display,
    int attempts,
    const char* wrong_letters,
    int wins) {
    werase(game);
    box(game, 0, 0);

    mvwprintw(game, 1, 2, "Placar: %d", wins);
    mvwprintw(game, 2, 2, "Palavra: %s", display);
    mvwprintw(game, 3, 2, "Tentativas restantes: %d", MAX_ATTEMPTS - attempts);
    wattron(game, COLOR_PAIR(2));
    mvwprintw(game, 4, 2, "Letras erradas: %s", wrong_letters);
    wattroff(game, COLOR_PAIR(2));

    draw_hangman(game, attempts);

    wrefresh(game);
}
/*

         +----+
         |    |
         O    |
        /|\   |
        / \   |
              |
         =======
*/
void draw_hangman(WINDOW *game, int attempts) {
    int x = 50;
    int y = 3;

    wattron(game, COLOR_PAIR(3));
    mvwprintw(game, y, x,     "+----+");
    mvwprintw(game, y + 1, x, "|    |");
    mvwprintw(game, y + 2, x, "     |");
    mvwprintw(game, y + 3, x, "     |");
    mvwprintw(game, y + 4, x, "     |");
    mvwprintw(game, y + 5, x, "     |");
    mvwprintw(game, y + 6, x, "=======");
    wattroff(game, COLOR_PAIR(3));

    wattron(game, COLOR_PAIR(4));
    if (attempts > 0)
        mvwprintw(game, y + 2, x, "O");
    if (attempts > 1)
        mvwprintw(game, y + 3, x, "|");
    if (attempts > 2)
        mvwprintw(game, y + 3, x - 1, "/");
    if (attempts > 3)
        mvwprintw(game, y + 3, x + 1, "\\");
    if (attempts > 4)
        mvwprintw(game, y + 4, x - 1, "/");
    if (attempts > 5)
        mvwprintw(game, y + 4, x + 1, "\\");

    wattroff(game, COLOR_PAIR(4));
}