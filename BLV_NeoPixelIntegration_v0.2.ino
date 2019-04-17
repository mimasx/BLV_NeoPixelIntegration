/* 
 NeoPixel related and Code comes from the following source:
 https://learn.adafruit.com/multi-tasking-the-arduino-part-1
 https://learn.adafruit.com/multi-tasking-the-arduino-part-2
 https://learn.adafruit.com/multi-tasking-the-arduino-part-3
 It was modified that it fits to our needs.

 Some informations regarding the transmitted data via serial communication comes from:
 https://github.com/dc42/PanelDueFirmware

 TODOS

 - Serial Port Sniffing and Message Handling -> State Maschine and hopefully get rid of the wrong readings.
 - Progress Circles pulsating and get rid of that one red dot (has something todo with the counting)
 - When Printing is finished, switch back to temperatur display
 - global dimming value
 - opt: Integration of more designs
 - opt: use one or two switches to change designs or shown values
*/

#include <Adafruit_NeoPixel.h>

String Message;
bool NewMessage=false;
bool UpdateSerial=false;

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, PROGRESSBAR, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

class PanelMessage {
  public:
  int Status;
  float ActTempHeatBed;
  float ActTempExtruder;
  float SetTempHeatBed;
  float SetTempExtruder;
  float HeaterStatusHeatBed;
  float HeaterStatusExtruder;
  float FractionPrinted;
  float TimesLeft1;
  float TimesLeft2;
  float TimesLeft3;  
};
PanelMessage Printer;


// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    uint16_t Progress;  // current progressindicator. Only implemented in Progressbar
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case PROGRESSBAR:
                    ProgressBarUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
        Progress=0;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        Progress=0;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
        Progress=0;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
        Progress=0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a ProgressBar
    void ProgressBar(uint32_t color1, uint32_t color2, uint8_t interval)
    {
        ActivePattern = PROGRESSBAR;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Progress=0;
    }

    // Update the ProgressBar Pattern
    void ProgressBarUpdate()
    { 
        for (int i = 1; i <= numPixels(); i++)
        {
            if ((i/Index<=1)&i<=Progress)
            {
              if (i/Index>0.05)
              {
                setPixelColor(numPixels()-i, Color(Red(Color2)*i/Index,Green(Color2)*i/Index,Blue(Color2)*i/Index));
              }
              else
              {
                setPixelColor(numPixels()-i, Color(Red(Color2)*0.05,Green(Color2)*0.05,Blue(Color2)*0.05));
              }
            }
            else if (i <= Progress)
            {
              setPixelColor(numPixels()-i, Color(Red(Color2)*0.05,Green(Color2)*0.05,Blue(Color2)*0.05));
            }
            else
            {
              setPixelColor(numPixels()-i, Color1);
            }
        }
        setPixelColor((numPixels()-Progress),Color(Red(Color2),Green(Color2),Blue(Color2)));
        show();
        Increment();
    }

        
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        Progress=0;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

void Ring1Complete();
void Ring2Complete();
void Ring3Complete();


// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
NeoPatterns Ring1(16, 6, NEO_GRB + NEO_KHZ800, &Ring1Complete);
NeoPatterns Ring2(16, 7, NEO_GRB + NEO_KHZ800, &Ring2Complete);
NeoPatterns Ring3(16, 8, NEO_GRB + NEO_KHZ800, &Ring3Complete);

float suptract_pos (float a, float b){
  if(a<b){
    return (b-a);
  }
  else{
    return (a-b);
  }
}

float FloatTemp2,FloatTemp, SetTempExtruderTemp, SetTempExtruderTemp2, SetTempHeatBedTemp, SetTempHeatBedTemp2, ActTempHeatBedTemp, ActTempHeatBedTemp2, ActTempExtruderTemp, ActTempExtruderTemp2;

