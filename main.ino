#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // 0.96 inç OLED için 64 piksel
#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int buzzerPin     = 14;
const int buttonPin     = 13;
const int kirmiziLedPin = 12; // Kırmızı LED -> GPIO 12
const int beyazLedPin   = 11; // Beyaz LED   -> GPIO 11

// Ses seviyesi ayarı: 1 (en kısık) ile 128 (en yüksek) arası
const int sesSeviyesi = 20;

// Nota frekansları (Hz)
#define c   523
#define cd  554
#define d   587
#define dd  622
#define e   659
#define f   698
#define fd  740
#define g   784
#define gd  830
#define a   880
#define ad  932
#define b   988
#define c2  1046
#define cd2 1108
#define d2  1174
#define dd2 1244
#define e2  1318
#define f2  1396

const int notalar[] = {
  c, f, g, gd, e, g, f,
  f, ad, c2, cd2, a, c2, ad,
  c2, ad, c2, g, g, c2, ad, gd, g, f, e, f, g, c,
  c, c, g, f, e, d, e, f, g, gd, g, f, e,
  c, f, g, gd, e, g, f, d2, c2, ad, gd, g, f, e, f, g, c, f
};

const int sureler[] = {
  800, 800, 800, 800, 400, 200, 1600,
  800, 800, 800, 800, 400, 200, 1600,
  200, 200, 200, 400, 200, 800, 400, 200, 400, 200, 400, 200, 400, 800,
  400, 200, 800, 400, 200, 400, 200, 400, 200, 800, 400, 200, 1600,
  800, 800, 800, 800, 400, 200, 800, 400, 200, 400, 200, 400, 200, 400, 200, 800, 400, 1600
};

const int toplamNota = sizeof(notalar) / sizeof(notalar[0]);

bool caliyor = false;
bool oledHazir = false;
int notaIndeksi = 0;
unsigned long notaBaslangic = 0;
bool notaSusturuldu = false;

bool sonButonDurumu = HIGH;
unsigned long sonBasmaZamani = 0;

void notaCal(int frekans, int indeks) {
  ledcWriteTone(buzzerPin, frekans);
  ledcWrite(buzzerPin, sesSeviyesi);

  if (indeks % 2 == 0) {
    digitalWrite(kirmiziLedPin, HIGH);
    digitalWrite(beyazLedPin, LOW);
  } else {
    digitalWrite(kirmiziLedPin, LOW);
    digitalWrite(beyazLedPin, HIGH);
  }
  notaSusturuldu = false;
}

void buzzerSustur() {
  ledcWrite(buzzerPin, 0);
  digitalWrite(kirmiziLedPin, LOW);
  digitalWrite(beyazLedPin, LOW);
  notaSusturuldu = true;
}

// 128x64 ekran için üstte başlık, altta 2 satır dize düzeni
void ekranaYaz(const char* satir1, const char* satir2) {
  if (!oledHazir) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Üst başlık bölümü (0-15 px arası)
  display.setCursor(20, 4);
  display.println("ISTIKLAL MARSI");
  display.drawLine(0, 15, 128, 15, SSD1306_WHITE);

  // Alt dize bölümü
  display.setCursor(0, 26);
  display.println(satir1);
  display.setCursor(0, 42);
  display.println(satir2);

  display.display();
}

void ritmeGoreEkranGuncelle(int indeks) {
  if (indeks == 0)       ekranaYaz("Korkma, sonmez", "bu safaklarda");
  else if (indeks == 7)  ekranaYaz("Yuzen al sancak!", "");
  else if (indeks == 14) ekranaYaz("Sonmeden yurdumun", "ustunde tuten");
  else if (indeks == 23) ekranaYaz("En son ocak.", "");
  else if (indeks == 28) ekranaYaz("O benim milletimin", "yildizidir");
  else if (indeks == 37) ekranaYaz("Parlayacak!", "");
  else if (indeks == 41) ekranaYaz("O benimdir, o benim", "milletimindir");
  else if (indeks == 52) ekranaYaz("ANCAK!", "");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(buttonPin, INPUT_PULLUP); // GND bağlantısı için (3.3V ise INPUT_PULLDOWN yap)
  pinMode(kirmiziLedPin, OUTPUT);
  pinMode(beyazLedPin, OUTPUT);
  digitalWrite(kirmiziLedPin, LOW);
  digitalWrite(beyazLedPin, LOW);

  ledcAttach(buzzerPin, 1000, 8);
  buzzerSustur();

  Wire.begin(I2C_SDA, I2C_SCL);

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledHazir = true;
    Serial.println("OLED 0x3C adresinde baslatildi.");
  } else if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    oledHazir = true;
    Serial.println("OLED 0x3D adresinde baslatildi.");
  } else {
    oledHazir = false;
    Serial.println("OLED bulunamadi! SDA/SCL ve guc baglantisini kontrol et.");
  }

  ekranaYaz("Hazir!", "Baslamak icin bas...");
}

void loop() {
  bool butonDurumu = digitalRead(buttonPin);

  if (sonButonDurumu == HIGH && butonDurumu == LOW && (millis() - sonBasmaZamani > 200)) {
    sonBasmaZamani = millis();
    caliyor = !caliyor;

    if (caliyor) {
      notaIndeksi = 0;
      ritmeGoreEkranGuncelle(notaIndeksi);
      notaCal(notalar[0], notaIndeksi);
      notaBaslangic = millis();
    } else {
      buzzerSustur();
      ekranaYaz("DURDURULDU", "Baslamak icin bas...");
    }
  }
  sonButonDurumu = butonDurumu;

  if (caliyor) {
    unsigned long gecenSure = millis() - notaBaslangic;

    if (!notaSusturuldu && gecenSure >= sureler[notaIndeksi]) {
      buzzerSustur();
    }

    if (gecenSure >= sureler[notaIndeksi] * 1.1) {
      notaIndeksi++;
      if (notaIndeksi >= toplamNota) {
        caliyor = false;
        buzzerSustur();
        ekranaYaz("Tamamlandi.", "Baslamak icin bas...");
      } else {
        ritmeGoreEkranGuncelle(notaIndeksi);
        notaCal(notalar[notaIndeksi], notaIndeksi);
        notaBaslangic = millis();
      }
    }
  }
}
