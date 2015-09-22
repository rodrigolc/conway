#include <mutex>
#include <condition_variable>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#include "game.hpp"

using namespace std;

int neighbours(sf::Color* board, int height, int width, int offset){
    auto x = offset / width;
    auto y = offset % width;
    int counter = 0;
    if(y > 0){
        if (board[offset-width] == sf::Color::White) counter++;
        if (x > 0 && board[offset-width-1] == sf::Color::White) counter++;
        if (x < (width-1) && board[offset-width+1] == sf::Color::White) counter++;
    }
    if(y < (height-1)){
        if (board[offset+width] == sf::Color::White) counter++;
        if (x > 0 && board[offset+width-1] == sf::Color::White) counter++;
        if (x < (width-1) && board[offset+width+1] == sf::Color::White) counter++;
    }
    if (x > 0 && board[offset-1] == sf::Color::White) counter++;
    if (x < (width-1) && board[offset+1] == sf::Color::White) counter++;

    return counter;
}

void worker_thread(Game* game, size_t id){

    //Set variables
    size_t num_threads = game->num_threads;

    int total = game->height * game->width;
    int div = total/num_threads;
    int min = div * id;
    int max = div * (id + 1);
    if(id == (num_threads - 1))
        max = total;

    //Setup and initial sync
    // cout << "ddthreads " << id << endl;
    game->draw_barrier.wait();
    while(true){
        game->process_barrier.wait();

            // cout << "dthread" << endl;
            if (! game->paused){

                    // cout << "dthreadp" << endl;
                    // cout << min << ' ' << max << endl;
                for (int i = min; i < max; i++)
                {
                    int nbrs = neighbours(game->board,game->height,game->width,i);
                    if(game->board[i] == sf::Color::White){
                        if(game->rules_life[nbrs])
                        {
                            game->aux_board[i] = sf::Color::White;
                        }else{
                            game->aux_board[i] = sf::Color::Black;
                        }
                    }else{
                        if(game->rules_death[nbrs])
                        {
                            game->aux_board[i] = sf::Color::White;
                        }else{
                            game->aux_board[i] = sf::Color::Black;
                        }
                    }
                }
            }

            game->draw_barrier.count_up();

        game->setup_barrier.wait();



            game->process_barrier.count_up();

        game->draw_barrier.wait();


            game->setup_barrier.count_up();
    }

}

void drawer_thread(Game* game){
    game->draw_barrier.wait();
    sf::Clock clock;
    sf::Clock game_clock;

    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    texture.create(game->width,game->height);
    sprite.setTexture(texture);
    while(true){
        // cout << "dthread" << endl;

        do{
            texture.update((sf::Uint8*)game->board);
            game->window.clear(sf::Color::Black);
            game->window.draw(sprite);
            sf::Time elapsed = clock.restart();
            int mili = elapsed.asMilliseconds();
            this_thread::sleep_for( (std::chrono::milliseconds(1000) / game->fps) - std::chrono::milliseconds(mili));

            game->window.display();

        }while(game->paused || std::chrono::milliseconds(game_clock.getElapsedTime().asMilliseconds()) < (std::chrono::milliseconds(1000) / game->ticks_per_second));
        game_clock.restart();
        game->process_barrier.wait();


            game->draw_barrier.count_up();

        game->setup_barrier.wait();

            sf::Color* b = game->board;
            game->board = game->aux_board;
            game->aux_board = b;


            game->process_barrier.count_up();

        game->draw_barrier.wait();

                    game->setup_barrier.count_up();

    }

}


Game::Game(int height, int width, size_t num_threads, array<int,9> rules_death, array<int,9> rules_life)
    :setup_barrier(num_threads + 1),
     draw_barrier(num_threads+1),
     process_barrier(num_threads+1),
     window(sf::VideoMode(height, width), "Game of Life",sf::Style::Titlebar|sf::Style::Close)
     {

    this->num_threads = num_threads;
    this->height = height;
    this->width = width;
    this->board = new sf::Color[height * width];
    this->aux_board = new sf::Color[height * width];
    this->rules_death = rules_death;
    this->rules_life = rules_life;



}

