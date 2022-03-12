#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

constexpr int WIDTH = 640, HEIGHT = 360;
constexpr long double ASPECT_RATIO = WIDTH / HEIGHT;
constexpr int INFINITY = 4;
constexpr long double ZOOM_FACTOR = 1.5;
constexpr long double ITERATION_DELTA = 8;
constexpr long double START_X = -0.938258087226625480867497203219,
                      START_Y = 0.261313681594769599639011686820;
// constexpr long double
//     START_X =
//         -1.9997740601362903593126807559602500475710416233856384007148508574291012335984591928248364190215796259575718318799960175396106897245889581254834492701372949636783094955897931317174101259095891469501748126725148714587333938548443819033709904187344921523413310221887295870857771431011674873342592895504186325482220668710775749899926429101099841583206278295793058921625817004481783699245865364627140554117737774937789463895102748671351750212506004241754983473339789940659968568850689353099462034492524909310777724611601104714214019347435268544619054369865904944457792527241696528695821059623303046651934176389789308453627525109367436309636375268231073110318555064708363221007235298404379856922536028913291478442839193381367508575286692330907891402483843152933153748354825108021776358693600801782904774626935265722056455978643513448489091026679036353407968495795003386248005939867069799946547181378474054113117046900560609110812439442002663909295191705374444149326937073460052706389967886211172676612720028299452788285465688867116337489531157494508508315428488520037968118008255840569742557333862639124341116894229885253643651920014148109308402199399127712572209466874971603743536096235390414412927589954662603878558182262865151900604451937214289079939337905846647369517138325441736853526711818853134657265043099539402286244220638999824999819000131999789999857999958,
//     START_Y =
//         -0.0000000032900403214794350534969786759266805967852946505878410088326046927853549452991056352681196631150325234171525664335353457621247922992470898021063583060218954321140472066153878996044171428801408137278072521468882260382336298800961530905692393992277070012433445706657829475924367459793505729004118759963065667029896464160298608486277109065108339157276150465318584383757554775431988245033409975361804443001325241206485033571912765723551757793318752425925728969073157628495924710926832527350298951594826689051400340011140584507852761857568007670527511272585460136585523090533629795012272916453744029579624949223464015705500594059847850617137983380334184205468184810116554041390142120676993959768153409797953194054452153167317775439590270326683890021272963306430827680201998682699627962109145863135950941097962048870017412568065614566213639455841624790306469846132055305041523313740204187090956921716703959797752042569621665723251356946610646735381744551743865516477084313729738832141633286400726001116308041460406558452004662264165125100793429491308397667995852591271957435535504083325331161340230101590756539955554407081416407239097101967362512942992702550533040602039494984081681370518238283847808934080198642728761205332894028474812918370467949299531287492728394399650466260849557177609714181271299409118059191938687461000000000000000000000000000000000000;
constexpr int FRAME_RATE = 30;

int MAX_ITERATION = 128;
long double min_re = -2, max_re = 2;
long double min_im = -1, max_im = 1;

template <typename T>
inline T squared(const T& x) {
  return x * x;
}

int main() {
  std::unique_ptr<sf::RenderWindow> window(
      new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set"));

  //  frame limit
  window->setFramerateLimit(FRAME_RATE);

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

    if (z > 1)
      MAX_ITERATION += ITERATION_DELTA;
    else
      MAX_ITERATION -= ITERATION_DELTA;

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
  //  computes (1 - t) * u + t * v
  //  where t should be a value between 0 and 1
  auto lerp = [](const sf::Color& u, const sf::Color& v,
                 const long double& a) -> sf::Color {
    const long double b = 1 - a;
    return sf::Color(b * u.r + a * v.r, b * u.g + a * v.g, b * u.b + a * v.b);
  };

  auto map_range = [](const long double& old_val, const long double& old_min,
                      const long double& old_max, const long double& new_min,
                      const long double& new_max) {
    long double old_range = old_max - old_min;
    long double new_range = new_max - new_min;

    long double new_val =
        (((old_val - old_min) * new_range) / old_range) + new_min;

    return new_val;
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
        //  mapping the viewport to the domain
        const long double re_0 =
            map_range(x, 0, WIDTH, min_re, max_re) + START_X;
        const long double im_0 =
            map_range(y, 0, HEIGHT, min_im, max_im) + START_Y;

        long double re = 0, im = 0;
        int iteration = 0;
        while (iteration++ < MAX_ITERATION &&
               squared(re) + squared(im) < squared(INFINITY)) {
          long double curr_re = re;
          //  z = z^2 + c
          re = squared(re) - squared(im) + re_0;
          im = 2 * curr_re * im + im_0;
        }

        // color pallet similar to Ultra Fractal and Wikipedia
        static const std::vector<sf::Color> colors{
            {0, 7, 100},   {32, 107, 203}, {237, 255, 255},
            {255, 170, 0}, {0, 2, 0},
        };
        // static const std::vector<sf::Color> colors{
        //     {0, 0, 0},     {213, 67, 31}, {251, 255, 121},
        //     {62, 223, 89}, {43, 30, 218}, {0, 255, 247}};

        static const auto max_color = colors.size() - 1;

        // if (iteration == MAX_ITERATION) iteration = 0;

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

    // image.saveToFile("./out/mandelbrot" + std::to_string(++cnt) + ".png");

    zoom(WIDTH / 2, HEIGHT / 2, ZOOM_FACTOR);

    window->display();
  }

  return 0;
}