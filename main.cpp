#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

// Global Constants for Grid System
const int WIDTH = 800;
const int HEIGHT = 600;
const int BLOCK_SIZE = 20;

// ==========================================
// 1. ABSTRACTION & INHERITANCE: Food Classes
// ==========================================
class Food {
protected:
    sf::Vector2f position;
    sf::CircleShape shape;

public:
    Food() {
        shape.setRadius(BLOCK_SIZE / 2.0f);
        respawn();
    }
    virtual ~Food() {}

    void respawn() {
        int columns = WIDTH / BLOCK_SIZE;
        int rows = HEIGHT / BLOCK_SIZE;
        position.x = static_cast<float>((rand() % columns) * BLOCK_SIZE);
        position.y = static_cast<float>((rand() % rows) * BLOCK_SIZE);
        shape.setPosition(position);
    }

    sf::Vector2f getPosition() const { return position; }

    virtual int getScore() const = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};

// Derived Class 1: Normal Food
class NormalFood : public Food {
public:
    NormalFood() : Food() { shape.setFillColor(sf::Color::Green); }

    int getScore() const override { return 10; }
    void render(sf::RenderWindow& window) override { window.draw(shape); }
};

// Derived Class 2: Bonus Food
class BonusFood : public Food {
public:
    BonusFood() : Food() { shape.setFillColor(sf::Color::Yellow); }

    int getScore() const override { return 20; }
    void render(sf::RenderWindow& window) override { window.draw(shape); }
};

// Derived Class 3: Poison Food
class PoisonFood : public Food {
public:
    PoisonFood() : Food() { shape.setFillColor(sf::Color::Red); }

    int getScore() const override { return -10; }
    void render(sf::RenderWindow& window) override { window.draw(shape); }
};


// ==========================================
// 2. ENCAPSULATION: Snake Class
// ==========================================
class Snake {
private:
    std::vector<sf::Vector2f> body;
    sf::Vector2f direction;
    sf::RectangleShape block;

public:
    Snake() {
        block.setSize(sf::Vector2f(BLOCK_SIZE - 2, BLOCK_SIZE - 2));
        block.setFillColor(sf::Color::Cyan);

        body.push_back(sf::Vector2f(400.f, 300.f));
        body.push_back(sf::Vector2f(400.f, 320.f));
        body.push_back(sf::Vector2f(400.f, 340.f));

        direction = sf::Vector2f(0.f, static_cast<float>(-BLOCK_SIZE));
    }

    void setDirection(sf::Vector2f newDir) {
        if (direction.x + newDir.x != 0 || direction.y + newDir.y != 0) {
            direction = newDir;
        }
    }

    void move() {
        for (size_t i = body.size() - 1; i > 0; --i) {
            body[i] = body[i - 1];
        }
        body[0] += direction;
    }

    void grow() {
        body.push_back(body.back());
    }

    void shrink() {
        if (body.size() > 1) {
            body.pop_back();
        }
    }

    sf::Vector2f getHeadPosition() const { return body[0]; }

    bool checkSelfCollision() const {
        for (size_t i = 1; i < body.size(); ++i) {
            if (body[0] == body[i]) return true;
        }
        return false;
    }

    bool checkWallCollision() const {
        sf::Vector2f head = body[0];
        return (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT);
    }

    void render(sf::RenderWindow& window) {
        for (size_t i = 0; i < body.size(); ++i) {
            if (i == 0) block.setFillColor(sf::Color::Blue);
            else block.setFillColor(sf::Color::Cyan);

            block.setPosition(body[i]);
            window.draw(block);
        }
    }
};


// ==========================================
// 3. ENGINE: Game Manager Class
// ==========================================
class Game {
private:
    sf::RenderWindow window;
    Snake snake;
    Food* currentFood;
    int score;
    int highScore;
    bool isGameOver;
    float currentSpeed;

    // Font must be declared BEFORE Text objects so it initializes first
    sf::Font font;
    sf::Text scoreText;
    sf::Text gameOverText;

