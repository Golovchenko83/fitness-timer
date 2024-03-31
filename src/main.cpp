#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverPower.h>
#define WIRE Wire
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &WIRE);
volatile byte flagSleep = 2,dysplay_init=1;
volatile byte flagNazatia;            // флаг о нажатии или отжатии кнопки
volatile byte flagFitnessTaimer;      // флаг о запуске или истечении Fitness таймера
volatile byte flagClickTaimer;        // флаг о запуске или истечении таймера для двойного нажатия
volatile byte d_update;               // флаг об необходимости обновления информации на дисплее
volatile byte click;                  // количество кликов
volatile byte minute = 1;             // количество выбранных минут для Fitness таймера
byte delays = 50;                     // задержка нажатия кнопки для антидребезга
unsigned long d_t;                    // таймер не обновления информации на дисплее
volatile unsigned long antidebounce;  // время нажатия кнопки для антидребезгаvolatile
unsigned long InterruptTaimer = 5000; // таймер пробуждения
unsigned long FitnessTaimer;          // количество времени Fitness таймера
unsigned long UpdateFitnessTaimer;    // таймер не обновления информации Fitness таймера на дисплее
unsigned long svoimillis;             // svoimillis=millis()
volatile unsigned long nazatie;       // время нажатия кнопки
volatile unsigned short int otrezok;  // отрезок времени от нажатия и отжатия кнопки
volatile unsigned long ClickTaimer;   // таймер для двойного и более нажатия
unsigned long ostatok;                // время в миллисекундах до истечения Fitness таймера

void Vivod() // вывод информации на дисплей
{
  if(dysplay_init==0){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dysplay_init=1;
  }
  display.clearDisplay();
  // вывод выбранных минут без запущенного таймера
  display.setCursor(17, 4);
  if (flagFitnessTaimer == 0)
  {
    display.println(minute);
  }
  // вывод надписи "Fitness"
  display.setTextSize(1);
  display.setCursor(67, 13);
  display.println("Fitness");
  display.setCursor(17, 4);
  display.setTextSize(8);
  // вывод рисунка молнии
  display.drawPixel(120, 11, WHITE);
  display.drawFastHLine(119, 12, 2, WHITE);
  display.drawFastHLine(118, 13, 3, WHITE);
  display.drawFastHLine(117, 14, 4, WHITE);
  display.drawFastHLine(117, 15, 5, WHITE);
  display.drawFastHLine(118, 16, 4, WHITE);
  display.drawFastHLine(118, 17, 3, WHITE);
  display.drawFastHLine(118, 18, 2, WHITE);
  display.drawPixel(118, 19, WHITE);
  // вывод рамки
  display.drawFastHLine(0, 0, 128, WHITE);
  display.drawFastHLine(0, 63, 128, WHITE);
  display.drawFastVLine(0, 0, 64, WHITE);
  display.drawFastVLine(127, 0, 64, WHITE);
  // вывод оставшегося времени Fitness таймера при запущенном Fitness таймере
  if (flagFitnessTaimer == 1)
  {
    ostatok = FitnessTaimer - svoimillis;
    display.print(ostatok / 60000);
    display.setTextSize(4);
    display.setCursor(64, 32);
    display.print((ostatok % 60000) / 1000);
    display.setTextSize(8);
  }
  // обновление дисплея
  display.display();
}

void Update() // вывод информации на дисплей каждые 300 миллисекунд при изменении отображаемых данных
{
  if (d_update == 1 && (d_t < svoimillis))
  {
    d_t = svoimillis + 300;
    Vivod(); // вывод информации на дисплей
    d_update = 0;
  }
}

void Interrupt() // функция прерывания
{
  InterruptTaimer = svoimillis + 660000; // обновление таймера пробуждения
  d_update = 1;                          // обновление информации на дисплее
  if (flagSleep > 1)
  {
    if (flagNazatia == 0 && (svoimillis - antidebounce) >= delays /* && flagSleep == 0*/) // если на кнопку нажали. Антидребезг
    {
      nazatie = svoimillis;      // фиксирование времени нажатия кнопки
      antidebounce = svoimillis; // фиксирование времени нажатия кнопки для антидребезга
      flagNazatia = 1;           // поднятие флага нажатия кнопки при нажатии кнопки
    }

    if (digitalRead(2) && nazatie != svoimillis && flagNazatia == 1) // если кнопку отжали после нажатия
    {
      otrezok = svoimillis - nazatie;
      flagNazatia = 0;   // опускание флага нажатия кнопки при отпускании кнопки
      if (otrezok < 400) // если кнопку отжали за 400 или меньше миллисекунд (клик)
      {
        click++; // счётчик кликов по кнопке
        if (click < 2)
        {
          ClickTaimer = svoimillis + 400; // обновление таймера на 400 миллисекунд для двойного нажатия
        }
        flagClickTaimer = 1; // поднятие флага таймера для двойного нажатия при запуске таймера для двойного нажатия
      }

      if (otrezok > 400) // если кнопку отжали дольше, чем за 400 миллисекунд (удержание)
      {
        switch (flagFitnessTaimer) // switch с проверкой флага запуска Fitness таймера
        {
        case 0:                                        // если Fitness таймер был не запущен, то запустить его
          flagFitnessTaimer = 1;                       // поднятие флага Fitness таймера при запуске Fitness таймера
          FitnessTaimer = svoimillis + 60000 * minute; // вычисление длительности Fitness таймера
          break;
        case 1:                  // если Fitness таймер был запущен, то выключить его
          flagFitnessTaimer = 0; // опускание флага Fitness таймера
          break;
        }
      }
    }
  }
  else
  {
    digitalWrite(7,0);
    delay(100);
    flagSleep++;
   
  }
}

