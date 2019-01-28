/**
  * Super Mario Overworld / Main Theme per Arduino
  * Code for Sketch (Arduino) tested on Arduino UNO R3
  *
  * Made by Reyboz Blog
  * [ http://blog.reyboz.it ]
  *
  * Un caloroso ringraziamento a Joseph Karam, autore delle trascrizioni degli
  * spartiti per pianoforte a due mani sulle note della composizione originale della
  * Nintendo Entertainment System by Koji Kondo, che ci ha permesso di riprodurre
  * fedelmente la canzone.
  * [ http://www.mariopiano.com ]
*/

//  [ http://code.google.com/p/rogue-code/wiki/ToneLibraryDocumentation ]
#include <Tone.h>

// Define speaker pins (PWM~) [REQUIRED]
#define PIN_PLAYER_1 11
#define PIN_PLAYER_2 10

// Define ground pins for display [OPTIONAL]
#define DISPLAY_1 13
#define DISPLAY_2 12

// Trimmer pins (Analog in) [OPTIONAL]
#define PIN_VOLUME 0

// LED pins [OPTIONAL]
#define LED_1 9
#define LED_2 8
#define LED_3 7
#define LED_4 6
#define LED_5 5
#define LED_6 4
#define LED_7 3
#define LED_8 2

Tone player_1;
Tone player_2;

int tempo;

// Questo valore più è alto più stacca le note fra loro. [OPTIONAL]
int tronca = 15;

// Puoi impostare i limiti di velocità [OPTIONAL]
int tempo_minimo = 78;
int tempo_massimo = 500;

// Definisci l'ordine dei pin nel display affinchè siano ordinati secondo la convenzione universale [OPTIONAL]
int pin_leds[8] = {LED_3, LED_4, LED_5, LED_6, LED_7, LED_2, LED_1, LED_8};

// Note facili da usare (Non modificare)
#define SOL_d2 104
#define LA2    110
#define LA_d2  114
#define SI2    124
#define DO3    131
#define DO_d3  139
#define RE3    147
#define RE_d3  156
#define MI3    165
#define FA3    175
#define FA_d3  185
#define SOL3   196
#define SOL_d3 208
#define LA3    220
#define LA_d3  233
#define SI3    247
#define DO4    262
#define DO_d4  277
#define RE4    294
#define RE_d4  311
#define MI4    330
#define FA4    349
#define FA_d4  370
#define SOL4   392
#define SOL_d4 415
#define LA4    440
#define LA_d4  466
#define SI4    494
#define DO5    523
#define DO_d5  554
#define RE5    587
#define RE_d5  622
#define MI5    659
#define FA5    698
#define FA_d5  740
#define SOL5   784
#define SOL_d5 831
#define LA5    880
#define LA_d5  932
#define SI5    988
#define DO6    1046
#define DO_d6  1109
#define RE6    1175
#define RE_d6  1245
#define MI6    1319
#define FA6    1397
#define FA_d6  1480
#define SOL6   1568
#define SOL_d6 1661
#define LA6   1760
#define LA_d6  1865

// 4/3 (DON'T EDIT)
#define TERZINA 1.333

// 4/3/2 (DON'T EDIT)
#define TERZINA_2 0.666

// 1/3 (DON'T EDIT)
#define TERZINA_3 0.333

// 1/4 (Non modificare)
#define SEMICROMA 0.25

