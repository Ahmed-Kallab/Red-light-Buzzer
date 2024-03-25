#include <Arduino.h>
#include <avr_debugger.h>
#include <RGLED.h>
#include <ShiftReg.h>
#include <Segment.h>
#include <LCD.h>
#include <USART.h>
#include <INT0.h>
#include <Ultrasonic.h>

#define ARRAY_SIZE 10
#define Buzzer PC4

volatile int Temp_Num = 0;
volatile int Temp_Flag = 0;

float read_distance()
{
  float distance;
  TCNT1 = 0x00;
  PORTB &= ~(1 << TRIG_PIN); // Clear the TRIG pin
  _delay_us(2);              // Wait for 2 microseconds
  PORTB |= (1 << TRIG_PIN);  // Set the TRIG pin
  _delay_us(10);             // Wait for 10 microseconds
  PORTB &= ~(1 << TRIG_PIN); // Clear the TRIG pin
  while (!(PINB & (1 << ECHO_PIN)))
    ;                    // Wait for ECHO pin to go high
  TCNT1 = 0;             // Reset timer counter
  TCCR1B |= (1 << CS10); // Start the timer
  while (PINB & (1 << ECHO_PIN))
    ;                                            // Wait for ECHO pin to go low
  TCCR1B = 0;                                    // Stop the timer
  distance = (float)((double)TCNT1 * 0.034 / 2); // Calculate the distance in cm
  return distance;
}