    void loadHighScore() {
        std::ifstream file("highscore.txt");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
        }
    }

    void saveHighScore() {
        std::ofstream file("highscore.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }

    void spawnRandomFood() {
        delete currentFood;
        int choice = rand() % 10;
        if (choice < 6) {
            currentFood = new NormalFood();
        } else if (choice < 8) {
            currentFood = new BonusFood();
        } else {
            currentFood = new PoisonFood();
        }
    }

    void updateDifficulty() {
        if (score < 50) currentSpeed = 0.15f;
        else if (score < 150) currentSpeed = 0.10f;
        else currentSpeed = 0.06f;
    }

public:
    // FIX 1: sf::VideoMode now requires sf::Vector2u, not two separate ints
    // FIX 2: sf::Text has no default constructor in SFML 3 — must pass font in initializer list
    Game() :
        window(sf::VideoMode({static_cast<unsigned int>(WIDTH), static_cast<unsigned int>(HEIGHT)}), "Smart Snake Game (OOP)"),
        score(0),
        highScore(0),
        isGameOver(false),
        currentFood(nullptr),
        scoreText(font),      // Pass font directly — required by SFML 3
        gameOverText(font)    // Pass font directly — required by SFML 3
    {
        srand(static_cast<unsigned>(time(0)));
        loadHighScore();
        spawnRandomFood();
        updateDifficulty();

        // FIX 3: SFML 3 uses openFromFile instead of loadFromFile
        if (!font.openFromFile("arial.ttf")) {
            std::cout << "Error loading font! Ensure arial.ttf is in the project directory.\n";
        }

        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::White);
        // FIX 4: setPosition now requires sf::Vector2f, not two separate floats
        scoreText.setPosition({10.f, 10.f});

        gameOverText.setCharacterSize(40);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("GAME OVER\nPress R to Restart");
        // FIX 4 (continued): wrap coordinates in braces to form sf::Vector2f
        gameOverText.setPosition({WIDTH / 2.0f - 150.f, HEIGHT / 2.0f - 50.f});
    }

    ~Game() {
        delete currentFood;
    }

    void run() {
        sf::Clock clock;
        float timer = 0.0f;

        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            timer += dt;

            processEvents();

            if (!isGameOver) {
                if (timer >= currentSpeed) {
                    update();
                    timer = 0;
                }
            }

            render();
        }
    }

private:
    void processEvents() {
        // FIX 5: SFML 3 event loop returns std::optional — no sf::Event default constructor
        while (const std::optional<sf::Event> event = window.pollEvent()) {

            // FIX 6: Use event->is<>() instead of event.type == sf::Event::Closed
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // FIX 6 (continued): Use event->getIf<>() to access typed event data
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (!isGameOver) {
                    // FIX 7: Keys are now sf::Keyboard::Key::W, not sf::Keyboard::W
                    if (keyPressed->code == sf::Keyboard::Key::W || keyPressed->code == sf::Keyboard::Key::Up)
                        snake.setDirection(sf::Vector2f(0.f, static_cast<float>(-BLOCK_SIZE)));
                    if (keyPressed->code == sf::Keyboard::Key::S || keyPressed->code == sf::Keyboard::Key::Down)
                        snake.setDirection(sf::Vector2f(0.f, static_cast<float>(BLOCK_SIZE)));
                    if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left)
                        snake.setDirection(sf::Vector2f(static_cast<float>(-BLOCK_SIZE), 0.f));
                    if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right)
                        snake.setDirection(sf::Vector2f(static_cast<float>(BLOCK_SIZE), 0.f));
                } else {
                    if (keyPressed->code == sf::Keyboard::Key::R) {
                        snake = Snake();
                        score = 0;
                        isGameOver = false;
                        spawnRandomFood();
                        updateDifficulty();
                    }
                }
            }
        }
    }

    void update() {
        snake.move();

        if (snake.checkWallCollision() || snake.checkSelfCollision()) {
            isGameOver = true;
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
            return;
        }

        if (snake.getHeadPosition() == currentFood->getPosition()) {
            int gainedPoints = currentFood->getScore();
            score += gainedPoints;

            if (gainedPoints > 0) {
                snake.grow();
            } else {
                snake.shrink();
            }

            if (score < 0) score = 0;

            updateDifficulty();
            spawnRandomFood();
        }
    }

    void render() {
        window.clear(sf::Color::Black);

        if (!isGameOver) {
            snake.render(window);
            currentFood->render(window);
        } else {
            window.draw(gameOverText);
        }

        scoreText.setString("Score: " + std::to_string(score) + "   High Score: " + std::to_string(highScore));
        window.draw(scoreText);

        window.display();
    }
};


// ==========================================
// 4. MAIN FUNCTION Entry point
// ==========================================
int main() {
    Game game;
    game.run();
    return 0;
}