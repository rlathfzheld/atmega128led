#define LED_PIN LED_BUILTIN
//#define프로그램이 컴파일되기 전에 프로그래머가 상수 값에 이름을 지정할 수 있게 해주는 유용한 C++ 구성 요소입니다.
//예를 들어 # 정의된 상수 이름이 다른 상수 또는 변수 이름에 포함된 경우 원치 않는 부작용이 발생할 수 있습니다
//이 경우 텍스트는 #정의된 숫자(또는 텍스트)로 대체됩니다.

#define LED_OFF 0
#define LED_ON 1
#define LED_BLINK 2

#define ON_MIL 3000
#define OFF_MIL 500

bool onVal;
bool offVal;

int mode; // 0:off, 1:on, 2:blink //int(정수)는 숫자 저장을 위한 기본 데이터 유형입니다.
int onMil;
int offMil;

bool blinkOn; // bool는 ture or false 두 값 중 하나를 보유
unsigned long nextMil; 
//unsigned long 변수는 숫자 저장을 위한 확장된 크기 변수이며 32비트(4바이트)를 저장합니다.
//표준 long과 달리 unsigned long은 음수를 저장하지 않으므로 범위는 0에서 4,294,967,295(2^32 - 1)입니다.

void ledInit(int pin, bool INonVal) {
  onVal = INonVal;
  offVal = !onVal;

  mode = LED_OFF;
  onMil = ON_MIL;
  offMil = OFF_MIL;
  pinMode(pin,OUTPUT);
  digitalWrite(pin,INonVal); // off
}
//The void keyword is used only in function declarations. VOID는 선언에서만 사용된다.
void ledSetOnMil(int INval) {
  onMil = INval;
}

void ledSetOffMil(int INval) {
  offMil = INval;
}

void ledOff() {
   digitalWrite(LED_PIN,offVal);
}

void ledOn() {
   digitalWrite(LED_PIN,onVal);
}

void ledBlink() {
  digitalWrite(LED_PIN,onVal);                
  blinkOn = true;
  nextMil = millis() + onMil;
}

void ledRun() {
  if (mode == LED_BLINK) {
    if(blinkOn && millis() > nextMil) { //&& Logical AND
      digitalWrite(LED_PIN,offVal);
      blinkOn = false;
      nextMil = millis() + offMil;
    }
    else if(!blinkOn && millis() > nextMil) {
      digitalWrite(LED_PIN,onVal);
      blinkOn = true;
      nextMil = millis() + onMil;
    }
  }
}
//setup()함수는 스케치가 시작될 때 호출됩니다. 변수 초기화, 핀 모드, 라이브러리 사용 시작 등에 사용
//setup()기능은 Arduino 보드의 전원을 켜거나 재설정할 때마다 한 번만 실행
void setup() {
  Serial.begin(115200);
  Serial.println();
 
  Serial.println("Input m 0 for led off");
  Serial.println("Input m 1 for led on");
  Serial.println("Input m 2 for led blink");
  Serial.println("Input n nnn for onMil");
  Serial.println("Input f nnn for offMil");

  ledInit(LED_PIN,LOW); // PIN, onVal
}
  
void loop() {
//setup()초기 값을 초기화하고 설정 하는 함수를 만든 후
//loop()함수는 이름이 제안하는 대로 정확하게 수행하고 프로그램이 변경하고 응답할 수 있도록 연속적으로 루프합니다.
//Arduino 보드를 능동적으로 제어하는 ​​데 사용
  while(Serial.available()) {
    char c = Serial.read();
    if (c == 'm') {
      int val = Serial.parseInt();
      //if 문과 마찬가지로 switch case 는 프로그래머가 다양한 조건에서 실행되어야 하는 다른 코드를 지정할 수 있도록 하여 프로그램의 흐름을 제어합니다.
      //특히 switch 문은 변수 값을 case 문에 지정된 값과 비교합니다.
      //변수의 값과 일치하는 case 문을 찾으면 해당 case 문에 있는 코드가 실행됩니다.
      switch (val) {
        case 0 : mode = LED_OFF;
                 ledOff();              
                 Serial.println("mode=LED_OFF");
                 break;
                 //break 키워드 는 switch 문을 종료하며 일반적으로 각 경우의 끝에 사용
        case 1 : mode = LED_ON;
                 ledOn();
                 Serial.println("mode=LED_ON");
                 break;
        case 2 : mode = LED_BLINK;
                 ledBlink();
                 Serial.println("mode=LED_BLINK");
                 break;
                 //break 문이 없으면 switch 문은 break 또는 switch 문의 끝에 도달할 때까지 다음 식("falling-through")을 계속 실행
        default: Serial.println("mode ERROR!!");
      }
    }
    else if (c == 'n') {
      int val = Serial.parseInt();
      ledSetOnMil(val);
      Serial.printf("ledSetOnMil=%d\n",val);
    }
    else if (c == 'f') {
      int val = Serial.parseInt();
      ledSetOffMil(val);
      Serial.printf("ledSetOffMil=%d\n",val);
    }
  }

  ledRun(); // LED_BLINK일 때 ON,OFF
}
