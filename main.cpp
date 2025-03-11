#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>  // Untuk usleep()
#include <termios.h> // Untuk non-blocking input

using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;
int score = 0;

class GameObject {
public:
    int x, y;
    char symbol;

    GameObject(int x, int y, char symbol) : x(x), y(y), symbol(symbol) {}
};

class Player : public GameObject {
public:
    Player(int x, int y) : GameObject(x, y, '^') {}

    void moveLeft() {
        if (x > 1) x--;
    }

    void moveRight() {
        if (x < WIDTH - 2) x++;
    }
};

class Bullet : public GameObject {
public:
    Bullet(int x, int y) : GameObject(x, y, '|') {}

    bool moveUp() {
        y--;
        return (y >= 0);
    }
};

class Enemy : public GameObject {
public:
    int hp;

    Enemy(int x, int y, int hp) : GameObject(x, y, 'V'), hp(hp) {}

    bool moveDown() {
        y++;
        return (y < HEIGHT);
    }

    void takeDamage() {
        hp--;
    }

    bool isDestroyed() {
        return hp <= 0;
    }
};

int getKeyPress() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void drawScreen(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies) {
    system("clear");
    cout << "SCORE: " << score << endl;

    cout << "+";
    for (int i = 0; i < WIDTH - 2; i++) cout << "-";
    cout << "+\n";

    for (int y = 0; y < HEIGHT - 1; y++) {
        cout << "|";
        bool hpDisplayed = false;

        for (int x = 0; x < WIDTH - 2; x++) {
            bool drawn = false;

            for (auto& bullet : bullets) {
                if (bullet.x == x + 1 && bullet.y == y) {
                    cout << bullet.symbol;
                    drawn = true;
                    break;
                }
            }

            for (auto& enemy : enemies) {
                if (enemy.x == x + 1 && enemy.y == y) {
                    cout << enemy.symbol;
                    drawn = true;

                    // Tampilkan HP musuh di sisi kiri
                    if (!hpDisplayed) {
                        cout << "  HP: " << enemy.hp;
                        hpDisplayed = true;
                    }
                    break;
                }
            }

            if (!drawn) cout << " ";
        }
        cout << "|\n";
    }

    cout << "+";
    for (int i = 0; i < WIDTH - 2; i++) cout << "-";
    cout << "+\n";

    cout << string(player.x, ' ') << player.symbol << endl;
}

void gameLoop() {
    Player player(WIDTH / 2, HEIGHT - 1);
    vector<Bullet> bullets;
    vector<Enemy> enemies;

    int enemySpawnCounter = 0;
    bool running = true;

    while (running) {
        if (enemySpawnCounter++ > 10) {
            enemies.push_back(Enemy(rand() % (WIDTH - 2) + 1, 0, 3)); // Musuh dengan 3 HP
            enemySpawnCounter = 0;
        }

        if (isatty(STDIN_FILENO)) {
            int ch = getKeyPress();
            switch (ch) {
                case 'a': player.moveLeft(); break;
                case 'd': player.moveRight(); break;
                case ' ': bullets.push_back(Bullet(player.x, player.y - 1)); break;
                case 'q': running = false; break;
            }
        }

        for (size_t i = 0; i < bullets.size(); i++) {
            if (!bullets[i].moveUp()) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        for (size_t i = 0; i < enemies.size(); i++) {
            if (!enemies[i].moveDown()) {
                enemies.erase(enemies.begin() + i);
                i--;
            }
        }

        for (size_t i = 0; i < bullets.size(); i++) {
            for (size_t j = 0; j < enemies.size(); j++) {
                if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    enemies[j].takeDamage();
                    bullets.erase(bullets.begin() + i);
                    i--;

                    if (enemies[j].isDestroyed()) {
                        enemies.erase(enemies.begin() + j);
                        score += 10;
                        j--;
                    }
                    break;
                }
            }
        }

        drawScreen(player, bullets, enemies);
        usleep(50000);
    }
}

int main() {
    srand(time(0));
    gameLoop();
    return 0;
}
