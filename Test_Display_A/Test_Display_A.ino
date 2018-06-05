#include <LiquidCrystal.h>

#define LCD_VSS 13
#define LCD_VDD 12
#define LCD_CNTRST 11
#define LCD_RST 10
#define LCD_RW 9
#define LCD_ENB 8
#define LCD_D0 22
#define LCD_D1 24
#define LCD_D2 26
#define LCD_D3 28
#define LCD_D4 30
#define LCD_D5 32
#define LCD_D6 34
#define LCD_D7 36

LiquidCrystal lcd(LCD_RST, LCD_ENB, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_CNTRST, OUTPUT);
  pinMode(LCD_VDD, OUTPUT);
  pinMode(LCD_VSS, OUTPUT);

  digitalWrite(LCD_RW, LOW);
  analogWrite(LCD_CNTRST, 150);
  digitalWrite(LCD_VDD, HIGH);
  digitalWrite(LCD_VSS, LOW);

  Serial.begin(9600);
  
  // set up the LCD's number of columns and rows:
  lcd.clear();
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Hello");
  lcd.setCursor(0, 1);
  
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
  
  delay(1000);
  Serial.println(millis()/1000);
  
}