void setup()
{
  pinMode(2, INPUT_PULLUP);                                     // подтяжка 2 пина к резистору
  pinMode(3, OUTPUT);                                            // 3 пин, на котором находится пищалка объявляется выходом
  pinMode(7, OUTPUT);
  digitalWrite(7,0);                                           
  attachInterrupt(digitalPinToInterrupt(2), Interrupt, CHANGE); // 2 пин объявляется пином с прерыванием и режимом FALLING
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(8);
  display.setTextColor(SSD1306_WHITE);
  power.setSleepMode(POWERDOWN_SLEEP); // выбор типа сна
  // отключение ненужных перифирий
  power.hardwareDisable(PWR_TIMER3);
  power.hardwareDisable(PWR_TIMER4);
  power.hardwareDisable(PWR_TIMER5);
  power.hardwareDisable(PWR_UART0);
  power.hardwareDisable(PWR_UART1);
  power.hardwareDisable(PWR_UART2);
  power.hardwareDisable(PWR_UART3);
  power.hardwareDisable(PWR_SPI);
  power.hardwareDisable(PWR_USB);
  Vivod(); // вывод информации на дисплей
}

void loop()
{
  Update();
  svoimillis = millis(); // приравнивание svoimillis к millis() для оптимизации

  if (svoimillis >= ClickTaimer && flagClickTaimer == 1 && flagFitnessTaimer == 0) // если истёк таймер "для двойного и более нажатия" и Fitness таймер не запущен
  {
    if (click == 1) // если нажали один раз
    {
      if (minute < 9)
      {
        minute = minute + 1;
      }
    }
    if (click > 1) // если нажали более 1 раза
    {
      if (minute > 1)
      {
        minute = minute - 1;
      }
    }
    flagClickTaimer = 0; // опускание флага таймера для двойного нажатия при истечении таймера для двойного нажатия
    click = 0;           // сброс счётчика нажатия на кнопку
    d_update = 1;        // обновление информации на дисплее
  }

  if (svoimillis >= UpdateFitnessTaimer && flagFitnessTaimer == 1) // если таймер не обновления информации Fitness таймера на дисплее истёк и Fitness таймер запущен
  {
    if (svoimillis >= FitnessTaimer) // если Fintess таймер истёк
    {
      display.clearDisplay();
      // вывод надписи "Fitness"
      display.setTextSize(1);
      display.setCursor(67, 13);
      display.println("Fitness");
      display.setCursor(17, 4);
      display.setTextSize(8);
      // вывод рисунка молнии
      display.drawPixel(120, 11, WHITE);
      display.drawFastHLine(119, 12, 2, WHITE);
      display.drawFastHLine(118, 13, 3, WHITE);
      display.drawFastHLine(117, 14, 4, WHITE);
      display.drawFastHLine(117, 15, 5, WHITE);
      display.drawFastHLine(118, 16, 4, WHITE);
      display.drawFastHLine(118, 17, 3, WHITE);
      display.drawFastHLine(118, 18, 2, WHITE);
      display.drawPixel(118, 19, WHITE);
      // вывод рамки
      display.drawFastHLine(0, 0, 128, WHITE);
      display.drawFastHLine(0, 63, 128, WHITE);
      display.drawFastVLine(0, 0, 64, WHITE);
      display.drawFastVLine(127, 0, 64, WHITE);
      // вывод оставшегося времени Fitness таймера при запущенном Fitness таймере
      display.print(0);
      display.setTextSize(4);
      display.setCursor(64, 32);
      display.print(0);
      display.setTextSize(8);
      // обновление дисплея
      display.display();
      // мелодия истечения Fitness таймера
      tone(3, 1000, 400);
      delay(500);
      tone(3, 2000, 400);
      delay(500);
      tone(3, 1000, 400);
      delay(500);
      tone(3, 2000, 400);
      delay(500);
      tone(3, 1000, 400);
      delay(500);
      tone(3, 2000, 400);
      delay(500);
      tone(3, 500, 2000);
      flagFitnessTaimer = 0; // опускание флага Fitness таймера при Fitness таймера
    }
    UpdateFitnessTaimer = svoimillis + 1000; // запуск таймера не обновления информации Fitness таймера на дисплее
    d_update = 1;                            // обновление информации на дисплее
  }

  if (InterruptTaimer <= svoimillis) // если таймер пробуждения истёк
  {
    flagSleep = 0;
    dysplay_init=0;
    display.clearDisplay();
    display.display();
    digitalWrite(7,1); 
    power.sleep(SLEEP_FOREVER); // уход Arduino в вечный сон до первого прерывания
  }
}