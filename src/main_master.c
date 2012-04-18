#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include <util/delay.h>

#include <1wire/1wire.h>
#include <avrcryptolib/sha256.h>

#define WIREPIN PB0

#define STATE_IDLE 0x00
#define STATE_CHALLENGE 0x01
#define STATE_RESPONSE 0x02

uint8_t secret[32];
uint8_t challenge[32];
uint8_t response[32];

void challenge_response_cycle();

SIGNAL(SIG_INTERRUPT0) {
  owi_init(1<<WIREPIN);
  challenge_response_cycle();
}

int main(void) {
  int i;
  for (i=0; i<4; i++) {
    secret[i] = 0xaf;
  }
}

void challenge_response_cycle() {
  owi_detectpresence(1<<WIREPIN);
  // TODO check return value of owi_detectpresence

  // generate random challenge
  int i,j;
  for (i=0; i<8; i++) {
    unsigned long random_single = random();
    for (j=(i*4); j<(i*4+4); j++) {
      challenge[j] = random_single & 0xff;
      random_single >>= 8;
    }
  }

  // send challenge
  for (i=0; i<32; i++) {
    owi_sendbyte(challenge[i], (1<<WIREPIN));
  }

  // wait for the slave to process the challenge
  // TODO count instructions on slave
  _delay_us(20);

  // receive response
  for (i=0; i<32; i++) {
    response[i] = owi_receivebyte(1<<WIREPIN);
  }

  // TODO check response
  // like this: if $response = $(sha256($secret & $challenge [& $random_slave])
  // $random_slave isn't actually needed, it's just plain paranoia
}