void Game::game_loop(){
    //create window
    this->paused = true;
    this->fps = 60;
    this->ticks_per_second = 60;
    this->window.clear(sf::Color::Black);
    this->window.display();
    this->window.setActive(false);
    sf::Event event = sf::Event();

    this->threads = new thread[num_threads];
    this->ticker = thread(drawer_thread,this);
    for (size_t i = 0; i < this->num_threads; i++) {
        this->threads[i] = thread(worker_thread,this,i);
    }

    int seed = 23231233;

    bool running = true;
    bool mouse_down = false;
    sf::Color mouse_color;
    sf::Clock clock;
    while(running){
        sf::Time elapsed = clock.restart();
        int mili = elapsed.asMilliseconds();
        this_thread::sleep_for( (std::chrono::milliseconds(1000)/this->fps) - std::chrono::milliseconds(mili));

        while (window.pollEvent(event))
        {

            if (event.type == sf::Event::Closed){
                cout << "closed" << endl;
                window.close();
                running = false;
            }
            if (event.type == sf::Event::Resized)
            {
                std::cout << "new width: " << event.size.width << std::endl;
                std::cout << "new height: " << event.size.height << std::endl;
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space)
                {
                    paused = !paused;
                    cout << "paused: " << paused << endl;
                }
                else if (event.key.code == sf::Keyboard::R)
                {
                    seed = seed * 2;
                    int h = seed%this->height;
                    for (size_t i = 0; i < this->width; i++) {
                        this->board[h*width + i] = sf::Color::White;
                    }
                }else if (event.key.code == sf::Keyboard::C)
                {
                    for (size_t i = 0; i < this->width * this->height; i++) {
                        this->board[i] = sf::Color::Black;
                    }
                }else if (event.key.code == sf::Keyboard::R)
                {
                    seed = seed * 2;
                    int h = seed%this->height;
                    for (size_t i = 0; i < this->width; i++) {
                        this->board[h*width + i] = sf::Color::White;
                    }
                }
                else if (event.key.code == sf::Keyboard::Num1)
                {
                    this->ticks_per_second = 1;
                }
                else if (event.key.code == sf::Keyboard::Num2)
                {
                    this->ticks_per_second = 2;
                }
                else if (event.key.code == sf::Keyboard::Num3)
                {
                    this->ticks_per_second = 5;
                }
                else if (event.key.code == sf::Keyboard::Num4)
                {
                    this->ticks_per_second = 10;
                }
                else if (event.key.code == sf::Keyboard::Num5)
                {
                    this->ticks_per_second = 15;
                }
                else if (event.key.code == sf::Keyboard::Num6)
                {
                    this->ticks_per_second = 20;
                }
                else if (event.key.code == sf::Keyboard::Num7)
                {
                    this->ticks_per_second = 30;
                }
                else if (event.key.code == sf::Keyboard::Num8)
                {
                    this->ticks_per_second = 60;
                }


            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    mouse_down = true;
                    auto size = this->window.getSize();
                    int x = event.mouseButton.x * this->width / size.x;
                    int y = event.mouseButton.y * this->height /size.y;
                    cout << "mouse pressed " << x << " " << y << endl;
                    if(this->board[y * width + x] == sf::Color::Black){
                        this->board[y * width + x] = sf::Color::White;
                        mouse_color = sf::Color::White;
                    }else{
                        this->board[y * width + x] = sf::Color::Black;
                        mouse_color = sf::Color::Black;
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {

                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    mouse_down = false;
                }
            }
            if (event.type == sf::Event::MouseMoved)
            {
                if (mouse_down == true)
                {

                    auto size = this->window.getSize();
                    int x = event.mouseMove.x * this->width / size.x;
                    int y = event.mouseMove.y * this->height /size.y;
                    cout << "mouse moved " << x << ' ' << y << endl;

                    this->board[y * width + x] = mouse_color;

                }
            }
        }
    }

}


int main(int argc, char* argv[]){
    Game game(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    cout << game.num_threads << " " << game.width << " " << game.height << endl;
    for (size_t i = 0; i < 10; i++) {
        cout << i << " " << (game.rules_death[i] == ((char)1)) << " " << (game.rules_life[i] == ((char)1)) << endl;
    }
    game.game_loop();
    return 0;
}
