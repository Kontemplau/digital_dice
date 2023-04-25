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

template<class Rng>
class Game {
    int counter = 0;
    int subtractor = 10;
    unsigned output = 0;
    unsigned prev_face = 6;
    bool previous = true;

    Rng &rng;
  public:
    Game(Rng &rng) : rng(rng) {}
    /// Update the game state.
    /// Returns the pattern to display.
    unsigned update(bool button) {
      if (button && !previous) {
        // Button wurde gedrückt
        counter = 250;
      }
      previous = button;
      if (counter > 0) {
        counter -= 1;
        if (counter % subtractor == 0) {
          output = FACES[rng() % 6];
          while (output == prev_face) {
            output = FACES[rng() % 6];
          }
          prev_face = output;
          subtractor += 5;

        } else if (counter % subtractor == (subtractor * 2) / 3) {
          output = 0;
        }
      } else {
        subtractor = 10;
      }
      return output;
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
