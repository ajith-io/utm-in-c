#include<math.h>

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  double var = -45678.12356; 
  //Serial.println(var, 7);
  //Serial.println(convert_double_to_String(var));
  convert_double_to_String(var);
  
}



String convert_double_to_String(double floatVal){
  int negative_float = 0;
  if (floatVal < 0){
    negative_float = 1;
    floatVal = (-1) * floatVal;
  }
  int noofdigits_before_pt = log10(floatVal) + 1;
  int noofdigit_after_pt = 7 - noofdigits_before_pt;
  char rev_digits_before_pt[16];
  char rev_digits_after_pt[16];
  char final_str[16];
  long intPart = floatVal;
  long decPart = (pow(10, noofdigit_after_pt))*(floatVal - intPart);
  long intPart_copy = intPart;
  long decPart_copy = decPart;
  if(decPart < 0){
    decPart =  (-1)*decPart;//if negative, multiply by -1
  }
  
  int i = 0;

  for (i = 0; ((i < 16) && (intPart_copy > 0));i++){
    rev_digits_before_pt[i] = 48 + (intPart_copy % 10);
    intPart_copy /= 10;
  }
  rev_digits_before_pt[i] = '\0';
  
  for (i = 0; ((i < 16) && (decPart_copy > 0));i++){
    rev_digits_after_pt[i] = 48 + (decPart_copy % 10);
    decPart_copy /= 10;
  }

  rev_digits_after_pt[i] = '\0';

  int count = 0;
  if (negative_float){
    final_str[count] = '-';
    count += 1 ;
  }

  for(i= noofdigits_before_pt - 1;i >= 0;i--) {
    final_str[count] = rev_digits_before_pt[i];
    count += 1;
  }
  
  final_str[count] = '.';
  count += 1;

  for(i = noofdigit_after_pt - 1; i >= 0;i--){
    final_str[count] = rev_digits_after_pt[i];
    count += 1;
  }
  while (count < 16){
    final_str[count] = ' ';
    count += 1;
  }
  String s = final_str;
  return (s);
}
