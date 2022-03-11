#include <stdio.h>

#include <SFML/Graphics.hpp>

constexpr int WIDTH = 620, HEIGHT = 360;
constexpr int MAX_ITERATION = 128;
constexpr int INFINITY = 4;
constexpr long double ZOOM_FACTOR = 2;

long double min_re = -2.5, max_re = 1;
long double min_im = -1, max_im = 1;

template <typename T>
inline T squared(const T& x) {
  return x * x;
}

int main() {
  sf::RenderWindow* window =
      new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

  //  center the window
  window->setPosition(sf::Vector2i(
      (sf::VideoMode::getDesktopMode().width - window->getSize().x) * 0.5,
      (sf::VideoMode::getDesktopMode().height - window->getSize().y) * 0.5));

  sf::Image image;
  image.create(WIDTH, HEIGHT);

  sf::Texture texture;
  sf::Sprite sprite;

  while (window->isOpen()) {
    sf::Event event;
    while (window->pollEvent(event)) {
      if (event.type == sf::Event::Closed) window->close();
      if (event.type == sf::Event::MouseButtonPressed) {
        auto zoom = [&](long double z) {
          //  changing the center to the mouse click point
          long double re_0 =
              min_re + (max_re - min_re) * event.mouseButton.x / WIDTH;
          long double im_0 =
              min_im + (max_im - min_im) * event.mouseButton.y / HEIGHT;

          //  zoom
          long double scaled_min_re = re_0 - (max_re - min_re) / 2.0 / z;
          long double scaled_max_re = re_0 + (max_re - min_re) / 2.0 / z;
          min_re = scaled_min_re;
          max_re = scaled_max_re;

          long double scaled_min_im = im_0 - (max_im - min_im) / 2.0 / z;
          long double scaled_max_im = im_0 + (max_im - min_im) / 2.0 / z;
          min_im = scaled_min_im;
          max_im = scaled_max_im;
        };
        //  left click to zoom in
        if (event.mouseButton.button == sf::Mouse::Left) {
          zoom(ZOOM_FACTOR);
        }
        //  right click to zoom out
        else if (event.mouseButton.button == sf::Mouse::Right) {
          zoom(1.0 / ZOOM_FACTOR);
        }
      }
    }

    window->clear();

    //  adding parallelization
#pragma omp parallel for
    for (int y{}; y < HEIGHT; ++y) {
      for (int x{}; x < WIDTH; ++x) {
        const long double re_0 =
            min_re +
            (max_re - min_re) * x / WIDTH;  //  x - axis as the real axis
        const long double im_0 =
            min_im +
            (max_im - min_im) * y / HEIGHT;  //  y - axis as the imaginary axis

        long double re = 0, im = 0;
        int iteration = 0;
        while (iteration++ < MAX_ITERATION) {
          long double curr_re = re;
          re = squared(re) - squared(im) + re_0;
          im = 2 * curr_re * im + im_0;

          if (squared(re) + squared(im) > INFINITY) break;
        }

        int r = 1.0 * (MAX_ITERATION - iteration) / MAX_ITERATION * 0xff;
        int g = r, b = r;

        image.setPixel(x, y, sf::Color(r, g, b));
      }
    }
    texture.loadFromImage(image);
    sprite.setTexture(texture);

    window->draw(sprite);

    std::printf("Done\n");
    std::fflush(stdout);
    window->display();
  }

  return 0;
}