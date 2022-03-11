#include <SFML/Graphics.hpp>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

constexpr int WIDTH = 640, HEIGHT = 360;
constexpr long double ASPECT_RATIO = WIDTH / HEIGHT;
constexpr int INFINITY = 128;
constexpr long double ZOOM_FACTOR = 1.00000000000000000001;

int MAX_ITERATION = 128;
long double min_re = -2.5, max_re = 1;
long double min_im = -1, max_im = 1;

template <typename T>
inline T squared(const T& x) {
  return x * x;
}

int main() {
  std::unique_ptr<sf::RenderWindow> window(
      new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set"));

  //  frame limit
  window->setFramerateLimit(30);

  //  center the window
  window->setPosition(sf::Vector2i(
      (sf::VideoMode::getDesktopMode().width - window->getSize().x) * 0.5,
      (sf::VideoMode::getDesktopMode().height - window->getSize().y) * 0.5));

  sf::Image image;
  image.create(WIDTH, HEIGHT);

  sf::Texture texture;
  sf::Sprite sprite;

  auto zoom = [&](const long double& pos_x, const long double& pos_y,
                  const long double& z) {
    //  changing the center to the mouse click point
    long double re_0 = min_re + (max_re - min_re) * pos_x / WIDTH;
    long double im_0 = min_im + (max_im - min_im) * pos_y / HEIGHT;

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

  //  linear interpolation
  //  computes v + t(u - v)
  //  where t should be a value between 0 and 1
  auto lerp = [](const sf::Color& v, const sf::Color& u,
                 const long double& a) -> sf::Color {
    const long double b = 1 - a;
    return sf::Color(b * v.r + a * u.r, b * v.g + a * u.g, b * v.b + a * u.b);
  };

  long long cnt = 0;
  while (window->isOpen()) {
    sf::Event event;
    while (window->pollEvent(event)) {
      if (event.type == sf::Event::Closed) window->close();

      if (event.type == sf::Event::KeyPressed) {
        //  move delta
        long double x_delta = (max_re - min_re) * ASPECT_RATIO * 0.3;
        long double y_delta = (max_im - min_im) * (1.0 / ASPECT_RATIO) * 0.3;

        if (event.key.code == sf::Keyboard::Left ||
            event.key.code == sf::Keyboard::A) {
          min_re -= x_delta, max_re -= x_delta;
        } else if (event.key.code == sf::Keyboard::Right ||
                   event.key.code == sf::Keyboard::D) {
          min_re += x_delta, max_re += x_delta;
        } else if (event.key.code == sf::Keyboard::Up ||
                   event.key.code == sf::Keyboard::W) {
          min_im -= y_delta, max_im -= y_delta;
        } else if (event.key.code == sf::Keyboard::Down ||
                   event.key.code == sf::Keyboard::S) {
          min_im += y_delta, max_im += y_delta;
        }
      }

      if (event.type == sf::Event::MouseButtonPressed) {
        //  left click to zoom in
        if (event.mouseButton.button == sf::Mouse::Left) {
          zoom(event.mouseButton.x, event.mouseButton.y, ZOOM_FACTOR);
        }
        //  right click to zoom out
        else if (event.mouseButton.button == sf::Mouse::Right) {
          zoom(event.mouseButton.x, event.mouseButton.y, 1.0 / ZOOM_FACTOR);
        }
      }

      //  set iteration level by mouse wheel
      if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.MouseWheelScrolled) {
          if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            if (event.mouseWheelScroll.delta > 0)
              MAX_ITERATION = std::max(1, (MAX_ITERATION >> 1));
            else
              MAX_ITERATION <<= 1;
          }
        }
      }
    }

    window->clear();

    //  adding parallelization
#pragma omp parallel for
    for (int y{}; y < HEIGHT; ++y) {
      for (int x{}; x < WIDTH; ++x) {
        //  making this as a range from 0 to window.DIM
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
          //  z = z^2 + c
          re = squared(re) - squared(im) + re_0;
          im = 2 * curr_re * im + im_0;

          if (squared(re) + squared(im) > INFINITY) break;
        }

        //  settting the color value in the range of 0 to 255
        // int r = 1.0 * (iteration - MAX_ITERATION) / MAX_ITERATION * 0xff;
        // int g = r, b = r;

        // color pallet similar to Ultra Fractal and Wikipedia
        static const std::vector<sf::Color> colors{
            {0, 7, 100},   {32, 107, 203}, {237, 255, 255},
            {255, 170, 0}, {0, 2, 0},
        };
        // static const std::vector<sf::Color> colors{
        //     {0, 0, 0},     {213, 67, 31}, {251, 255, 121},
        //     {62, 223, 89}, {43, 30, 218}, {0, 255, 247}};

        static const auto max_color = colors.size() - 1;

        if (iteration == MAX_ITERATION) iteration = 0;

        long double mu = 1.0 * iteration / MAX_ITERATION;
        mu *= max_color;
        size_t i_mu = static_cast<std::size_t>(mu);
        sf::Color col_1 = colors[i_mu];
        sf::Color col_2 = colors[std::min(i_mu + 1, max_color)];
        sf::Color col = lerp(col_1, col_2, mu - i_mu);

        image.setPixel(x, y, sf::Color(col));
      }
    }

    texture.loadFromImage(image);
    sprite.setTexture(texture);
    window->draw(sprite);

    image.saveToFile("./out/mandelbrot" + std::to_string(++cnt) + ".png");
    zoom(
        -1.999774060136290359312680755960250047571041623385638400714850857429101233598459192824836419021579625957571831879996017539610689724588958125483449270137294963678309495589793131717410125909589146950174812672514871458733393854844381903370990418734492152341331022188729587085777143101167487334259289550418632548222066871077574989992642910109984158320627829579305892162581700448178369924586536462714055411773777493778946389510274867,
        -0.0000000032900403214794350534969786759266805967852946505878410088326046927853549452991056352681196631150325234171525664335353457621247922992470898021063583060218954321140472066153878996044171428801408137278072521468882260382336298800961530905692393992277070012433445706657829475924367459793505729004118759963065667029896464160298608486277109065108339157276150465318584383757554775431988245033409975361804443001325241,
        ZOOM_FACTOR);

    std::printf("DONE\n");
    std::fflush(stdout);

    window->display();
  }

  return 0;
}