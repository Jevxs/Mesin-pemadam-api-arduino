// ROBOT PEMADAM LILIN v2 (jalan keliling + kejar api + padamkan)

#include <Servo.h>

// ---------- PIN ----------
#define PIN_RODA1 4
#define PIN_RODA2 5
#define KIPAS     6
#define BUZZER    3
#define TRIG      11
#define ECHO      12

// ---------- KALIBRASI ----------
#define STOP1        93
#define STOP2        95
#define API          500    // batas "ada api"
#define SENSOR_TERBALIK false // true kalau sensor memberi angka KECIL saat api dekat
#define JARAK_TIUP   15     // cm, jarak ke lilin untuk meniup
#define JARAK_TEMBOK 12     // cm, jarak ke tembok saat jalan keliling
#define LAMA_TIUP    5000   // ms
#define SELISIH      30     // selisih minimal sensor supaya belok (anti goyang)

#define KIPAS_ON  LOW
#define KIPAS_OFF HIGH

Servo roda1, roda2;

// ---------- GERAK ----------
void maju()     { roda1.write(180);   roda2.write(0);     }
void mundur()   { roda1.write(0);     roda2.write(180);   }
void kiri()     { roda1.write(STOP1); roda2.write(0);     }
void kanan()    { roda1.write(180);   roda2.write(STOP2); }
void putar()    { roda1.write(180);   roda2.write(180);   }
void berhenti() { roda1.write(STOP1); roda2.write(STOP2); }

// ---------- BACA JARAK ----------
int bacaJarak() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long durasi = pulseIn(ECHO, HIGH, 6000);
  if (durasi == 0) return 100;
  return durasi / 58;
}

// ---------- BACA 1 SENSOR API (sudah dibalik kalau perlu) ----------
int bacaSatu(int i) {
  int v = analogRead(A1 + i);
  return SENSOR_TERBALIK ? (1023 - v) : v;
}

// ---------- CARI SENSOR API TERKUAT ----------
int bacaApi(int &kuat) {
  int idx = 0;
  int maks = 0;
  for (int i = 0; i < 5; i++) {
    int v = bacaSatu(i);
    if (v > maks) {
      maks = v;
      idx = i;
    }
  }
  kuat = maks;
  return idx;
}

// ---------- PADAMKAN ----------
void padamkan() {
  berhenti();
  digitalWrite(KIPAS, KIPAS_ON);
  delay(LAMA_TIUP);
  digitalWrite(KIPAS, KIPAS_OFF);
  delay(500);

  int kuat;
  bacaApi(kuat);
  if (kuat < API) {
    // api mati -> bunyi lalu selesai
    digitalWrite(BUZZER, HIGH);
    delay(2000);
    digitalWrite(BUZZER, LOW);
    while (true) {}
  }
  // api belum mati -> loop akan mengulang
}

// ---------- JALAN KELILING (tidak ada api) ----------
void jalanKeliling(int jarak) {
  if (jarak <= JARAK_TEMBOK) {
    // ada tembok: mundur sebentar lalu putar
    berhenti();
    delay(100);
    mundur();
    delay(300);
    putar();
    delay(400);
    berhenti();
  } else {
    maju();
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(KIPAS, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(KIPAS, KIPAS_OFF);
  digitalWrite(BUZZER, LOW);
  roda1.attach(PIN_RODA1);
  roda2.attach(PIN_RODA2);
  berhenti();
  delay(2000);
}

// ---------- LOOP ----------
void loop() {
  int kuat;
  int idx = bacaApi(kuat);
  int jarak = bacaJarak();

  Serial.print("api=");
  Serial.print(kuat);
  Serial.print(" idx=");
  Serial.print(idx);
  Serial.print(" jarak=");
  Serial.println(jarak);

  // 1. Tidak ada api -> jalan keliling sambil cari
  if (kuat < API) {
    jalanKeliling(jarak);
    delay(50);
    return;
  }

  // 2. Api ada dan sudah dekat -> padamkan
  if (jarak <= JARAK_TIUP) {
    padamkan();
    return;
  }

  // 3. Api ada tapi jauh -> kejar
  int tengah = bacaSatu(2);
  if (idx == 2 || (kuat - tengah) < SELISIH) {
    maju();
  } else if (idx < 2) {
    kiri();
  } else {
    kanan();
  }
  delay(50);
}