int main()
{
#ifdef __DEBUG__
  dbg_start();
#endif
  // Ultrasonic:
  DDRB |= (1 << TRIG_PIN);  // Set TRIG pin as output
  DDRB &= ~(1 << ECHO_PIN); // Set ECHO pin as input
  // Shif Registor
  init_shift(DATA, CLOCK, LATCH);

  // 7-sigment Number display: 0->9
  byte digits[ARRAY_SIZE] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};

  // USART Commands:
  USART_init();
  char CommandNumber_Str[MAX_TEXT];
  int CommandNumber_int;

  // Buzzer
  DDRC |= (1 << Buzzer);
  // LCD Commands:
  LCD_init();
  LCD_command(1);

  // RGB LED
  DDRD |= (1 << Red_LED) | (1 << Green_LED);
  DDRD &= ~(1 << Button_Pedestrian);
  int i, j, ThatFlag = 0, thisFlag = 0;

  // int Distance = 7;

  // interrupt
  //PCINT0_init();
  while (1)
  {
    if (Temp_Num == 1)
    {
      OFF();
      LCD_command(1);
      LCD_string("Interrupt Occured");
      USART_send_string("\nInterrupt Occured\n");
      USART_send_string("Welcome to Master Access, Please Enter the command number for the instruction needed: \n");
      USART_send_string("Commands:\n\n[0] Car taffic to "
                        "Green"
                        "\n[1] Car traffic to "
                        "Red"
                        "\n[2] Deactive the Buzzer\n[3] Active the Buzzer\n[4] Deactive the Button for Pedestrian\n[5] Aactive the Button for Pedestrian\n[6] Turn OFF the system\n\nCommand:");
      memset(CommandNumber_Str, 0, MAX_TEXT);
      USART_get_string(CommandNumber_Str);
      CommandNumber_int = atoi(CommandNumber_Str);
      sprintf(CommandNumber_Str, "%d", CommandNumber_int);
      // memset(CommandNumber_Str, 0, MAX_TEXT);
      switch (CommandNumber_int)
      {
      case 0:
        Temp_Flag == 0;
        Temp_Num == 0;
        break;

      case 1:
        Temp_Flag == 1;
        Temp_Num == 0;
        break;

      case 2:
        ThatFlag = 1;
        Temp_Num = 0;
        break;

      case 3:
        ThatFlag = 0;
        Temp_Num = 0;
        break;

      case 4:
        thisFlag = 1;
        Temp_Num = 0;
        break;

      case 5:
        thisFlag = 0;
        Temp_Num = 0;
        break;

      case 6:
        USART_send_string("--------------Good Bye!------------------\n");
        LCD_command(1);
        LCD_string("Good Bye");
        exit(0);
      }
    }

    if (Temp_Flag == 0)
    {
      // Car traffic "Green" for 10 sec time and Pedestrian Traffic "Red"
      LCD_command(1);
      LCD_string("Car Traffic");                                                             // In LCD line 1 "it is Car traffic"
      USART_send_string("\nCar Traffic light Green and Red traffic light for Pedestrian\n"); // In USART: "it is Car traffic"
      Green();
      // Displaying the remaining time to 7-sigment
      for (i = 9; i > -1; i--)
      {
        displyValue(digits[i]);
        _delay_ms(1000); // Delay of 1 sec in each iterations

        // if pedestrian button is pressed then decriment the time by half
        if ((PIND2 & (1 << Button_Pedestrian)) && thisFlag == 0) // Check if button is pressed:time(x) * 0.5;
        {
          i = i / 2;
        }
        if (i == 0) // Check if time(x) is over
        {
          OFF();
          Temp_Flag = 1;
        }
      }
    }

    if (Temp_Flag == 1)
    {
      // Car traffic "red" for 10 sec time and Pedestrian Traffic "Green"
      LCD_command(1);
      LCD_string("Pedestian Trafic");                                                        // In LCD line 1 "it is Pedestrian traffic"
      USART_send_string("\nCar Traffic light Red and Green traffic light for Pedestrian\n"); // In USART: "it is Pedestrian traffic"
      Red();
      // Displaying the remaining time to 7-sigment
      for (j = 9; j > -1; j--)
      {
        displyValue(digits[j]);
        _delay_ms(1000); // Delay of 1 sec in each iterations
        float Distance = read_distance();
        // if Ultrasonic detects Objcet then Buzz for 2 seconds
        if (0 < Distance && Distance <= 100 && ThatFlag == 0)
        {
          // In LCD line 2
          LCD_command(0xC0); // Move to second line
          LCD_string("Objection occure");
          USART_send_string("Objection occure\n");
          PORTC |= (1 << Buzzer);
          _delay_ms(500); // Buzzer on for 2 sec

          PORTC &= ~(1 << Buzzer);
          LCD_command(1);
          LCD_string("Pedestian Trafic");
        }
        if (j == 0 || j < 0) // Check if time(x) is over
        {
          OFF();
          Temp_Flag = 0;
        }
      }
      Temp_Num = 1;
    }

    //-_- Case 1
    /*Add another Button to access the Master access*/

    //-_- Case 2
    /*Maybe save it to an text file [Vehicle 1: 12:15PM]*/
    /*Add another Button to access the Master access*/

    // Else if:                                                                -_- Case 3
    // Master Access USART: Switch case :
    /*In LCD Line 2: Interrup Occured*/
    /*In USART: Interrup Occured*/

    // byte INPUT = 5
    // While (INPUT == 5 || INPUT > 5 || Input < 0 )

    /*In USART:

    Commands:
    [0] Car taffic to "Green"
    [1] Car traffic to "Red"
    [2] Deactive the Buzzer
    [3] Active the Buzzer
    [4] Deactive the Button for Pedestrian
    [5] Aactive the Button for Pedestrian
    [6] Turn OFF the system

    Command: ###
    */

    //[Case 0]:
    // Car taffic to "Green"
    //[Case 1]:
    // Car traffic to "Red"
    //[Case 2]:
    // Deactive the Buzzer
    //[Case 3]:
    // Deactive the Button for Pedestrian
    //[Case 4]:
    // Turn OFF the system

    // Start off with the car traffic light Green and pedestrian light Red
    //
  }
}

ISR(INT0_vect)
{
  Temp_Num = 1;
  Temp_Flag = 2;
}
