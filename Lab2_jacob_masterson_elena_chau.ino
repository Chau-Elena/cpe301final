/****************************************
 * Name:
 * Assignment:
 * Date:
 ****************************************/

void setup() {
  Serial.begin(9600); // sets up the serial port, 9600 is the baud rate. The speed at which a signal can travel
  Serial.println("Hello from the Serial monitor!");
 
  pinMode(8, OUTPUT); // Output is the macro
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, INPUT);
  pinMode(3, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  int buttonFirst = digitalRead(3);
  int buttonSecond = digitalRead(4);

    // DECODER BLOCK
  if(buttonSecond == LOW && buttonFirst == LOW){      // First starts on the right
    // write 0001 to the LED
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(7,LOW);
    digitalWrite(8,HIGH); 
    delay(1000);
  }else if(buttonSecond == LOW && buttonFirst == HIGH){
    // write 0010 to the LED 
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(7,HIGH);
    digitalWrite(8,LOW); 
    delay(1000);
    
  }else if(buttonSecond == HIGH && buttonFirst == LOW){
    // write 0100 to the LED 
    digitalWrite(5,LOW);
    digitalWrite(6,HIGH);
    digitalWrite(7,LOW);
    digitalWrite(8,LOW); 
    delay(1000);
    
  }else if(buttonSecond == HIGH && buttonFirst == HIGH){
    // write 1000 to the LED 
    digitalWrite(5,HIGH);
    digitalWrite(6,LOW);
    digitalWrite(7,LOW);
    digitalWrite(8,LOW); 
    delay(1000);
    
  } // end DECODER BLOCK
  
}