// DON'T EDIT SETUP
void setup() {
  //Serial.begin(9600);
  pinMode(DISPLAY_1, OUTPUT);
  pinMode(DISPLAY_2, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(LED_6, OUTPUT);
  pinMode(LED_7, OUTPUT);
  pinMode(LED_8, OUTPUT);
  player_1.begin(PIN_PLAYER_1);
  player_2.begin(PIN_PLAYER_2);
}

/* YOU CAN USE THESE FUNCTIONS:
/**
 * OVERWORLD();
 * UNDERWATER();
 * UNDERWORLD();
 * CASTLE();
 * FLAGPOLEFANFARE();
 * DAMAGEWARP();
 * COIN( number_of_coin );
 * POWERUP();
 * STARMAN();
 * DEATH();
*/
void loop() {
  ONEUP();
  pausa(8);
  OVERWORLD();
  pausa(8);
  FLAGPOLEFANFARE();
  pausa(8);
  DAMAGEWARP();
  pausa(4);
  UNDERWORLD();
  UNDERWORLD();
  pausa(16);
  COIN(1);
  pausa(1);
  COIN(1);
  COIN(1);
  COIN(5);
  POWERUP();
  pausa(2);
  STARMAN();
  STARMAN();
  pausa(8);
  UNDERWATER();
  pausa(8);
  DEATH();
  while(1); // Shut up!
}

void enableDisplay(int pin) {
  digitalWrite(pin, LOW);
}

void disableDisplay(int pin) {
  digitalWrite(pin, HIGH);
}

int inputTempo() {
  //global tempo_massimo, tempo_minimo;
  return (analogRead(PIN_VOLUME) / 1023.0) * (tempo_massimo - tempo_minimo) + tempo_minimo;
}

/*
    _____________________________________________________________
    |                          Uhm...                            |
    |              Ciò potrebbe annoiarti a morte!               |
    |____________________________________________________________|
    Alcuni sanno che tutte le note hanno una frequenza e che essa
    si può prevedere tramite questa funzione matematica:

      frequenza(n) = 440 * 2^(n/12)
      [ http://it.wikipedia.org/wiki/Nota_musicale ]

    Guardatela bene.
    Questa formula ci rivela che tutte le note che noi usiamo
    sono in riferimento ad una e una sola frequenza: 440Hz.
    Ovvero il nostro 'LA' ('A' in americano).

    Fermandosi a ragionare ci si rente conto di una cosa strana:
    Qualcuno si è mai chiesto per quale motivo noi 'normali' chiamiamo
    le note DO, RE MI FA... iniziando dal DO, mentre quegli 'strani' degli
    Inglesi le chiamano A, B, C, D... inziando dalla A che sarebbe un LA?

    Ebbene siamo noi quegli strani. Perchè la notazione Inglese inizia
    appunto con la nota più importante, ovvero quella a 440Hz: 'A'.

    Ed è sconvolgente sapere che se noi siamo quelli 'strani', la 'colpa' è tutta
    di un monaco (pure Italiano).
    
      [ http://it.wikipedia.org/wiki/Paolo_Diacono ]

    Curioso? No? Bah...

    Detto ciò, c'è un motivo se siamo dovuti andare a cercare quella
    formula. Infatti per suonare io non faccio altro che passare una frequenza
    ad un nostro metodo 'suona(frequenza, tempo)'.
    Perciò all'interno di 'suona', avrei potuto creare un enorme 'switch case',
    controllando manualmente tutte le frequenze possibili immaginabili che mi
    arrivano in ingresso e associarle a quello che devo far visualizzare sul display.
    Questo però ci è sembrato un metodo di programmazione idiota, e quindi ci
    siamo presi una nottata del nostro tempo per esplicitare la nota dalla
    funzione della frequenza citata in alto, e abbiamo ottenuto un vero e proprio
    riconoscitore di note dinamico.
    
    Ed ecco il risultato:
*/
int getAmericanIndexOfLetterFromFrequency(int frequency) {
  double nota = 12*(log(((double)frequency)/440)/log(2));
  //Serial.println();
  //Serial.println(nota);
  while(nota >= 12) {
     nota = nota-12;
  }
  while(nota < 0 ) {
     nota = nota+12;
  }
  //Serial.println(nota);
  // Arrotondamento
  float virgola = nota - ((int) nota);
  if(virgola >= 0) {
    return (virgola < 0.5) ? nota : ++nota;
  } else {
    return (virgola <-0.5) ? nota : --nota;
  }
}

void setLetter(int index_letter) {
  // global pin_leds;
  boolean letter_table[12][8] = {
    {true, true, true, false, true, true, true, false}, // A
    {true, true, true, false, true, true, true, true}, // A_d
    {false, false, true, true, true, true, true, false}, // B
    {true, false, false, true, true, true, false, true}, // C_d
    {true, false, false, true, true, true, false, false}, // C
    {false, true, true, true, true, false, true, false}, // D
    {false, true, true, true, true, false, true, true}, // D_d
    {true, false, false, true, true, true, true, false}, // E
    {true, false, false, false, true, true, true, false}, // F
    {true, false, false, false, true, true, true, true}, // F_d
    {true, false, true, true, true, true, true, false}, // G
    {true, false, true, true, true, true, true, true} // G_d
  };
  for(int i=0; i<8; i++) {
    digitalWrite(pin_leds[i], (letter_table[index_letter][i]) ? HIGH : LOW);
  }
}

void suona(int frequency, float fract) {
  setLetter(getAmericanIndexOfLetterFromFrequency(frequency));
  player_2.play(frequency, inputTempo() * fract - tronca);
  enableDisplay(DISPLAY_1);
  while(player_2.isPlaying());
  disableDisplay(DISPLAY_1);
  delay(tronca);
}

void suonaCoppia(int frequency, float fract) {
  int index_letter = getAmericanIndexOfLetterFromFrequency(frequency);
  int ms = inputTempo() * fract - tronca;
  player_1.play(frequency, ms);
  player_2.play(frequency, ms);
  while(player_1.isPlaying() || player_2.isPlaying()) {
    enableDisplay(DISPLAY_1);
    setLetter(index_letter);
    delay(4);
    disableDisplay(DISPLAY_1);
    delay(1);
    enableDisplay(DISPLAY_2);
    setLetter(index_letter);
    delay(4);
    disableDisplay(DISPLAY_2);
  }
  delay(tronca);
}

void suonaDoppio(int frequency_1, int frequency_2, float fract) {
  int index_letter_1 = getAmericanIndexOfLetterFromFrequency(frequency_1);
  int index_letter_2 = getAmericanIndexOfLetterFromFrequency(frequency_2);
  int ms = inputTempo() * fract - tronca;
  player_1.play(frequency_1, ms);
  player_2.play(frequency_2, ms);
  while(player_1.isPlaying() || player_2.isPlaying()) {
    enableDisplay(DISPLAY_1);
    setLetter(index_letter_1);
    delay(4);
    disableDisplay(DISPLAY_1);
    delay(1);
    enableDisplay(DISPLAY_2);
    setLetter(index_letter_2);
    delay(4);
    disableDisplay(DISPLAY_2);
  }
  delay(tronca);
}

void pausa(float fract) {
  delay(inputTempo() * fract);
}

void introduzione() {
  // Battuta 1
  suonaDoppio(MI5, FA_d4, 1);
  suonaDoppio(MI5, FA_d4, 1);
  pausa(1);
  suonaDoppio(MI5, FA_d4, 1);
  // Battuta 2
  pausa(1);
  suonaDoppio(DO5, FA_d4, 1);
  suonaDoppio(MI5, FA_d4, 1);
  pausa(1);
  
  // Battuta 3
  suonaDoppio(SOL5, SOL4, 1);
  pausa(3);
  // Battuta 4
  suonaDoppio(SOL4, SOL3, 1);
  pausa(3);
}

void OVERWORLDFoglio1Riga2() {
  // Battita 1
  suonaDoppio(DO5, SOL3, 1);
  pausa(2);
  suonaDoppio(SOL4, MI3, 1);
  // Battuta 2
  pausa(2);
  suonaDoppio(MI4, DO3, 1);
  pausa(1);
  // Battuta 3
  pausa(1);
  suonaDoppio(LA4, FA3, 1);
  pausa(1);
  suonaDoppio(SI4, SOL3, 1);
  // Battuta 4
  pausa(1);
  suonaDoppio(LA_d4, FA_d3, 1);
  suonaDoppio(LA4, FA3, 1);
  pausa(1);
}

void OVERWORLDFoglio1Riga3() {
  // Battuta 1
  suonaDoppio(SOL4, MI3, TERZINA);
  suonaDoppio(MI5, DO4, TERZINA);
  suonaDoppio(SOL5, MI4, TERZINA);
  // Battuta 2
  suonaDoppio(LA5, FA4, 1);
  pausa(1);
  suonaDoppio(FA5, RE4, 1);
  suonaDoppio(SOL5, MI4, 1);
  // Battuta 3
  pausa(1);
  suonaDoppio(MI5, DO4, 1);
  pausa(1);
  suonaDoppio(DO5, LA3, 1);
  // Battuta 4
  suonaDoppio(RE5, SI3, 1);
  suonaDoppio(SI4, SOL4, 1);
  pausa(2);
}

void OVERWORLDFoglio2Riga1Battuta1() {
  suona(DO3, 1);
  pausa(1);
  suonaDoppio(SOL5, MI5, 1);
  suonaDoppio(FA_d5, RE_d5, 1);
}

void OVERWORLDFoglio2Riga1() {
  // Battuta 1
  OVERWORLDFoglio2Riga1Battuta1();
  // Battuta 2
  suonaDoppio(FA5, RE5, 1);
  suonaDoppio(RE_d5, SI4, 1);
  suona(DO4, 1);
  suonaDoppio(MI5, DO5, 1);
  // Battuta 3
  suona(FA3, 1);
  suonaDoppio(SOL_d4, MI4, 1);
  suonaDoppio(LA4, FA4, 1);
  suonaDoppio(DO5, SOL4, 1);
  // Battuta 4
  suona(DO4, 1);
  suonaDoppio(LA4, DO4, 1);
  suonaDoppio(DO5, MI4, 1);
  suonaDoppio(RE5, FA4, 1);
}

void OVERWORLDFoglio2Riga2() {
  // Battuta 1
  OVERWORLDFoglio2Riga1Battuta1();
  // Battuta 2
  suonaDoppio(FA5, RE5, 1);
  suonaDoppio(RE_d5, SI4, 1);
  suona(SOL3, 1);
  suonaDoppio(MI5, DO4, 1);
  // Battuta 3
  pausa(1);
  suonaDoppio(DO6, SOL5, 1);
  pausa(1);
  suonaDoppio(DO6, SOL5, 1);
  // Battuta 4
  suonaDoppio(DO6, SOL5, 1);
  pausa(1);
  suona(SOL3, 1);
  pausa(1);
}

void OVERWORLDFoglio2Riga4() {
  // Battuta 1
  suona(DO3, 1);
  pausa(1);
  suonaDoppio(RE_d5, SOL_d3, 1);
  pausa(1);
  // Battuta 2
  pausa(1);
  suonaDoppio(RE5, LA_d3, 1);
  pausa(2);
  // Battuta 3
  suonaDoppio(DO5, DO4, 1);
  pausa(2);
  suona(SOL3, 1);
  // Battuta 4
  suona(SOL3, 1);
  pausa(1);
  suona(DO3, 1);
  pausa(1);
}

void OVERWORLDFoglio4Riga1Battuta1() {
  suonaDoppio(DO5, SOL_d4, 1);
  suonaDoppio(DO5, SOL_d4, 1);
  pausa(1);
  suonaDoppio(DO5, SOL_d4, 1);
}

void OVERWORLDFoglio4Riga1() {
  // Battuta 1
  OVERWORLDFoglio4Riga1Battuta1();
  // Battuta 2
  pausa(1);
  suonaDoppio(DO5, SOL_d4, 1);
  suonaDoppio(RE5, LA_d4, 1);
  pausa(1);
  // Battuta 3
  suonaDoppio(MI5, SOL4, 1);
  suonaDoppio(DO5, MI4, 1);
  pausa(1);
  suonaDoppio(LA4, MI4, 1);
  // Battuta 4
  suonaDoppio(SOL4, DO4, 1);
  pausa(1);
  suona(SOL3, 1); // Non mettere SOL2 perchè la frequenza bassa manda a troie tutta la classe Tone
  pausa(1);  
}

void OVERWORLDFoglio4Riga2() {
  // Battuta 1
  OVERWORLDFoglio4Riga1Battuta1();
  // Battuta 2
  pausa(1);
  suonaDoppio(DO5, SOL_d4, 1);
  suonaDoppio(RE5, LA_d4, 1);
  suonaDoppio(MI5, SOL4, 1);
  // Battuta 3
  suona(SOL3, 1);
  pausa(2);
  suona(DO3, 1);
  // Battuta 4
  pausa(2);
  suona(SOL3, 1); // Non mettere SOL2, la frequenza è troppo bassa e manda a puttane tutta la classe Tone
  pausa(1);
}

void OVERWORLDFoglio6Riga1() {
  // Battuta 1
  suonaDoppio(MI5, DO5, 1);
  suonaDoppio(DO5, LA4, 1);
  pausa(1);
  suonaDoppio(SOL4, MI4, 1);
  // Battuta 2
  suona(SOL3, 1);
  pausa(1);
  suonaDoppio(SOL_d4, MI4, 1);
  pausa(1);
  // Battuta 3
  suonaDoppio(LA4, FA4, 1);
  suonaDoppio(FA5, DO5, 1);
  suona(FA3, 1);
  suonaDoppio(FA5, DO5, 1);
  // Battuta 4
  suonaDoppio(LA4, FA4, 1);
  suona(DO4, 1);
  suona(FA3, 1);
  pausa(1);
}

void OVERWORLDFoglio6Riga2() {
  // Battuta 1
  suonaDoppio(SI4, SOL4, TERZINA);
  suonaDoppio(LA5, FA5, TERZINA);
  suonaDoppio(LA5, FA5, TERZINA);
  // Battuta 2
  suonaDoppio(LA5, FA5, TERZINA);
  suonaDoppio(SOL5, MI5, TERZINA);
  suonaDoppio(FA5, RE5, TERZINA);
  // Battuta 3
  suonaDoppio(MI5, DO5, 1);
  suonaDoppio(DO5, LA4, 1);
  suona(SOL3, 1);
  suonaDoppio(LA4, FA4, 1);
  // Battuta 4
  suonaDoppio(SOL4, MI4, 1);
  suona(DO5, 1);
  suona(SOL4, 1);
  pausa(1);
}

void OVERWORLDFoglio6Riga4() {
  // Battuta 1
  suonaDoppio(SI4, SOL4, 1);
  suonaDoppio(FA5, RE5, 1);
  pausa(1);
  suonaDoppio(FA5, RE5, 1);
  // Battuta 2
  suonaDoppio(FA5, RE5, TERZINA);
  suonaDoppio(MI5, DO5, TERZINA);
  suonaDoppio(RE5, SI4, TERZINA);
  // Battuta 3
  suonaDoppio(DO5, SOL4, 1);
  suona(MI4, 1);
  suona(SOL3, 1);
  suona(MI4, 1);
  // Battuta 4
  suona(DO4, 1);
  pausa(3);
}

void OVERWORLDFoglio1() {
  // Riga 2
  OVERWORLDFoglio1Riga2();
  // Riga 3
  OVERWORLDFoglio1Riga3();
  // Riga 4
  OVERWORLDFoglio1Riga2();
  // Riga 5
  OVERWORLDFoglio1Riga3();
}

void OVERWORLDFoglio2() {
  // Riga 1
  OVERWORLDFoglio2Riga1();
  // Riga 2
  OVERWORLDFoglio2Riga2();
  // Riga 3
  OVERWORLDFoglio2Riga1();
  // Riga 4
  OVERWORLDFoglio2Riga4();
}

void OVERWORLDFoglio4() {
  // Riga 1
  OVERWORLDFoglio4Riga1();
  // Riga 2 
  OVERWORLDFoglio4Riga2();
  // Riga 3
  OVERWORLDFoglio4Riga1();
  // Riga 4
  introduzione(); 
}

void OVERWORLDFoglio6() {
  // Riga 1
  OVERWORLDFoglio6Riga1();
  // Riga 2
  OVERWORLDFoglio6Riga2();
  // Riga 3
  OVERWORLDFoglio6Riga1();
  // Riga 4
  OVERWORLDFoglio6Riga4();
}

/* UNDERWORLD */

void UNDERWORLDFoglio1Riga1Battuta1() {
  suonaCoppia(DO4, 1);
  suonaCoppia(DO5, 1);
  suonaCoppia(LA3, 1);
  suonaCoppia(LA4, 1);
}

void UNDERWORLDFoglio1Riga1Battuta2() {
  suonaCoppia(LA_d3, 1);
  suonaCoppia(LA_d4, 1);
  pausa(2);
}

void UNDERWORLDFoglio1Riga1() {
  // Battuta 1
  UNDERWORLDFoglio1Riga1Battuta1();
  // Battuta 2
  UNDERWORLDFoglio1Riga1Battuta2();
  // Battuta 3
  pausa(4);
  // Battuta 4
  UNDERWORLDFoglio1Riga1Battuta1();
  // Battuta 5
  UNDERWORLDFoglio1Riga1Battuta2();
  // Battuta 6
  pausa(4);
}

void UNDERWORLDFoglio1Riga2Battuta1() {
  suonaCoppia(FA3, 1);
  suonaCoppia(FA4, 1);
  suonaCoppia(RE3, 1);
  suonaCoppia(RE4, 1);
}

void UNDERWORLDFoglio1Riga2Battuta2() {
  suonaCoppia(RE_d3, 1);
  suonaCoppia(RE_d4, 1);
  pausa(2);
}

void UNDERWORLDFoglio1Riga2() {
  // Battuta 1
  UNDERWORLDFoglio1Riga2Battuta1();
  // Battuta 2
  UNDERWORLDFoglio1Riga2Battuta2();
  // Battuta 3
  pausa(4);
  // Battuta 4
  UNDERWORLDFoglio1Riga2Battuta1();
  // Battuta 5
  UNDERWORLDFoglio1Riga2Battuta2();
  // Battuta 6
  pausa(2);
  suonaCoppia(RE_d4, TERZINA_2);
  suonaCoppia(RE4, TERZINA_2);
  suonaCoppia(DO_d4, TERZINA_2);
}

void UNDERWORLDFoglio1Riga3() {
  // Battuta 1
  suonaCoppia(DO4, 2);
  suonaCoppia(RE_d4, 2);
  // Battuta 2
  suonaCoppia(RE4, 2);
  suonaCoppia(SOL_d3, 2);
  // Battuta 3
  suonaCoppia(SOL3, 2);
  suonaCoppia(DO_d4, 2);
}

void UNDERWORLDFoglio1Riga4() {
  // Battuta 1
  suonaCoppia(DO4, TERZINA_2);
  suonaCoppia(FA_d4, TERZINA_2);
  suonaCoppia(FA4, TERZINA_2);
  suonaCoppia(MI4, TERZINA_2);
  suonaCoppia(LA_d4, TERZINA_2);
  suonaCoppia(LA4, TERZINA_2);
  // Battuta 2
  suonaCoppia(SOL_d4, TERZINA);
  suonaCoppia(RE_d4, TERZINA);
  suonaCoppia(SI3, TERZINA);
  // Battuta 3
  suonaCoppia(LA_d3, TERZINA);
  suonaCoppia(LA3, TERZINA);
  suonaCoppia(SOL_d3, TERZINA);
}

void UNDERWORLD() {
  // Riga 1
  UNDERWORLDFoglio1Riga1();
  // Riga 2
  UNDERWORLDFoglio1Riga2();
  // Riga 3
  UNDERWORLDFoglio1Riga3();
  // Riga 4
  UNDERWORLDFoglio1Riga4();
  // Riga 4
  pausa(12);
}

void COIN(int n_coin) {
  for(int i=0; i<n_coin; i++) {
     suonaCoppia(SI5, 0.5);  
  }
  suonaCoppia(MI6, 1.5);
}

void STARMAN() {
  suonaDoppio(DO5, FA4, 1);
  suonaDoppio(DO5, FA4, 1);
  suonaDoppio(DO5, FA4, 1);
  suona(RE4, (float) 1/2);
  suonaDoppio(DO5, FA4, (float) 1/2);
  pausa((float) 1/2);
  suonaDoppio(DO5, FA4, 1);
  suona(RE4, (float) 1/2);
  suonaDoppio(DO5, FA4, (float) 1/2);
  suona(RE4, (float) 1/2);
  suonaDoppio(DO5, FA4, 1);
  suonaDoppio(SI4, MI4, 1);
  suonaDoppio(SI4, MI4, 1);
  suonaDoppio(SI4, MI4, 1);
  suona(DO4, (float) 1/2);
  suonaDoppio(SI4, MI4, (float) 1/2);
  pausa((float) 1/2);
  suonaDoppio(SI4, MI4, 1);
  suona(DO4, (float) 1/2);
  suonaDoppio(SI4, MI4, (float) 1/2);
  suona(DO4, (float) 1/2);
  suonaDoppio(SI4, MI4, 1);
}

void FLAGPOLEFANFARE() {
  int pretronca = tronca;
  tronca = 0;
  float freqPartenza = SOL3;
  float freqFinale = FA_d5;
  float tempo = 0.0851;
  float passoFreq = (freqFinale - freqPartenza) / 47;
  float freq = freqPartenza;
  while(freq<=freqFinale) {
    suona(freq, tempo);
    freq += passoFreq;
  }
  tronca = pretronca;
  suonaCoppia(SOL5, 0.25);
  pausa(5.75);
  suona(SOL3, TERZINA_2);
  suonaDoppio(DO4, MI3, TERZINA_2);
  suonaDoppio(MI4, SOL3, TERZINA_2);
  suonaDoppio(SOL4, DO3, TERZINA_2);
  suonaDoppio(DO5, MI3, TERZINA_2);
  suonaDoppio(MI5, SOL3, TERZINA_2);
  suonaDoppio(SOL5, MI3, 2);
  suonaDoppio(MI5, DO3, 2);
  suona(SOL_d3, TERZINA_2);
  suonaDoppio(DO4, RE_d3, TERZINA_2);
  suonaDoppio(RE_d4, SOL_d3, TERZINA_2);
  suonaDoppio(SOL_d4, DO3, TERZINA_2);
  suonaDoppio(DO5, MI3, TERZINA_2);
  suonaDoppio(RE_d5, SOL_d3, TERZINA_2);
  suonaDoppio(SOL_d5, RE_d3, 2);
  suonaDoppio(RE_d5, DO3, 2);
  
  suona(LA_d3, TERZINA_2);
  suonaDoppio(RE4, LA_d3, TERZINA_2);
  suonaDoppio(FA4, FA3, TERZINA_2);
  suonaDoppio(LA_d4, RE3, TERZINA_2);
  suonaDoppio(RE5, FA3, TERZINA_2);
  suonaDoppio(FA5, LA_d3, TERZINA_2);
  suonaDoppio(LA_d5, RE4, 2);
  suonaDoppio(LA_d5, LA_d3, TERZINA_2);
  suonaDoppio(LA_d5, LA_d3, TERZINA_2);
  suonaDoppio(LA_d5, LA_d3, TERZINA_2);
  suonaDoppio(DO6, DO4, 4);
}

void POWERUP() {
  suonaCoppia(SOL4, 0.2);
  suonaCoppia(SI4, 0.2);
  suonaCoppia(RE5, 0.2);
  suonaCoppia(SOL5, 0.2);
  suonaCoppia(SI5, 0.2);
  suonaCoppia(SOL_d4, 0.2);
  suonaCoppia(DO5, 0.2);
  suonaCoppia(RE_d5, 0.2);
  suonaCoppia(SOL_d5, 0.2);
  suonaCoppia(DO6, 0.2);
  suonaCoppia(LA_d4, 0.2);
  suonaCoppia(RE5, 0.2);
  suonaCoppia(FA5, 0.2);
  suonaCoppia(LA_d5, 0.2);
  suonaCoppia(RE6, 0.2);
  pausa(1);
}

void ITEMBLOCK() {
  suonaDoppio(SOL4, SOL3, 0.5);
  suonaDoppio(SOL_d4, SOL_d3, 0.5);
  suonaDoppio(LA4, LA3, 0.5);
  suonaDoppio(LA_d4, LA_d3, 0.5);
  suonaDoppio(SI4, SI3, 0.5);
  pausa(1.0);
}

void DAMAGEWARP() {
  for(int i=0; i<3; i++) {
    suonaCoppia(LA4, TERZINA_3);
    suonaCoppia(MI4, TERZINA_3);
    suonaCoppia(LA3, TERZINA_3);
    pausa(1);
  }
  pausa(2);
}

void ONEUP() {
  suonaCoppia(MI5, 1);
  suonaCoppia(SOL5, 1);
  suonaCoppia(MI6, 1);
  suonaCoppia(DO6, 1);
  suonaCoppia(RE6, 1);
  suonaCoppia(SOL6, 1);
  pausa(2);
}

void RESCUEFANFARE() {
  int pretronca = tronca;
  tronca = 0;
  for(int i=0; i<2; i++) {
    suonaDoppio(DO5, MI3, TERZINA_2);
    suonaDoppio(SOL4, MI3, TERZINA_2);
    suonaDoppio(MI4, MI3, TERZINA_2);
  }
  tronca = pretronca;
  suonaDoppio(DO5, MI3, 1);
  suonaDoppio(DO5, MI3, 0.5);
  suonaDoppio(DO5, MI3, 0.5);
  for(int i=0; i<3; i++) {
    suonaDoppio(DO5, MI3, TERZINA_2);
  }
  
  tronca = 0;
  for(int i=0; i<2; i++) {
    suonaDoppio(DO_d5, FA3, TERZINA_2);
    suonaDoppio(SOL_d4, FA3, TERZINA_2);
    suonaDoppio(FA4, FA3, TERZINA_2);
  }
  tronca = pretronca;
  suonaDoppio(DO_d5, FA3, 1);
  suonaDoppio(DO_d5, FA3, 0.5);
  suonaDoppio(DO_d5, FA3, 0.5);
  for(int i=0; i<3; i++) {
    suonaDoppio(DO_d5, FA3, TERZINA_2);
  }
  
  tronca = 0;
  for(int i=0; i<2; i++) {
    suonaDoppio(RE_d5, SOL4, TERZINA_2);
    suonaDoppio(LA_d4, RE_d4, TERZINA_2);
    suonaDoppio(SOL4, LA_d3, TERZINA_2);
  }
  tronca = pretronca;
  suonaDoppio(RE_d5, SOL4, 1);
  suonaDoppio(RE_d5, SOL4, 0.5);
  suonaDoppio(RE_d5, SOL4, 0.5);
  for(int i=0; i<3; i++) {
    suonaDoppio(FA5, LA4, TERZINA_2);
  }
  suonaDoppio(SOL5, SI4, 4);
}

void DEATH() {
  suonaCoppia(DO5, (float) 1/4);
  suonaCoppia(DO_d5, (float) 1/4);
  suonaCoppia(RE5, 0.5);
  pausa(3);
  suonaDoppio(SI4, SOL4, 1);
  suonaDoppio(FA5, RE5, 1);
  pausa(1);
  suonaDoppio(FA5, RE5, 1);
  suonaDoppio(FA5, RE5, TERZINA);
  suonaDoppio(MI5, DO5, TERZINA);
  suonaDoppio(RE5, SI4, TERZINA);
  suonaDoppio(DO5, SOL4, 1);
  suona(MI4, 1);
  suona(SOL3, 1);
  suona(MI4, 1);
  suonaDoppio(DO4, DO3, 1);
  pausa(3);
}

void GAMEOVER() {
  suonaDoppio(DO5, MI4, 1);
  pausa(2);
  suonaDoppio(SOL4, DO4, 1);
  pausa(2);
  suonaDoppio(MI4, SOL3, 2);

  suonaDoppio(LA4, FA4, TERZINA);
  suonaDoppio(SI4, FA4, TERZINA);
  suonaDoppio(LA4, FA4, TERZINA);

  suonaDoppio(SOL_d4, FA4, 2);
  suonaDoppio(LA_d4, FA4, 2);

  suonaDoppio(SOL_d4, FA4, 2);
  suonaDoppio(SOL4, MI4, 1);
  suonaDoppio(SOL4, RE4, 1);

  suonaDoppio(SOL4, MI4, 8);
}

void CASTLEBattuta1() {
  suonaDoppio(RE5, LA_d4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(DO_d4, LA4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(DO4, SOL_d4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(DO_d4, LA4, SEMICROMA);
  suona(SOL4, SEMICROMA);

  suonaDoppio(RE5, LA_d4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(RE_d4, SI4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(RE4, LA_d4, SEMICROMA);
  suona(SOL4, SEMICROMA);
  suonaDoppio(DO_d4, LA4, SEMICROMA);
  suona(SOL4, SEMICROMA);
}

void CASTLEBattuta3() {
  suonaDoppio(DO_d5, LA4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(DO4, SOL_d4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(DO_d4, LA4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(RE4, LA_d4, SEMICROMA);
  suona(FA_d4, SEMICROMA);

  suonaDoppio(DO_d5, LA4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(RE4, LA_d4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(DO_d4, LA_d4, SEMICROMA);
  suona(FA_d4, SEMICROMA);
  suonaDoppio(DO4, SOL_d4, SEMICROMA);
  suona(FA_d4, SEMICROMA); 
}

void CASTLEBattuta5() {
  suonaDoppio(FA5, DO_d5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(FA_d5, RE5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(FA5, DO_d5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(MI5, DO4, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  
  suonaDoppio(FA5, DO_d5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(MI5, DO5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(RE_d5, SI4, SEMICROMA);
  suona(LA_d4, SEMICROMA);
  suonaDoppio(MI5, DO5, SEMICROMA);
  suona(LA_d4, SEMICROMA);
}

void CASTLE() {
  int pretronca = tronca;
  tronca = 0; 
  for(int i=0; i<2; i++) {
    CASTLEBattuta1();
    CASTLEBattuta1();
    CASTLEBattuta3();
    CASTLEBattuta3();
    CASTLEBattuta5();
    CASTLEBattuta5();
  }
  tronca = pretronca;  
}

void UNDERWATER() {
  int pretronca = tronca;
  tronca = 0;

  suonaDoppio(RE4, RE4, 1);
  suonaDoppio(MI4, DO_d4, 1);
  suonaDoppio(FA_d4, DO4, 1);
  
  suonaDoppio(SOL4, SI3, 1);
  suonaDoppio(LA4, DO4, 1);
  suonaDoppio(LA_d4, DO_d4, 1);
  
  suonaDoppio(SI4, RE4, 0.5);
  suonaDoppio(SI4, RE4, 0.5);
  suonaDoppio(SI4, RE4, 1);
  suonaDoppio(SI4, SOL3, 1);
  suonaDoppio(SI4, SOL3, 2);
  suona(SOL4, 1);

  // Riga 2
  suonaDoppio(MI5, DO3, 1);
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, DO4, 1);
  
  suonaDoppio(RE_d5, SI2, 1);
  suonaDoppio(RE_d5, SOL3, 1);
  suonaDoppio(RE_d5, SI3, 1);
  
  suonaDoppio(MI5, DO3, 1);
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, DO4, 1);
  
  suona(MI4, 0.5);
  suonaDoppio(SOL4, MI4, 0.5);
  suonaDoppio(LA4, SOL3, 0.5);
  suonaDoppio(SI4, SOL3, 0.5);
  suonaDoppio(DO5, DO4, 0.5);
  suonaDoppio(RE5, DO4, 0.5);
  
  // Riga 3
  suonaDoppio(MI5, DO3, 1);
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, DO4, 1);
  
  suonaDoppio(RE_d5, SI2, 1);
  suonaDoppio(RE_d5, SOL3, 1);
  suonaDoppio(FA5, SI3, 1);
  
  suonaDoppio(MI5, DO3, 1);
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, DO4, 1);
  
  suona(MI3, 1);
  suona(SOL3, 1);
  suona(DO4, 0.5);
  suona(SOL4, 0.5);
  
  // Riga 4
  suonaDoppio(RE5, RE3, 1);
  suonaDoppio(RE5, SOL3, 1);
  suonaDoppio(RE5, SI3, 1);
  
  suonaDoppio(DO_d5, DO_d3, 1);
  suonaDoppio(DO_d5, FA_d3, 1);
  suonaDoppio(DO_d5, LA_d3, 1);
  
  suonaDoppio(RE5, RE3, 1);
  suonaDoppio(RE5, SOL3, 1);
  suonaDoppio(RE5, SI3, 1);
  
  suona(SI2, 0.5);
  suonaDoppio(SOL4, SI2, 0.5);
  suonaDoppio(LA4, SOL3, 0.5);
  suonaDoppio(SI4, SOL3, 0.5);
  suonaDoppio(DO5, SI3, 0.5);
  suonaDoppio(DO_d5, SI3, 0.5);
  
  // Foglio 2
  
  // Riga 1
  suonaDoppio(RE5, RE3, 1);
  suonaDoppio(RE5, SOL3, 1);
  suonaDoppio(RE5, SI3, 1);

  suonaDoppio(SOL4, SI2, 1);
  suonaDoppio(SOL4, SOL3, 1);
  suonaDoppio(FA5, SI3, 1);
  
  suonaDoppio(MI5, DO3, 1);
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, SOL4, 1);
  
  suona(DO3, 1); // Non posso fare SOL2: Arduino esplode
  suona(SOL3, 1);
  suona(DO4, 0.5);
  suonaDoppio(SOL4, DO4, 0.5);
  
  // Riga 2
  suonaDoppio(SOL5, DO3, 1);
  suonaDoppio(SOL5, SOL3, 1);
  suonaDoppio(SOL5, MI4, 1);
  
  suonaDoppio(SOL5, SI2, 1);
  suonaDoppio(SOL5, SOL3, 1);
  suonaDoppio(SOL5, RE4, 1);
  
  suonaDoppio(SOL5, SI2, 1); // CON LA_d2 Esplode!
  suonaDoppio(SOL5, SOL3, 1);
  suonaDoppio(SOL5, DO_d4, 1);
  
  suonaDoppio(SOL5, DO_d3, 1);
  suonaDoppio(LA5, SOL3, 1);
  suona(MI4, 0.5);
  suonaDoppio(SOL5, MI4, 0.5);
  
  // Riga 3
  suonaDoppio(FA5, RE3, 1);
  suonaDoppio(FA5, LA3, 1);
  suonaDoppio(FA5, RE4, 1);
  
  suonaDoppio(FA5, DO_d3, 1);
  suonaDoppio(FA5, LA3, 1);
  suonaDoppio(FA5, DO_d4, 1);
  
  suonaDoppio(FA5, DO3, 1);
  suonaDoppio(FA5, LA3, 1);
  suonaDoppio(FA5, RE4, 1);
  
  suonaDoppio(FA5, SI2, 1);
  suonaDoppio(SOL5, SOL3, 1);
  suona(RE4, 0.5);
  suonaDoppio(FA5, RE4, 0.5);
  
  // Riga 4
  
  suonaDoppio(MI5, DO3, 1); // COn DO2 esplode!
  suonaDoppio(MI5, SOL3, 1);
  suonaDoppio(MI5, MI4, 1);
  
  suonaDoppio(LA4, SOL3, 1); // Con SOL2 esplode!
  suonaDoppio(SI4, SOL3, 1);
  suonaDoppio(FA5, SOL3, 1);
  
  suonaDoppio(MI5, MI3, 0.5);
  suonaDoppio(MI5, MI3, 0.5);
  suonaDoppio(MI5, MI3, 1.5);
  suonaDoppio(SI4, SI2, 0.5);
  
  suonaDoppio(DO5, DO3, 3);

  tronca = pretronca;
  
}

void OVERWORLD() {
  introduzione();
  OVERWORLDFoglio1();
  // Foglio 2
  OVERWORLDFoglio2();
  // Foglio 3
  OVERWORLDFoglio2();
  // Foglio 4
  OVERWORLDFoglio4();
  // Foglio 5
  OVERWORLDFoglio1();
  // Foglio 6
  OVERWORLDFoglio6();
  // Foglio 7
  //OVERWORLDFoglio6();
  // Foglio 8
  //OVERWORLDFoglio4();
  // Foglio 9
  //OVERWORLDFoglio6(); 
}
