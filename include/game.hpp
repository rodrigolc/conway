#include <thread>
#include <array>
#include "barrier.hpp"
#include "SFML/Graphics.hpp"
using namespace std;

class Game{

public:

    int height,width;
    sf::Color * board;
    sf::Color * aux_board;

    array<int,9> rules_death;
    array<int,9> rules_life;

    size_t num_threads;
    thread *threads;
    thread ticker;
    int fps;
    int ticks_per_second;

    barrier setup_barrier;
    barrier draw_barrier;
    barrier process_barrier;
    sf::RenderWindow window;
    bool paused;

    Game(int height, int width, size_t num_threads, array<int,9> rules_death, array<int,9> rules_life);
    Game(int height, int width, size_t num_threads) : Game::Game(height,width,num_threads,{{0,0,0,1,0,0,0,0,0}},{{0,0,1,1,0,0,0,0,0}}){};

    void start();
    void stop();

    void set_cell(int height, int width, char state);
    char get_cell(int height, int width);

    void game_loop();

};