void CheckSerial() {
  long time1=millis();
  String temp="";
  int i=0,j=0, Pos=0, Pos2=0;
  
  //get Status
/*
  connecting = 0,
  idle = 1,
  printing = 2,
  stopped = 3,
  configuring = 4,
  paused = 5,
  busy = 6,
  pausing = 7,
  resuming = 8,
  flashing = 9
*/

  Pos=Message.indexOf("status");
  char pstatus=Message.substring(Pos+9,Pos+10).charAt(0);
  switch (pstatus)
  {
    case 'I':
      Printer.Status=1;
      break;
    case 'P':
      Printer.Status=2;
      break;
    case 'S':
      Printer.Status=3;
      break;
  }

  //****************************************************************
  //The following part is still messy. Due to serial communication issues (sometimes the returned Temperatures
  //drop to zero or jump to much higher Values. I made some contruct to filter these values out. When there is a
  //solution for that issue, the workaround can be removed and the marked lines can be uncommented. 
  //****************************************************************
  
  //get ActTemperatures
  Pos=Message.indexOf("heaters");
  Pos2=Message.indexOf(",",Pos);
  //Printer.ActTempHeatBed=Message.substring(Pos+10,Pos2).toFloat(); <-can be uncommented when filter workaround is not needed anymore.

  //begin filter workaround
  ActTempHeatBedTemp2=ActTempHeatBedTemp;
  ActTempHeatBedTemp=Message.substring(Pos+10,Pos2).toFloat();
  if(ActTempHeatBedTemp>0){
    if((ActTempHeatBedTemp2==0) || (suptract_pos(ActTempHeatBedTemp2, ActTempHeatBedTemp)<7)){
      Printer.ActTempHeatBed=ActTempHeatBedTemp;  
    }
  }
  //end workaround
  
  Pos=Message.indexOf("]",Pos2);
  //Printer.ActTempExtruder=Message.substring(Pos2+1,Pos).toFloat(); <-can be uncommented when filter workaround is not needed anymore.

  //begin filter workaround
  ActTempExtruderTemp2=ActTempExtruderTemp;
  ActTempExtruderTemp=Message.substring(Pos2+1,Pos).toFloat();
  if(ActTempExtruderTemp>0){
    if((ActTempExtruderTemp2==0) || (suptract_pos(ActTempExtruderTemp2, ActTempExtruderTemp)<7)){
      Printer.ActTempExtruder=ActTempExtruderTemp;  
    }
  }
  //end workaround
 
  //get SetTemperatures
  Pos=Message.indexOf("active");
  Pos2=Message.indexOf(",",Pos);

  //Printer.SetTempHeatBed=Message.substring(Pos+9,Pos2).toFloat(); <-can be uncommented when filter workaround is not needed anymore.
  //begin filter workaround
  SetTempHeatBedTemp2=SetTempHeatBedTemp;
  SetTempHeatBedTemp=Message.substring(Pos+9,Pos2).toFloat();
  if(SetTempHeatBedTemp>0){
    if((SetTempHeatBedTemp2==0) || (suptract_pos(SetTempHeatBedTemp2, SetTempHeatBedTemp)<7)){
      Printer.SetTempHeatBed=SetTempHeatBedTemp;  
    }
  }
  //end workaround  
  
  Pos=Message.indexOf("]",Pos2);

  //Printer.SetTempExtruder=Message.substring(Pos2+1,Pos).toFloat(); <-can be uncommented when filter workaround is not needed anymore.
  //begin filter workaround
  SetTempExtruderTemp2=SetTempExtruderTemp;
  SetTempExtruderTemp=Message.substring(Pos2+1,Pos).toFloat();
  if(SetTempExtruderTemp>0){
    if((SetTempExtruderTemp2==0) || (suptract_pos(SetTempExtruderTemp2, SetTempExtruderTemp)<7)){
      Printer.SetTempExtruder=SetTempExtruderTemp;  
    }
  }
  //end workaround  
    
  //get HeaterStatus   
  Pos=Message.indexOf("hstat");
  Pos2=Message.indexOf(",",Pos);
  Printer.HeaterStatusHeatBed=Message.substring(Pos+8,Pos2).toFloat();  
  Pos=Message.indexOf("]",Pos2);
  Printer.HeaterStatusExtruder=Message.substring(Pos2+1,Pos).toFloat();
  
  //get Progress  
  Pos=Message.indexOf("fraction_printed");
  FloatTemp=Message.substring(Pos+18,Pos+24).toFloat();
  if(FloatTemp!=0){
    Printer.FractionPrinted=FloatTemp;
  }
}

void serialEvent2(){
  UpdateSerial=true;
}

