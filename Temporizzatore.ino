/*
 * Temporizzatore aspirapolvere per utensili vari. Idealmente da integrare in insieme di due prese 230/50,
 * una sempre alimentata ma sotto sensore e una sotto relè per l'aspirapolvere. Sensore di corrente ACS712-30A,
 * uscita in tensione data da Vcc/2 + 66mV/A. Condizionato per avere portata teoricamente 0-5V, purtroppo massima 
 * effettiva pari a ca 3Vpk (Vsat opamp...) con poco più di 500mA(AC) di carico. Forma d'onda di sinusoide 
 * rettificata singola, causa opamp in single supply. 
 * Scheda alimentata a 12V con regolatore a 5V, alloggiamento ACS712 e condizionamento per lo stesso, 
 * uC ATMega328P-AU (TQFP) e relè di uscita in potenza con relativo transistor. Attrezzata obvsly per ICSP. 
 * Nome file .brd: "TIMERASPIRAPOLVERE".
 */

//Definizione pin I/O
#define PINSENS A0
#define PINRELE 2

//Definizione costanti
#define SOGLIA_TENSIONE 1.25
#define RITARDO_S 20

//Prototipi funzioni
bool ControllaIngresso(int pin); //Controllo stato ingresso analogico, ritorna true se in condizione di carico e false se privo di carico
void Temporizzatore(bool carico); //Controllo uscita in base a condizione fornita da ControllaIngresso()

void setup()
{
  pinMode(PINSENS, INPUT);
  pinMode(PINRELE, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop()
{
  bool trapanoAttivo = ControllaIngresso(PINSENS);
  Temporizzatore(trapanoAttivo);
  digitalWrite(3, !digitalRead(3));
  delayMicroseconds(500);
}

bool ControllaIngresso(int pin)
{
  int varLetto = analogRead(pin);
  static int ptiSup = 0;
  static int ptiInf = 0;

  //A partire da varLetto si calcola il numero di cicli per cui il segnale è sopra/sotto soglia. 
  //Se va sopra per un certo tempo, carico = true; se sta sotto per più di un altro tempo, carico = false.
  
  if(varLetto > ((SOGLIA_TENSIONE * 1023)/5)) //converto soglia tensione in valore ADC. Pigrizia in def costanti...
  {
    ptiSup++;
  }
  else
  {
    ptiInf++;
  }

  if(ptiSup == 5) 
  {
    /*
     * Programma gira a 1kHz. La rete a 50Hz. Quindi 20 cicli pgm per periodo di rete. Solo la semionda positiva è visibile,
     * quindi in realtà 10 cicli pgm utili per vedere il segnale. Diciamo che se è per almeno metà tempo sopra la soglia,
     * l'utensile viene considerato in uso.
     */
    ptiSup = 0;
    ptiInf = 0;
    return true;
  }

  if(ptiInf == 30)
  {
    /*
     * Ok bastavano anche un solo ciclo utile (semiperiodo positivo) sotto soglia, ma per sicurezza facciamo due. E' 30 perchè tra
     * i due semiperiodi in esame ce n'è uno che sarà sicuramente a zero (la semionda negativa) che non deve influenzare la misura.
     */
    ptiSup = 0;
    ptiInf = 0;
    return false;
  }

  return NULL;
}

void Temporizzatore(bool carico)
{
  static bool caricoPrec = false;

  if(carico == NULL)
  {
    return;
  }

  if((carico == true)&&(caricoPrec == false))
  {
    digitalWrite(PINRELE, HIGH);
  }

  if((carico == false)&&(caricoPrec == true))
  {
    delay(RITARDO_S * 1000);
    digitalWrite(PINRELE, LOW);
  }

  caricoPrec = carico;
}
