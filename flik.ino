uint16_t pb; // 1 - x and y

uint8_t mw; // 2 - x deflection
uint8_t breath; // 2 - y deflection

bool sustain; // 3 - x deflection
uint8_t expression; // 3 - y deflection

void setup() {
  pb = 0x2000;
  mw = 0;
  expression = 0;
  sustain = false;
  breath = 0;
  
  Serial.begin(31250);
}

//10-bit ADC to 7-bit CC value. Measure deflection from center value.
inline uint8_t get_deflection(uint16_t val) {
  val >>= 2;
  if( !(val & B10000000) )
    val = 127 - val;
  return (uint8_t) val & (B1111111);
}

// ALL SENT ON CHANNEL 1
inline void send_cc(uint8_t cc_type, uint8_t cc_val) {
  byte mw_message[3] = { B10110000, cc_type, cc_val };
  Serial.write(mw_message, sizeof(mw_message));
}

void loop() {
  uint16_t x1 = analogRead(A0);
  uint16_t y1 = analogRead(A1);
  uint16_t x2 = analogRead(A2);
  uint16_t y2 = analogRead(A3);
  uint16_t x3 = analogRead(A4);
  uint16_t y3 = analogRead(A5);

  uint16_t x1_distance = abs((int16_t)0x200 - (int16_t)x1);
  uint16_t y1_distance = abs((int16_t)0x200 - (int16_t)y1);
  
  uint16_t new_pb = (x1_distance > y1_distance ? x1 : y1 ) << 4; //10-bit ADC to 14-bit pitch bend value

  uint8_t new_mw = get_deflection(x2);
  uint8_t new_breath = get_deflection(y2);

  bool new_sustain = x3 < 256 || x3 > 768;
  uint8_t new_expression = get_deflection(y3);
    
  if( new_pb != pb ) {
    pb = new_pb;
        
    // Pitch bend, channel 1 - LSB - MSB
    byte pb_message[3] = { B11100000, (byte)(pb & B01111111), (byte)((pb >> 7) & B01111111) };
    Serial.write(pb_message, sizeof(pb_message));
  }

  if( new_mw != mw ) {
    mw = new_mw;
    send_cc(1, mw);
  }

  if( new_expression != expression ){
    expression = new_expression;
    send_cc(11, expression);
  }

  if( new_sustain != sustain ) {
    sustain = new_sustain;
    send_cc(64, sustain ? 127 : 0 );
  }

  if( new_breath != breath ) {
    breath = new_breath;
    send_cc(2, breath);
  }
}