void GetMessage(){                                                      //wait until Buffer is filled with one Message?!?
    while(Serial2.available() > 0) 
    {    
      char inChar = Serial2.read();
      if (inChar=='{')
      {
        NewMessage=true;
      }
      else if (NewMessage==true)
      {
        if (inChar=='}')
        {
          NewMessage=false;
 //         Serial.println(Message);
          CheckSerial();
          Serial.println("Status: "+String(Printer.Status)+"; THB: "+Printer.ActTempHeatBed+"; TExtr: "+Printer.ActTempExtruder+"; TSetHeatBed: "+Printer.SetTempHeatBed+"; TSetExtr: "+Printer.SetTempExtruder+"; hHeatBed: "+Printer.HeaterStatusHeatBed+"; hExtr: "+Printer.HeaterStatusExtruder+" Progress: "+Printer.FractionPrinted);
          Message="";
        }else
        {
          Message += inChar;
        }
      }
    }
}

// Initialize everything and prepare to start
void setup()
{   
  Serial.begin(38400);  
  while(!Serial)
  {;}
  Serial2.begin(57600);
  while(!Serial2)
  {;}
  Serial.println("Duet port started");
  
    // Initialize all the pixelStrips
    Ring1.begin();
    Ring2.begin();
    Ring3.begin();
    
    // Kick off a pattern
    Ring1.ProgressBar(Ring1.Color(25,25,25), Ring1.Color(255,0,0), 100);
    Ring2.Fade(Ring2.Color(25,25,25), Ring2.Color(0,255,0), 8, 100);
    Ring3.ProgressBar(Ring3.Color(25,25,25), Ring3.Color(255,0,0), 100);
}

// Main loop
void loop()
{
    // Update the rings.
    

    
    Ring1.Update();
    Ring2.Update();    
    Ring3.Update();
    
  int AmbientTemp=15; //nicer Display of ProgressBars
  
  if(UpdateSerial=true){
    GetMessage();
/*
  connecting = 0,
  idle = 1,
  printing = 2,
  stopped = 3,
  configuring = 4,
  paused = 5,
  busy = 6,
  pausing = 7,
  resuming = 8,
  flashing = 9
*/

    //****************************************************************
    //Assignment of Colors to the printer.Status
    //****************************************************************
    switch (Printer.Status)
    {
      case 1:
        Ring2.Color2 = Ring2.Color(0,0,255);
        break;
      case 2:
        Ring2.Color2 = Ring2.Color(0,255,0);
        break;
      case 3:
        Ring2.Color2 = Ring2.Color(0,0,255);
        break;
      case 4:
        Ring2.Color2 = Ring2.Color(255,255,0);
        break;
      case 5:
        Ring2.Color2 = Ring2.Color(160,32,240);
        break;
      case 6:
        Ring2.Color2 = Ring2.Color(255,255,0);
        break;
      case 7:
        Ring2.Color2 = Ring2.Color(160,32,240);
        break;
      case 8:
        Ring2.Color2 = Ring2.Color(255,255,0);
        break;
      case 9:
        Ring2.Color2 = Ring2.Color(255,255,0);
        break;
      default:
        Ring2.Color2 = Ring2.Color(50,50,50);
        break;
    }
    
    //****************************************************************
    // Here happens the assignment of Printer Values to each NeoPixel
    //****************************************************************
    if(Printer.FractionPrinted>0){ //is the print startet or is the printer still heating?
      Ring1.Color2=Ring1.Color(0,0,255);
      Ring3.Color2=Ring1.Color(0,0,255);
      Ring1.Progress=(Printer.FractionPrinted*16);
      Ring3.Progress=(Printer.FractionPrinted*16);
    }
    else{
      Ring1.Color2=Ring1.Color(255,0,0);
      Ring3.Color2=Ring1.Color(255,0,0);
      Ring1.Progress=((Printer.ActTempHeatBed-AmbientTemp)/(Printer.SetTempHeatBed-AmbientTemp)*16);
      Ring3.Progress=((Printer.ActTempExtruder-AmbientTemp)/(Printer.SetTempExtruder-AmbientTemp)*16);
    }
    //****************************************************************
    UpdateSerial=false;
  }

}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Ring1 Completion Callback
void Ring1Complete()
{
  //Ring1.Reverse();
}

// Ring 2 Completion Callback
void Ring2Complete()
{
  Ring2.Reverse();
}

// Stick Completion Callback
void Ring3Complete()
{
  //Ring3.Reverse();
}
