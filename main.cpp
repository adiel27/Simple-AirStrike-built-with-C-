#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>  // Untuk usleep()
#include <termios.h> // Untuk non-blocking input

using namespace std;

// Ukuran layar permainan
const int WIDTH = 40;
const int HEIGHT = 20;

// Variabel skor
int score = 0;

// Kelas untuk objek permainan
class GameObject {
public:
    int x, y;
    char symbol;

    GameObject(int x, int y, char symbol) : x(x), y(y), symbol(symbol) {}
};

// Kelas untuk pesawat pemain
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

// Kelas untuk peluru
class Bullet : public GameObject {
public:
    Bullet(int x, int y) : GameObject(x, y, '|') {}

    bool moveUp() {
        y--;
        return (y >= 0);
    }
};

// Kelas untuk musuh
class Enemy : public GameObject {
public:
    Enemy(int x, int y) : GameObject(x, y, 'V') {}

    bool moveDown() {
        y++;
        return (y < HEIGHT);
    }
};

// Fungsi untuk menangani input tanpa enter
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

// Fungsi untuk menggambar layar permainan
void drawScreen(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies) {
    system("clear"); // Bersihkan layar

    // Tampilkan skor
    cout << "SCORE: " << score << endl;

    // Gambar batas atas
    cout << "+";
    for (int i = 0; i < WIDTH - 2; i++) cout << "-";
    cout << "+\n";

    // Gambar area permainan
    for (int y = 0; y < HEIGHT - 1; y++) {
        cout << "|";
        for (int x = 0; x < WIDTH - 2; x++) {
            bool drawn = false;

            // Gambar peluru
            for (auto& bullet : bullets) {
                if (bullet.x == x + 1 && bullet.y == y) {
                    cout << bullet.symbol;
                    drawn = true;
                    break;
                }
            }

            // Gambar musuh
            for (auto& enemy : enemies) {
                if (enemy.x == x + 1 && enemy.y == y) {
                    cout << enemy.symbol;
                    drawn = true;
                    break;
                }
            }

            if (!drawn) cout << " ";
        }
        cout << "|\n";
    }

    // Gambar batas bawah
    cout << "+";
    for (int i = 0; i < WIDTH - 2; i++) cout << "-";
    cout << "+\n";

    // Gambar pesawat pemain
    cout << string(player.x, ' ') << player.symbol << endl;
}

// Fungsi utama permainan
void gameLoop() {
    Player player(WIDTH / 2, HEIGHT - 1);
    vector<Bullet> bullets;
    vector<Enemy> enemies;

    int enemySpawnCounter = 0;
    bool running = true;

    while (running) {
        // Spawn musuh setiap beberapa loop
        if (enemySpawnCounter++ > 10) {
            enemies.push_back(Enemy(rand() % (WIDTH - 2) + 1, 0));
            enemySpawnCounter = 0;
        }

        // Input dari keyboard
        if (isatty(STDIN_FILENO)) { // Cek apakah ada input
            int ch = getKeyPress();
            switch (ch) {
                case 'a': player.moveLeft(); break;
                case 'd': player.moveRight(); break;
                case ' ': bullets.push_back(Bullet(player.x, player.y - 1)); break;
                case 'q': running = false; break;
            }
        }

        // Gerakkan peluru
        for (size_t i = 0; i < bullets.size(); i++) {
            if (!bullets[i].moveUp()) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        // Gerakkan musuh
        for (size_t i = 0; i < enemies.size(); i++) {
            if (!enemies[i].moveDown()) {
                enemies.erase(enemies.begin() + i);
                i--;
            }
        }

        // Cek tabrakan antara peluru dan musuh
        for (size_t i = 0; i < bullets.size(); i++) {
            for (size_t j = 0; j < enemies.size(); j++) {
                if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    score += 10; // Tambah skor jika musuh terkena peluru
                    i--; j--;
                    break;
                }
            }
        }

        // Gambar ulang layar
        drawScreen(player, bullets, enemies);
        usleep(50000); // Tunggu 50 milidetik
    }
}

int main() {
    srand(time(0));
    gameLoop();
    return 0;
}
