int dro_bits[24];        // For storing the data bit. bit_array[0] = data bit 1 (LSB), bit_array[23] = data bit 24 (MSB).
char str[7]; //Array for the char "string"

unsigned long droTimer = 0; //update frequency (the DRO is checked this often)
unsigned long buttonTimer = 0; //debouncing

int clk = 4; //Blue
int data = 5; //Red
int button = 6; //button pin

float convertedValue = 0.0; //raw conversion value (bit to mms)
float resultValue = 0.0; //final result value: conversion value - tare value (if there is any taring)
float previousresultValue = 0.0; //temporary storage to be able to register a change in the value
float tareValue = 0.0; //tare value to set a new zero point anywhere (using a button)

void setup()
{
  Serial.begin(9600);
  Serial.println("DRO reading Arduino"); //test message to see if the serial works
  pinMode(clk, INPUT_PULLUP);
  pinMode(data, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
}

void loop()
{
  readEncoder();
  readButton();
  printSerial();
}

void printSerial()
{
  //The serial is only updated if the value of the DRO changes.
  if (resultValue != previousresultValue)
  {
    Serial.println("Result after taring: ");
    Serial.println(resultValue); //Print the current result value to the serial
    previousresultValue = resultValue; //save the most recent reading
  }
}

void readButton()
{
  if (digitalRead(button) == 0) //The button should pull the circuit to GND! (It is pulled up by default)
  {
    if (millis() - buttonTimer > 300) //software "debounce"
    {
      tareValue = convertedValue; //use the most recent conversion value as the tare value
      buttonTimer = millis();
    }
  }
}

void readEncoder()
{
  //This function reads the encoder
  //I added a timer if you don't need high update frequency

  if (millis() - droTimer > 1000) //if 1 s passed we start to wait for the incoming reading
  {
    convertedValue = 0; //set it to zero, so no garbage will be included in the final conversion value

    //This part was not shown in the video because I added it later based on a viewer's idea
    unsigned long syncTimer = 0; //timer for syncing the readout process with the DRO's clock signal
    bool synchronized = false; // Flag that let's the code know if the clk is synced
    while (synchronized == false)
    {
      syncTimer = millis(); //start timer
      while (digitalRead(clk) == HIGH) {} //wait until the clk goes low

      if (millis() - syncTimer > 5) //if the signal has been high for more than 5 ms, we know that it has been synced
      {
        synchronized = true;
      }
      else
      {
        synchronized = false;
      }
    }

    for (int i = 0; i < 23; i++) //We read the whole data block - just for consistency
    {
      while (digitalRead(clk) == LOW) {} // wait for the rising edge

      dro_bits[i] = digitalRead(data);
      //Print the data on the serial
      Serial.print(dro_bits[i]);
      Serial.print(" ");

      while (digitalRead(clk) == HIGH) {} // wait for the falling edge
    }
    Serial.println(" ");

    //Reconstructing the real value
    for (int i = 0; i < 20; i++) //we don't process the whole array, it is not necessary for our purpose
    {
      convertedValue = convertedValue + (pow(2, i) * dro_bits[i]);
    }

    if (dro_bits[20] == 1)
    {
      Serial.println("Positive ");
    }
    else
    {
      convertedValue = -1 * convertedValue; // convert to negative
      Serial.println("Negative ");
    }

    convertedValue = (convertedValue / 100.0); //conversion to mm

    //The final result is stored in a separate variable where the tare is subtracted
    resultValue = convertedValue - tareValue;

    //Dump everything on the serial
    Serial.print("Raw reading: ");
    Serial.println(convertedValue);
    Serial.print("Tare value: ");
    Serial.println(tareValue);
    Serial.print("Result after taring: ");
    Serial.println(resultValue);
    Serial.println(" ");

    droTimer = millis();
  }
}
