#include <Arduino.h>
#include <FlexiTimer2.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const uint8_t rxPin = 8; //변경 금지
const uint8_t txPin = 9; //변경 금지

// Set up a new SoftwareSerial object
SoftwareSerial SoftSerial(rxPin, txPin);

uint8_t Data_Receive_Flag;

void digital_toggle( uint8_t pin )
{
    digitalRead( pin ) ? digitalWrite( pin, 0 ) : digitalWrite( pin, 1 );
}

void trigger_cb()
{
    digitalWrite(txPin, HIGH);  //트리거 중지
    Data_Receive_Flag = 1;
    FlexiTimer2::stop();
}

void setup() 
{
    Serial.begin(115200);
    SoftSerial.begin(115200);
    pinMode(txPin, OUTPUT);  //로우 레벨 펄스, 트리거 출력
    digitalWrite(txPin, HIGH);
    FlexiTimer2::set(5, 0.0001, trigger_cb);  //500us 타이밍 설정
    lcd.init();
    lcd.backlight();
    lcd.clear();  
    delay(1000);
    Serial.println("Init Finished2");
}

void loop() 
{
    static uint32_t trigger_time;
    static uint32_t led_time;
    uint8_t recv_buf[10] = {0};
    uint16_t distance = 0;
    uint16_t length = 0;
    uint8_t sum = 0;
    if ((millis() - trigger_time) > 500)
    {
        digitalWrite(txPin, LOW);  //트리거 센서 출력
        FlexiTimer2::start();
        trigger_time = millis();
    }
    if ((SoftSerial.available() > 0) && (Data_Receive_Flag == 1))
    {
        for(uint8_t i = 0; i < 4; i ++)
        {
            recv_buf[i] = SoftSerial.read();
            Serial.write(recv_buf[i]);
            length++;
        }
    }

    if ((length == 4) && (recv_buf[0] == 0xff) && (Data_Receive_Flag == 1))
    {
        sum = recv_buf[0] + recv_buf[1] + recv_buf[2];
        if (sum == recv_buf[3])
        {
            distance = (recv_buf[1] << 8) | recv_buf[2];
            float velocity = distance/(millis() - trigger_time);
            char out_txt[16] = {0};
            sprintf(out_txt, "%d mm \r\n", distance);
            Serial.println(out_txt) ;
            Serial.print(velocity);Serial.println("m/sec");
            Serial.println(millis() - trigger_time);

            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Distance Check");
            lcd.setCursor(1, 1);
            lcd.print(distance); 
            lcd.print("mm "); 
            lcd.print(velocity); 
            lcd.print("m/sec");
        }
        Data_Receive_Flag = 0;
    }
    
    if ((millis() - led_time) > 100 )
    {
        digital_toggle(LED_BUILTIN);
        led_time = millis();
    }
}
