#include <EEPROM.h>

int LEDS[] = {A3, A4, 6, A0, 5, 7, A5, 4, 2};
int BUTTON = 3;

unsigned FACES[] = {
  0b000010000, 0b100000001, 0b100010001,
  0b101000101, 0b101010101, 0b101101101
};

void display(unsigned pattern) {
  unsigned mask = 0b100000000;
  for (int i = 0; i < 9; ++i) {
    digitalWrite(LEDS[i], (pattern & mask) ? HIGH : LOW);
    mask >>= 1;
  }
}

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 9; ++i) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
  pinMode(BUTTON, INPUT_PULLUP);
}

class Animation {
    int light_counter = 0;
    int dark_counter = 0;
    int duration;

  public:
    static constexpr unsigned DARK = 0b00;
    static constexpr unsigned LIGHT = 0b01;
    static constexpr unsigned NEW_FACE = 0b10;
    unsigned animation() {
      if (dark_counter > 0) {
        dark_counter -= 1;
        if (dark_counter == 0) {
          duration += 5;
          if (duration > 50) duration = 0;
          light_counter = duration;
          return LIGHT + NEW_FACE;
        }
        return DARK;
      }
      if (light_counter > 0) {
        light_counter -= 1;
        if (light_counter == 0) {
          dark_counter = duration;
          return DARK;
        }
        return LIGHT;
      }
      return LIGHT;
    }
    void start() {
      dark_counter = 5;
      duration = 5;
    }
};

template<class Rng>
class Game {
    unsigned face = 0;
    bool previous = true;
    Animation a;
    Rng &rng;

  public:
    Game(Rng &rng) : rng(rng) {}
    /// Update the game state.
    /// Returns the pattern to display.
    unsigned update(bool button) {
      if (button && !previous) {
        // Button wurde gedrückt
        a.start();
      }
      previous = button;

      unsigned x = a.animation();

      if (x & a.NEW_FACE) {
        unsigned new_face = FACES[rng() % 6];
        while (face == new_face) {
          new_face = FACES[rng() % 6];
        }
        face = new_face;
      }
      return (x & a.LIGHT) ? face : 0;
    }
};

class Xorshift {
    unsigned long x32;
  public:
    Xorshift() {
      EEPROM.get(0, x32);
      x32 += 1;
      EEPROM.put(0, x32);
    }
    unsigned long operator()() {
      x32 ^= x32 << 13;
      x32 ^= x32 >> 17;
      x32 ^= x32 << 5;
      return x32;
    }
};

Xorshift x;
Game<decltype(x)> g(x);

void loop() {
  // put your main code here, to run repeatedly:
  display(g.update(digitalRead(BUTTON) == LOW));
  delay(10);
}
